//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSServer.h"

#include <algorithm>
#include <future>

namespace dns {
    DNSServer::DNSServer()
            : running_(false),
              thread_count_(Constants::DEFAULT_THREAD_COUNT),
              total_requests_(0),
              success_requests_(0),
              failed_requests_(0),
              cached_requests_(0) {
        task_queue_ = std::make_shared<DNSBlockingQueue<std::shared_ptr<DNSServerTask>>>();
        host_manager_ = std::make_shared<DNSHostManager>();
        speed_checker_ = std::make_shared<DNSSpeedChecker>();
    }

    DNSServer::~DNSServer() {
        stop();
    }

    void DNSServer::start() {
        if (running_.load()) {
            Logger::log(LogLevel::WARN, "DNSServer", "服务器已在运行中");
            return;
        }
        Logger::log(LogLevel::INFO, "DNSServer", "启动DNS服务器...");
        running_.store(true);

        for (int i = 0; i < thread_count_; i++) {
            worker_threads_.push_back(std::make_shared<std::thread>(&DNSServer::worker_loop, this));
        }
        Logger::log(LogLevel::INFO, "DNSServer",
                    "DNS服务器已启动，工作线程数: " + std::to_string(thread_count_));
    }

    void DNSServer::stop() {
        if (!running_.load()) {
            return;
        }
        Logger::log(LogLevel::INFO, "DNSServer", "停止DNS服务器...");
        running_.store(false);

        for (size_t i = 0; i < worker_threads_.size(); i++) {
            task_queue_->try_put(nullptr);
        }

        for (auto &thread: worker_threads_) {
            if (thread && thread->joinable()) {
                thread->join();
            }
        }

        worker_threads_.clear();

        task_queue_->clear();
        Logger::log(LogLevel::INFO, "DNSServer", "DNS服务器已停止");
    }

    bool DNSServer::is_running() const {
        return running_.load();
    }

    void DNSServer::worker_loop() {
        Logger::log(LogLevel::DEBUG, "DNSServer", "工作线程启动");

        while (running_.load()) {
            try {
                auto task = task_queue_->task(std::chrono::milliseconds(1000));
                if (task.has_value() && task.value()) {
                    process_task(task.value());
                }
            } catch (const std::exception& e) {
                Logger::log(LogLevel::ERROR, "DNSServer",
                            std::string("工作线程异常: ") + e.what());
            }
        }
        Logger::log(LogLevel::DEBUG, "DNSServer", "工作线程退出");
    }

    void DNSServer::process_task(std::shared_ptr<dns::DNSServerTask> task) {
        if (!task) {
            return;
        }

        Logger::log(LogLevel::DEBUG, "DNSServer",
                    "处理任务: " + task->get_hostname());

        total_requests_++;

        try {
            task->execute();
            success_requests_++;
        } catch (const std::exception& e) {
            failed_requests_++;
            Logger::log(LogLevel::ERROR, "DNSServer",
                        "任务执行失败: " + std::string(e.what()));
        }
    }

    std::shared_ptr<dns::DNSHostModel> DNSServer::resolve_sync(const std::string &hostname) {
        Logger::log(LogLevel::INFO, "DNSServer", "同步解析: " + hostname);

        auto cached_host = host_manager_->get_host(hostname);
        if (cached_host && cached_host->has_valid_ip()) {
            Logger::log(LogLevel::INFO, "DNSServer", "从缓存获取: " + hostname);
            cached_requests_++;
            return cached_host;
        }

        return perform_resolve(hostname);
    }

    void DNSServer::resolve_async(const std::string &hostname, dns::TaskCallback callback) {
        Logger::log(LogLevel::INFO, "DNSServer", "异步解析: " + hostname);

        // 创建 ResolveHostTask，注入 perform_resolve 函数
        auto resolve_func = [this](const std::string& host) {
            return this->perform_resolve(host);
        };

        auto task = std::make_shared<ResolveHostTask>(hostname, callback, resolve_func);
        task_queue_->put(task);
    }

