//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSHostManager.h"

#include <chrono>
#include <algorithm>
#include <thread>

namespace dns {


    DNSHostManager::DNSHostManager() {
        Logger::log(LogLevel::INFO, "DNSHostManager", "主机管理器已创建");
    }

    DNSHostManager::~DNSHostManager() {
        clear_cache();
        Logger::log(LogLevel::INFO, "DNSHostManager", "主机管理器已销毁");
    }

    std::shared_ptr<dns::DNSHostModel> DNSHostManager::get_host(const std::string &hostname) {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto it = host_cache_.find(hostname);
        if (it != host_cache_.end()) {
            Logger::log(LogLevel::DEBUG, "DNSHostManager",
                        "获取主机: " + hostname);
            return it->second;
        }

        if (data_cache_) {
            std::string json_data = data_cache_->load(hostname);
            if (!json_data.empty()) {
                auto host = DNSHostModel::from_json(json_data);
                if (host) {
                    host_cache_[hostname] = host;
                    Logger::log(LogLevel::DEBUG, "DNSHostManager",
                                "从持久化缓存加载: " + hostname);
                    return host;
                }
            }
        }

        Logger::log(LogLevel::DEBUG, "DNSHostManager",
                    "主机不存在: " + hostname);
        return nullptr;
    }

    void DNSHostManager::update_host(std::shared_ptr<dns::DNSHostModel> host) {
        if (!host) {
            return;
        }

        std::lock_guard<std::mutex> lock(cache_mutex_);

        std::string hostname = host->get_hostname();
        host_cache_[hostname] = host;

        Logger::log(LogLevel::DEBUG, "DNSHostManager",
                    "更新主机: " + hostname);
        if (data_cache_) {
            std::thread([this, hostname, host]() {
                try {
                    std::string json_data = host->to_json();
                    data_cache_->save(hostname, json_data);
                } catch (const std::exception &e) {
                    Logger::log(LogLevel::ERROR, "DNSHostManager",
                                "保存缓存失败: " + std::string(e.what()));
                }
            }).detach();
        }
    }

    void DNSHostManager::remove_host(const std::string &hostname) {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        host_cache_.erase(hostname);
        if (data_cache_) {
            data_cache_->remove(hostname);
        }

        Logger::log(LogLevel::DEBUG, "DNSHostManager",
                    "删除主机: " + hostname);
    }

    void DNSHostManager::clear_cache() {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        int count = host_cache_.size();
        host_cache_.clear();
        if (data_cache_) {
            data_cache_->clear();
        }

        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "清空缓存，删除 " + std::to_string(count) + " 个主机");
    }

    std::vector<std::shared_ptr<dns::DNSHostModel>> DNSHostManager::get_all_hosts() {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        std::vector<std::shared_ptr<DNSHostModel>> hosts;
        hosts.reserve(host_cache_.size());

        for (const auto &pair: host_cache_) {
            hosts.push_back(pair.second);
        }
        return hosts;
    }


    int DNSHostManager::get_host_count() const {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return host_cache_.size();
    }

    void DNSHostManager::set_data_cache(std::shared_ptr<dns::DNSDataCache> cache) {
        data_cache_ = cache;
    }

    void DNSHostManager::load_from_cache() {
        if (!data_cache_) {
            Logger::log(LogLevel::WARN, "DNSHostManager",
                        "数据缓存未设置，无法加载");
            return;
        }

        Logger::log(LogLevel::INFO, "DNSHostManager", "开始加载缓存");

        // TODO: 实现从文件系统加载所有缓存的主机
        // 这需要DataCache支持列出所有缓存的key

        Logger::log(LogLevel::INFO, "DNSHostManager", "缓存加载完成");
    }

    void DNSHostManager::save_to_cache() {
        if (!data_cache_) {
            Logger::log(LogLevel::WARN, "DNSHostManager",
                        "数据缓存未设置，无法保存");
            return;
        }

        std::lock_guard<std::mutex> lock(cache_mutex_);
        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "开始保存缓存，共 " + std::to_string(host_cache_.size()) + " 个主机");

        int save_count = 0;
        for (const auto &pair: host_cache_) {
            try {
                std::string json_data = pair.second->to_json();
                data_cache_->save(pair.first, json_data);
                save_count++;
            } catch (const std::exception &e) {
                Logger::log(LogLevel::ERROR, "DNSHostManager",
                            "保存失败 " + pair.first + ": " + e.what());
            }
        }
        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "缓存保存完成，成功 " + std::to_string(save_count) + " 个");
    }

    void DNSHostManager::clean_expired_hosts(int expire_seconds) {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();

        int clean_count = 0;
        auto it = host_cache_.begin();
        while (it != host_cache_.end()) {
            if (is_host_expired(it->second, expire_seconds)) {
                if (data_cache_) {
                    data_cache_->remove(it->first);
                }
                it = host_cache_.erase(it);
                clean_count++;
            } else {
                ++it;
            }
        }

        if (clean_count > 0) {
            Logger::log(LogLevel::INFO, "DNSHostManager",
                        "清理过期主机 " + std::to_string(clean_count) + " 个");
        }
    }

    DNSHostManager::CacheStats DNSHostManager::get_stats() const {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        CacheStats stats;
        stats.total_hosts = host_cache_.size();
        stats.valid_hosts = 0;
        stats.expired_hosts = 0;
        stats.oldest_timestamp = LONG_MAX;
        stats.newest_timestamp = 0;

        for (const auto &pair: host_cache_) {
            auto host = pair.second;
            long update_time = host->get_update_time();

            if (host->has_valid_ip()) {
                stats.valid_hosts++;
            }

            if (update_time < stats.oldest_timestamp) {
                stats.oldest_timestamp = update_time;
            }

            if (update_time > stats.newest_timestamp) {
                stats.newest_timestamp = update_time;
            }
        }

        stats.expired_hosts = stats.total_hosts - stats.valid_hosts;

        return stats;
    }

    bool DNSHostManager::is_host_expired(const std::shared_ptr<dns::DNSHostModel> &host,
                                         int expire_seconds) {
        if (!host) {
            return true;
        }

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();

        long age = now - host->get_update_time();
        return age > expire_seconds;
    }

}
