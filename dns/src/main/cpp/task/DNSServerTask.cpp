//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSServerTask.h"

#include <chrono>

#include "DNSHostManager.h"
#include "DNSSpeedChecker.h"

namespace dns {
    DNSServerTask::DNSServerTask(
            dns::DNSServerTaskType task_type,
            const std::string &hostname,
            dns::TaskCallback callback
    ) : task_type_(task_type),
        hostname_(hostname),
        callback_(callback),
        priority_(0) {

        create_time_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    DNSServerTaskType DNSServerTask::get_task_type() const {
        return task_type_;
    }

    std::string DNSServerTask::get_hostname() const {
        return hostname_;
    }

    TaskCallback DNSServerTask::get_callback() const {
        return callback_;
    }

    long DNSServerTask::get_create_time() const {
        return create_time_;
    }

    int DNSServerTask::get_priority() const {
        return priority_;
    }

    void DNSServerTask::set_priority(int priority) {
        priority_ = priority;
    }

    void DNSServerTask::set_callback(dns::TaskCallback callback) {
        callback_ = callback;
    }

    bool DNSServerTask::operator<(const dns::DNSServerTask &other) const {
        return priority_ < other.priority_;
    }

    // ========== ResolveHostTask 实现 ==========
    ResolveHostTask::ResolveHostTask(
            const std::string &hostname,
            dns::TaskCallback callback,
            std::function<std::shared_ptr<DNSHostModel>(const std::string&)> resolve_func
    ) : DNSServerTask(DNSServerTaskType::RESOLVE_HOST, hostname, callback),
        resolve_func_(resolve_func) {
        priority_ = 10;
    }

    void ResolveHostTask::execute() {
        Logger::log(LogLevel::INFO, "ResolveHostTask",
                    "开始解析主机: " + hostname_);

        try {
            if (!resolve_func_) {
                Logger::log(LogLevel::ERROR, "ResolveHostTask", "解析函数未设置");
                if (callback_) {
                    callback_(nullptr, false, nullptr);
                }
                return;
            }

            // 调用注入的解析函数（实际上是 DNSServer::perform_resolve）
            auto result = resolve_func_(hostname_);

            if (result && result->has_valid_ip()) {
                Logger::log(LogLevel::INFO, "ResolveHostTask",
                            "主机解析成功: " + hostname_ + " -> " +
                            std::to_string(result->get_ip_list().size()) + " 个IP");

                if (callback_) {
                    callback_(result, true, nullptr);
                }
            } else {
                Logger::log(LogLevel::WARN, "ResolveHostTask",
                            "主机解析失败或无有效IP: " + hostname_);

                if (callback_) {
                    callback_(result, false, nullptr);
                }
            }

        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "ResolveHostTask",
                        "解析异常: " + std::string(e.what()));

            if (callback_) {
                callback_(nullptr, false, nullptr);
            }
        }
    }

    // ========== SpeedCheckTask 实现 ==========
    SpeedCheckTask::SpeedCheckTask(
            const std::string &hostname,
            std::shared_ptr<dns::DNSHostModel> host_model,
            std::shared_ptr<DNSSpeedChecker> speed_checker,
            std::shared_ptr<DNSHostManager> host_manager,
            dns::TaskCallback callback
    ) : DNSServerTask(DNSServerTaskType::SPEED_CHECK, hostname, callback),
        host_model_(host_model),
        speed_checker_(speed_checker),
        host_manager_(host_manager) {
        priority_ = 5;
    }

    void SpeedCheckTask::execute() {
        if (!host_model_) {
            Logger::log(LogLevel::ERROR, "SpeedCheckTask", "主机模型为空");
            if (callback_) {
                callback_(nullptr, false, nullptr);
            }
            return;
        }

        if (!speed_checker_) {
            Logger::log(LogLevel::ERROR, "SpeedCheckTask", "速度检测器未设置");
            if (callback_) {
                callback_(host_model_, false, nullptr);
            }
            return;
        }

        auto ip_list = host_model_->get_ip_list();
        if (ip_list.empty()) {
            Logger::log(LogLevel::WARN, "SpeedCheckTask", "IP列表为空: " + hostname_);
            if (callback_) {
                callback_(host_model_, false, nullptr);
            }
            return;
        }

        Logger::log(LogLevel::INFO, "SpeedCheckTask",
                    "开始测速: " + hostname_ + " (" +
                    std::to_string(ip_list.size()) + " 个IP)");

        try {
            // 使用 DNSSpeedChecker 进行批量速度检测
            speed_checker_->check_multiple(ip_list);

            // 按速度排序
            host_model_->sort_by_speed();

            // 更新到缓存
            if (host_manager_) {
                host_manager_->update_host(host_model_);
            }

            Logger::log(LogLevel::INFO, "SpeedCheckTask",
                        "测速完成: " + hostname_ + ", 最快IP: " +
                        host_model_->get_best_ip_string());

            if (callback_) {
                callback_(host_model_, true, nullptr);
            }
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "SpeedCheckTask",
                        "测速失败: " + std::string(e.what()));

            if (callback_) {
                callback_(host_model_, false, nullptr);
            }
        }
    }

    // ========== CacheUpdateTask 实现 ==========
    CacheUpdateTask::CacheUpdateTask(
            const std::string &hostname,
            std::shared_ptr<dns::DNSHostModel> host_model,
            std::shared_ptr<DNSHostManager> host_manager
    ) : DNSServerTask(DNSServerTaskType::CACHE_UPDATE, hostname, nullptr),
        host_model_(host_model),
        host_manager_(host_manager) {
        priority_ = 1;
    }

    void CacheUpdateTask::execute() {
        if (!host_model_) {
            Logger::log(LogLevel::ERROR, "CacheUpdateTask", "主机模型为空");
            return;
        }

        if (!host_manager_) {
            Logger::log(LogLevel::ERROR, "CacheUpdateTask", "主机管理器未设置");
            return;
        }

        Logger::log(LogLevel::DEBUG, "CacheUpdateTask",
                    "更新缓存: " + hostname_);

        try {
            // 调用 DNSHostManager::update_host() 更新缓存
            host_manager_->update_host(host_model_);

            Logger::log(LogLevel::DEBUG, "CacheUpdateTask",
                        "缓存更新完成: " + hostname_ + ", IP数量: " +
                        std::to_string(host_model_->get_ip_list().size()));
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "CacheUpdateTask",
                        "缓存更新失败: " + std::string(e.what()));
        }
    }
}
