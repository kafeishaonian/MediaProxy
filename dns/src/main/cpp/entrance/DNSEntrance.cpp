//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSEntrance.h"


namespace dns {

    std::shared_ptr<dns::DNSEntrance> DNSEntranceImpl::instance_ = nullptr;
    std::mutex DNSEntranceImpl::instance_mutex_;

    DNSEntranceImpl::DNSEntranceImpl()
            : network_state_(DNSAppNetState::UNKNOWN),
              cache_dir_("/data/local/tmp/dns"),
              doh_server_(""),
              system_dns_enabled_(true),
              http_dns_enabled_(false),
              local_cache_enabled_(true),
              maintenance_running_(false) {

        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已创建");
    }

    DNSEntranceImpl::~DNSEntranceImpl() noexcept {
        // Fixed: Add try-catch to prevent exceptions from escaping destructor
        try {
            stop_maintenance_thread();

            if (dns_server_) {
                dns_server_->stop();
            }
            Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已销毁");
        } catch (const std::exception& e) {
            // Log but don't rethrow - destructors must not throw
            Logger::log(LogLevel::ERROR, "DNSEntrance",
                       std::string("析构异常: ") + e.what());
        } catch (...) {
            // Catch all to ensure noexcept
        }
    }


    std::shared_ptr<dns::DNSEntrance> DNSEntranceImpl::get_instance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        if (!instance_) {
            instance_ = std::make_shared<DNSEntranceImpl>();
            Logger::log(LogLevel::INFO, "DNSEntrance", "创建DNS单例");
        }
        return instance_;
    }


    void DNSEntranceImpl::init() {
        std::lock_guard<std::mutex> lock(mutex_);

        Logger::log(LogLevel::INFO, "DNSEntrance", "开始初始化DNS服务");

        data_cache_ = std::make_shared<DNSDataCache>(Constants::DEFAULT_CACHE_SIZE);
        data_cache_->set_cache_dir(cache_dir_);

        dns_manager_ = std::make_shared<DNSHostManager>();
        dns_manager_->set_data_cache(data_cache_);

        // 从持久化缓存加载历史数据
        dns_manager_->load_from_cache();

        speed_checker_ = std::make_shared<DNSSpeedChecker>();

        dns_server_ = std::make_shared<DNSServer>();
        dns_server_->set_host_manager(dns_manager_);
        dns_server_->set_speed_checker(speed_checker_);

        init_dns_handlers();

        // 设置速度检测回调，将速度检测任务提交到 DNSServer 的任务队列
        dns_manager_->set_speed_check_callback(
            [this](const std::string& hostname, std::shared_ptr<DNSHostModel> host) {
                if (dns_server_) {
                    dns_server_->submit_speed_check_task(hostname, host, nullptr);
                }
            }
        );

        dns_server_->start();

        // 启动维护线程
        start_maintenance_thread();

        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS服务初始化完成");
    }

    void DNSEntranceImpl::init_dns_handlers() {
        if (system_dns_enabled_) {
            auto system_dns = std::make_shared<DNSSystemServerHandle>();
            dns_server_->add_server_handle(DNSServerType::SYSTEM, system_dns);
            Logger::log(LogLevel::INFO, "DNSEntrance", "已启用系统DNS");
        }
        if (http_dns_enabled_ && !doh_server_.empty()) {
            auto http_dns = std::make_shared<DNSHttpServerHandle>(doh_server_);
            dns_server_->add_server_handle(DNSServerType::HTTP_DNS, http_dns);
            Logger::log(LogLevel::INFO, "DNSEntrance", "已启用HTTP DNS: " + doh_server_);
        }
        if (local_cache_enabled_) {
            auto local_dns = std::make_shared<DNSLocalServerHandle>();
            dns_server_->add_server_handle(DNSServerType::LOCAL, local_dns);
            Logger::log(LogLevel::INFO, "DNSEntrance", "已启用本地缓存");
        }
    }

    std::string DNSEntranceImpl::resolve_host(const std::string &hostname) {
        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "DNSEntrance", "DNS服务未初始化");
            return "";
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "解析: " + hostname);
        auto host = dns_server_->resolve_sync(hostname);
        if (host && host->has_valid_ip()) {
            std::string best_ip = host->get_best_ip_string();
            Logger::log(LogLevel::INFO, "DNSEntrance", hostname + " -> " + best_ip);
            return best_ip;
        }
        Logger::log(LogLevel::WARN, "DNSEntrance", "解析失败: " + hostname);
        return "";
    }

    void DNSEntranceImpl::resolve_host_async(const std::string &hostname, dns::TaskCallback callback) {
        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "DNSEntrance", "DNS服务未初始化");
            if (callback) {
                callback(nullptr, false, nullptr);
            }
            return;
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "异步解析: " + hostname);
        dns_server_->resolve_async(hostname, callback);
    }

    std::vector<std::string> DNSEntranceImpl::get_all_ips(const std::string &hostname) {
        std::vector<std::string> ips;
        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "DNSEntrance", "DNS服务未初始化");
            return ips;
        }
        auto host = dns_server_->resolve_sync(hostname);
        if (host) {
            auto ip_list = host->get_ip_list();
            for (const auto &ip_model: ip_list) {
                if (ip_model->is_valid()) {
                    ips.push_back(ip_model->get_ip());
                }
            }
        }
        return ips;
    }

    void DNSEntranceImpl::set_network_state(dns::DNSAppNetState state) {
        std::lock_guard<std::mutex> lock(mutex_);
        network_state_ = state;
        Logger::log(LogLevel::INFO, "DNSEntrance",
            "网络状态: " + std::to_string(static_cast<int>(state)));
        if (state == DNSAppNetState::NONE && dns_manager_) {
            dns_manager_->save_to_cache();
        }
    }

    void DNSEntranceImpl::clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (dns_manager_) {
            dns_manager_->clear_cache();
        }
        if (data_cache_) {
            data_cache_->clear();
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "清空缓存");
    }

    void DNSEntranceImpl::set_doh_server(const std::string &server) {
        std::lock_guard<std::mutex> lock(mutex_);
        doh_server_ = server;
        Logger::log(LogLevel::INFO, "DNSEntrance", "DoH: " + server);
        if (dns_server_ && http_dns_enabled_) {
            auto http_dns = std::make_shared<DNSHttpServerHandle>(server);
            dns_server_->add_server_handle(DNSServerType::HTTP_DNS, http_dns);
        }
    }

    void DNSEntranceImpl::enable_system_dns(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        system_dns_enabled_ = enable;
        Logger::log(LogLevel::INFO, "DNSEntrance",
            std::string("系统DNS: ") + (enable ? "启用" : "禁用"));
    }

    void DNSEntranceImpl::enable_http_dns(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        http_dns_enabled_ = enable;
        Logger::log(LogLevel::INFO, "DNSEntrance",
            std::string("HTTP DNS: ") + (enable ? "启用" : "禁用"));
    }

    void DNSEntranceImpl::enable_local_cache(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);
        local_cache_enabled_ = enable;
        Logger::log(LogLevel::INFO, "DNSEntrance",
            std::string("本地缓存: ") + (enable ? "启用" : "禁用"));
    }

    void DNSEntranceImpl::set_cache_dir(const std::string &dir) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_dir_ = dir;
        if (data_cache_) {
            data_cache_->set_cache_dir(dir);
        }
    }

    void DNSEntranceImpl::set_cache_size(size_t size) {
        if (!dns_server_ || !dns_server_->is_running()) {
            data_cache_ = std::make_shared<DNSDataCache>(size);
        }
    }

    void DNSEntranceImpl::set_cache_expire_time(int seconds) {
        Logger::log(LogLevel::INFO, "DNSEntrance",
            "缓存过期时间: " + std::to_string(seconds) + "秒");
    }

    void DNSEntranceImpl::set_thread_count(int count) {
        if (dns_server_) {
            dns_server_->set_thread_count(count);
        }
    }

    DNSServer::ServerStats DNSEntranceImpl::get_stats() const {
        if (dns_server_) {
            return dns_server_->get_stats();
        }
        return DNSServer::ServerStats();
    }

    void DNSEntranceImpl::clear_instance() {
        instance_ = nullptr;
    }

    void DNSEntranceImpl::start_maintenance_thread() {
        if (maintenance_running_.load()) {
            Logger::log(LogLevel::WARN, "DNSEntrance", "维护线程已在运行");
            return;
        }

        maintenance_running_.store(true);
        maintenance_thread_ = std::make_shared<std::thread>(&DNSEntranceImpl::maintenance_loop, this);

        Logger::log(LogLevel::INFO, "DNSEntrance", "维护线程已启动");
    }

    void DNSEntranceImpl::stop_maintenance_thread() {
        if (!maintenance_running_.load()) {
            return;
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "停止维护线程...");
        maintenance_running_.store(false);
        maintenance_cv_.notify_one();

        if (maintenance_thread_ && maintenance_thread_->joinable()) {
            // Fixed: Use timed join to avoid indefinite blocking
            auto start = std::chrono::steady_clock::now();
            auto timeout = std::chrono::seconds(5);

            while (maintenance_thread_->joinable()) {
                maintenance_cv_.notify_one();  // Keep notifying
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if (std::chrono::steady_clock::now() - start > timeout) {
                    Logger::log(LogLevel::WARN, "DNSEntrance",
                               "维护线程等待超时，强制分离");
                    // Cannot force-kill thread in C++, but we detach to avoid blocking
                    // The thread will eventually exit when it checks maintenance_running_
                    maintenance_thread_->detach();
                    break;
                }
            }

            if (maintenance_thread_->joinable()) {
                maintenance_thread_->join();
            }
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "维护线程已停止");
    }

    void DNSEntranceImpl::maintenance_loop() {
        Logger::log(LogLevel::INFO, "DNSEntrance", "维护线程启动");

        while (maintenance_running_.load()) {
            std::unique_lock<std::mutex> lock(maintenance_mutex_);
            if (maintenance_cv_.wait_for(lock, std::chrono::seconds(Constants::CLEANUP_INTERVAL_SECONDS),
                [this] { return !maintenance_running_.load(); })) {
                break;
            }

            if (!maintenance_running_.load()) {
                break;
            }

            try {
                Logger::log(LogLevel::INFO, "DNSEntrance", "执行定期维护");
                if (dns_manager_) {
                    int before = dns_manager_->get_host_count();
                    dns_manager_->clean_expired_hosts(Constants::CACHE_EXPIRE_SECONDS);
                    int after = dns_manager_->get_host_count();
                    Logger::log(LogLevel::INFO, "DNSEntrance",
                        "清理过期主机: " + std::to_string(before - after) + " 个");
                    dns_manager_->save_to_cache();
                }
                if (dns_server_) {
                    auto stats = dns_server_->get_stats();
                    Logger::log(LogLevel::INFO, "DNSEntrance",
                        "统计 - 总:" + std::to_string(stats.total_requests) +
                        " 成功:" + std::to_string(stats.success_requests) +
                        " 失败:" + std::to_string(stats.failed_requests) +
                        " 缓存:" + std::to_string(stats.cached_requests));
                }
            } catch (const std::exception &e) {
                Logger::log(LogLevel::ERROR, "DNSEntrance",
                    "维护异常: " + std::string(e.what()));
            }
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "维护线程退出");
    }
}