    std::shared_ptr<dns::DNSHostModel> DNSServer::perform_resolve(const std::string &hostname) {
        std::shared_ptr<DNSHostModel> best_result = nullptr;
        std::atomic<bool> resolved{false};
        std::mutex result_mutex;
        std::vector<std::future<void>> futures;

        // Fixed: Copy handlers under lock to avoid race condition
        std::vector<std::shared_ptr<DNSServerHandle>> handlers;
        {
            std::lock_guard<std::mutex> lock(handlers_mutex_);
            for (auto& pair : server_handlers_) {
                handlers.push_back(pair.second);
            }
        }

        for (auto& handler : handlers) {
            futures.push_back(std::async(std::launch::async, [&, handler]() {
                if (resolved.load(std::memory_order_acquire)) {
                    return;
                }

                try {
                    auto result = handler->resolve(hostname);
                    if (result && result->has_valid_ip()) {
                        std::lock_guard<std::mutex> lock(result_mutex);
                        if (!resolved.load(std::memory_order_acquire)) {
                            best_result = result;
                            resolved.store(true, std::memory_order_release);
                        }
                    }
                } catch (const std::exception& e) {
                    Logger::log(LogLevel::DEBUG, "DNSServer",
                        "DNS解析异常: " + std::string(e.what()));
                }
            }));
        }

        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(Constants::RESOLVE_TIMEOUT_SECONDS);
        while (!resolved.load(std::memory_order_acquire) &&
               std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        resolved.store(true, std::memory_order_release);

        // Fixed: Use wait_for with timeout to avoid blocking indefinitely
        for (auto& future : futures) {
            if (future.valid()) {
                future.wait_for(std::chrono::milliseconds(100));
            }
        }

        if (best_result) {
            if (speed_checker_) {
                auto ip_list = best_result->get_ip_list();
                speed_checker_->check_multiple(ip_list);
                best_result->sort_by_speed();
            }

            if (host_manager_) {
                host_manager_->update_host(best_result);
            }

            if (running_.load() && speed_checker_ && host_manager_) {
                auto speed_task = std::make_shared<SpeedCheckTask>(
                    hostname, best_result, speed_checker_, host_manager_, nullptr
                );
                speed_task->set_priority(3);
                task_queue_->try_put(speed_task);
            }
        } else {
            if (host_manager_) {
                host_manager_->remove_host(hostname);
                Logger::log(LogLevel::DEBUG, "DNSServer", "解析失败，移除缓存: " + hostname);
            }
        }

        return best_result;
    }


    void DNSServer::add_server_handle(dns::DNSServerType type,
                                      std::shared_ptr<dns::DNSServerHandle> handle) {
        if (handle) {
            handle->set_host_manager(host_manager_);
            std::lock_guard<std::mutex> lock(handlers_mutex_);  // Fixed: add lock
            server_handlers_[type] = handle;
            Logger::log(LogLevel::INFO, "DNSServer",
                        "添加DNS处理器: " + std::to_string(static_cast<int>(type)));
        }
    }

    void DNSServer::remove_server_handle(dns::DNSServerType type) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);  // Fixed: add lock
        server_handlers_.erase(type);
    }

    void DNSServer::set_thread_count(int count) {
        if (count > 0 && count <= 16) {
            thread_count_ = count;
        }
    }

    void DNSServer::set_queue_size(size_t size) {
        if (!running_.load()) {
            task_queue_ = std::make_shared<DNSBlockingQueue<std::shared_ptr<DNSServerTask>>>(size);
        }
    }

    void DNSServer::set_speed_checker(std::shared_ptr<dns::DNSSpeedChecker> checker) {
        speed_checker_ = checker;
    }

    void DNSServer::set_host_manager(std::shared_ptr<dns::DNSHostManager> manager) {
        host_manager_ = manager;
    }

    std::shared_ptr<dns::DNSHostManager> DNSServer::get_host_manager() const {
        return host_manager_;
    }


    DNSServer::ServerStats DNSServer::get_stats() const {
        ServerStats stats;
        stats.total_requests = total_requests_.load();
        stats.success_requests = success_requests_.load();
        stats.failed_requests = failed_requests_.load();
        stats.cached_requests = cached_requests_.load();
        stats.queue_size = task_queue_->size();
        stats.thread_count = worker_threads_.size();
        return stats;
    }

    void DNSServer::submit_speed_check_task(
            const std::string& hostname,
            std::shared_ptr<DNSHostModel> host_model,
            TaskCallback callback) {

        if (!running_.load()) {
            Logger::log(LogLevel::WARN, "DNSServer", "服务器未运行，无法提交速度检测任务");
            return;
        }

        if (!host_model || !speed_checker_ || !host_manager_) {
            Logger::log(LogLevel::ERROR, "DNSServer", "速度检测任务参数不完整");
            return;
        }

        Logger::log(LogLevel::DEBUG, "DNSServer", "提交速度检测任务: " + hostname);

        auto task = std::make_shared<SpeedCheckTask>(
            hostname,
            host_model,
            speed_checker_,
            host_manager_,
            callback
        );

        task_queue_->try_put(task);
    }

    void DNSServer::submit_cache_update_task(
            const std::string& hostname,
            std::shared_ptr<DNSHostModel> host_model) {

        if (!running_.load()) {
            Logger::log(LogLevel::WARN, "DNSServer", "服务器未运行，无法提交缓存更新任务");
            return;
        }

        if (!host_model || !host_manager_) {
            Logger::log(LogLevel::ERROR, "DNSServer", "缓存更新任务参数不完整");
            return;
        }

        Logger::log(LogLevel::DEBUG, "DNSServer", "提交缓存更新任务: " + hostname);

        auto task = std::make_shared<CacheUpdateTask>(
            hostname,
            host_model,
            host_manager_
        );

        task_queue_->try_put(task);
    }
}