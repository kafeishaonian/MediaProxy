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
              local_cache_enabled_(true) {

        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已创建");
    }

    DNSEntranceImpl::~DNSEntranceImpl() noexcept {
        if (dns_server_) {
            dns_server_->stop();
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已销毁");
    }


    std::shared_ptr<dns::DNSEntrance> DNSEntranceImpl::get_instance() {
        if (instance_) {
            return instance_;
        }
        std::lock_guard<std::mutex> local(instance_mutex_);

        if (instance_ == nullptr) {
            instance_ = std::make_shared<DNSEntranceImpl>();
            Logger::log(LogLevel::INFO, "DNSEntrance",
                        "创建实例: ");
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

        speed_checker_ = std::make_shared<DNSSpeedChecker>();

        dns_server_ = std::make_shared<DNSServer>();
        dns_server_->set_host_manager(dns_manager_);
        dns_server_->set_speed_checker(speed_checker_);

        init_dns_handlers();

        dns_server_->start();

        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS服务初始化完成");
    }

    void DNSEntranceImpl::init_dns_handlers() {
        if (system_dns_enabled_) {
            auto system_dns = std::make_shared<DNSSystemServerHandle>();
            dns_server_->add_server_handle(DNSServerType::SYSTEM, system_dns);
            Logger::log(LogLevel::INFO, "MMDNSEntrance", "已启用系统DNS");
        }

        if (http_dns_enabled_ && !doh_server_.empty()) {
            auto http_dns = std::make_shared<DNSHttpServerHandle>(doh_server_);
            dns_server_->add_server_handle(DNSServerType::HTTP_DNS, http_dns);
            Logger::log(LogLevel::INFO, "MMDNSEntrance", "已启用HTTP DNS: " + doh_server_);
        }

        if (local_cache_enabled_) {
            auto local_dns = std::make_shared<DNSLocalServerHandle>();
            dns_server_->add_server_handle(DNSServerType::LOCAL, local_dns);
            Logger::log(LogLevel::INFO, "MMDNSEntrance", "已启用本地缓存");
        }
    }

    std::string DNSEntranceImpl::resolve_host(const std::string &hostname) {
        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "MMDNSEntrance", "DNS服务未初始化");
            return "";
        }

        Logger::log(LogLevel::INFO, "MMDNSEntrance", "解析主机: " + hostname);

        auto host = dns_server_->resolve_sync(hostname);
        if (host && host->has_valid_ip()) {
            std::string best_ip = host->get_bast_ip_string();
            Logger::log(LogLevel::INFO, "MMDNSEntrance",
                        "解析成功: " + hostname + " -> " + best_ip);
            return best_ip;
        }

        Logger::log(LogLevel::WARN, "MMDNSEntrance", "解析失败: " + hostname);
        return "";
    }

    void
    DNSEntranceImpl::resolve_host_async(const std::string &hostname, dns::TaskCallback callback) {
        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "MMDNSEntrance", "DNS服务未初始化");
            if (callback) {
                callback(nullptr, false, nullptr);
            }
            return;
        }

        Logger::log(LogLevel::INFO, "MMDNSEntrance", "异步解析: " + hostname);
        dns_server_->resolve_async(hostname, callback);
    }

    std::vector<std::string> DNSEntranceImpl::get_all_ips(const std::string &hostname) {
        std::vector<std::string> ips;

        if (!dns_server_) {
            Logger::log(LogLevel::ERROR, "MMDNSEntrance", "DNS服务未初始化");
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
        Logger::log(LogLevel::INFO, "MMDNSEntrance",
                    "网络状态变更: " + std::to_string(static_cast<int>(state)));

        if (state == DNSAppNetState::NONE) {
            if (dns_manager_) {
                dns_manager_->save_to_cache();
            }
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

        Logger::log(LogLevel::INFO, "MMDNSEntrance", "清空所有缓存");
    }

    void DNSEntranceImpl::set_doh_server(const std::string &server) {
        std::lock_guard<std::mutex> lock(mutex_);

        doh_server_ = server;
        Logger::log(LogLevel::INFO, "MMDNSEntrance", "设置DoH服务器: " + server);

        if (dns_server_ && http_dns_enabled_) {
            auto http_dns = std::make_shared<DNSHttpServerHandle>(server);
            dns_server_->add_server_handle(DNSServerType::HTTP_DNS, http_dns);
        }
    }

    void DNSEntranceImpl::enable_system_dns(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);

        system_dns_enabled_ = enable;
        Logger::log(LogLevel::INFO, "MMDNSEntrance",
                    std::string("系统DNS: ") + (enable ? "启用" : "禁用"));
    }


    void DNSEntranceImpl::enable_http_dns(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);

        http_dns_enabled_ = enable;
        Logger::log(LogLevel::INFO, "MMDNSEntrance",
                    std::string("HTTP DNS: ") + (enable ? "启用" : "禁用"));
    }

    void DNSEntranceImpl::enable_local_cache(bool enable) {
        std::lock_guard<std::mutex> lock(mutex_);

        local_cache_enabled_ = enable;
        Logger::log(LogLevel::INFO, "MMDNSEntrance",
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
        //可以存储到配置中，在创建LocalServerHandle时使用
        Logger::log(LogLevel::INFO, "MMDNSEntrance",
                    "设置缓存过期时间: " + std::to_string(seconds) + "秒");
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
}