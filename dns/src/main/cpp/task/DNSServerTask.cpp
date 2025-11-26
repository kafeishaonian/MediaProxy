//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSServerTask.h"

#include <chrono>

#include "DNSServer.h"

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

    ResolveHostTask::ResolveHostTask(
            const std::string &hostname,
            dns::TaskCallback callback
    ) : DNSServerTask(DNSServerTaskType::RESOLVE_HOST, hostname, callback) {
        priority_ = 10;
    }

    void ResolveHostTask::execute() {
        Logger::log(LogLevel::INFO, "ResolveHostTask",
                    "开始解析主机: " + hostname_);

        try {
            //这里需要访问DNSServer实例来执行解析
            // 由于execute是在server的工作线程中调用的，
            // 实际实现中会通过回调或其他方式获取server引用

            // 简化实现：直接调用回调
            if (callback_) {
                // 实际应该从DNS服务器获取结果
                auto result = std::make_shared<DNSHostModel>(hostname_);
                callback_(result, false, nullptr);
            }

            Logger::log(LogLevel::INFO, "ResolveHostTask",
                        "主机解析完成: " + hostname_);
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "ResolveHostTask",
                        "解析失败: " + std::string(e.what()));

            if (callback_) {
                callback_(nullptr, false, nullptr);
            }
        }
    }


    SpeedCheckTask::SpeedCheckTask(
            const std::string &hostname,
            std::shared_ptr<dns::DNSHostModel> host_model,
            dns::TaskCallback callback
    ) : DNSServerTask(DNSServerTaskType::SPEED_CHECK, hostname, callback),
        host_model_(host_model) {
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

        Logger::log(LogLevel::INFO, "SpeedCheckTask",
                    "开始测速: " + hostname_ + " (" +
                    std::to_string(host_model_->get_ip_list().size()) + " 个IP)");

        try {
            // 实际实现中会使用DNSSpeedChecker
            // 这里简化处理
            auto ipList = host_model_->get_ip_list();

            // 模拟测速
            for (auto &ip: ipList) {
                // 实际应该调用speedChecker->checkSpeed()
                ip->set_speed(100);  // 模拟速度
            }

            // 排序
            host_model_->sort_by_speed();

            Logger::log(LogLevel::INFO, "SpeedCheckTask", "测速完成");

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

    CacheUpdateTask::CacheUpdateTask(
            const std::string &hostname,
            std::shared_ptr<dns::DNSHostModel> host_mode
    ) : DNSServerTask(DNSServerTaskType::CACHE_UPDATE, hostname, nullptr),
        host_model_(host_mode) {
        priority_ = 1;
    }


    void CacheUpdateTask::execute() {
        if (!host_model_) {
            Logger::log(LogLevel::ERROR, "CacheUpdateTask", "主机模型为空");
            return;
        }

        Logger::log(LogLevel::DEBUG, "CacheUpdateTask",
                    "更新缓存: " + hostname_);

        try {
            // 实际实现中会调用DNSHostManager::updateHost()
            // 这里简化处理

            Logger::log(LogLevel::DEBUG, "CacheUpdateTask", "缓存更新完成");
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "CacheUpdateTask",
                        "缓存更新失败: " + std::string(e.what()));
        }
    }
}
