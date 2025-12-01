//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSHostManager.h"
#include "DNSRAIIUtils.h"

#include <chrono>
#include <algorithm>
#include <thread>
#include <fstream>
#include <sstream>
#include <cstring>

namespace dns {


    DNSHostManager::DNSHostManager() {
        Logger::log(LogLevel::INFO, "DNSHostManager", "主机管理器已创建");
    }

    DNSHostManager::~DNSHostManager() {
        clear_cache();
        Logger::log(LogLevel::INFO, "DNSHostManager", "主机管理器已销毁");
    }

    std::shared_ptr<dns::DNSHostModel> DNSHostManager::get_host(const std::string &hostname) {
        {
            std::shared_lock<std::shared_mutex> lock(cache_mutex_);
            auto it = host_cache_.find(hostname);
            if (it != host_cache_.end()) {
                Logger::log(LogLevel::DEBUG, "DNSHostManager", "获取主机: " + hostname);
                return it->second;
            }
        }

        if (data_cache_) {
            std::string json_data = data_cache_->load(hostname);
            if (!json_data.empty()) {
                auto host = DNSHostModel::from_json(json_data);
                if (host) {
                    std::unique_lock<std::shared_mutex> lock(cache_mutex_);
                    host_cache_[hostname] = host;
                    Logger::log(LogLevel::DEBUG, "DNSHostManager", "从持久化缓存加载: " + hostname);
                    return host;
                }
            }
        }

        Logger::log(LogLevel::DEBUG, "DNSHostManager", "主机不存在: " + hostname);
        return nullptr;
    }

    void DNSHostManager::update_host(std::shared_ptr<dns::DNSHostModel> host) {
        if (!host) {
            return;
        }

        std::string hostname = host->get_hostname();

        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            host_cache_[hostname] = host;
        }

        Logger::log(LogLevel::DEBUG, "DNSHostManager", "更新主机: " + hostname);

        if (data_cache_) {
            // Fixed: Use shared_ptr to manage lifetime properly in detached thread
            auto cache_ptr = data_cache_;  // Keep shared_ptr alive
            std::thread([cache_ptr, hostname, host]() {
                try {
                    std::string json_data = host->to_json();
                    cache_ptr->save(hostname, json_data);
                } catch (const std::exception &e) {
                    Logger::log(LogLevel::ERROR, "DNSHostManager",
                                "保存缓存失败: " + std::string(e.what()));
                }
            }).detach();
        }
    }

    void DNSHostManager::remove_host(const std::string &hostname) {
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            host_cache_.erase(hostname);
        }

        if (data_cache_) {
            data_cache_->remove(hostname);
        }

        Logger::log(LogLevel::DEBUG, "DNSHostManager", "删除主机: " + hostname);
    }

    void DNSHostManager::clear_cache() {
        int count;
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            count = host_cache_.size();
            host_cache_.clear();
        }

        if (data_cache_) {
            data_cache_->clear();
        }

        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "清空缓存，删除 " + std::to_string(count) + " 个主机");
    }

    std::vector<std::shared_ptr<dns::DNSHostModel>> DNSHostManager::get_all_hosts() {
        std::shared_lock<std::shared_mutex> lock(cache_mutex_);
        std::vector<std::shared_ptr<DNSHostModel>> hosts;
        hosts.reserve(host_cache_.size());
        for (const auto &pair: host_cache_) {
            hosts.push_back(pair.second);
        }
        return hosts;
    }

    int DNSHostManager::get_host_count() const {
        std::shared_lock<std::shared_mutex> lock(cache_mutex_);
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

        std::string cache_dir = data_cache_->get_cache_dir();

        // Use RAII wrapper for DIR*
        DirectoryHandle dir(cache_dir);
        if (!dir.is_open()) {
            Logger::log(LogLevel::WARN, "DNSHostManager", "无法打开缓存目录: " + cache_dir);
            return;
        }

        int loaded_count = 0;
        int failed_count = 0;

        struct dirent *entry;
        while ((entry = dir.read()) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            std::string filename = entry->d_name;
            if (filename.length() < 6 || filename.substr(filename.length() - 6) != ".cache") {
                continue;
            }

            try {
                std::string file_path = cache_dir + "/" + filename;

                // Use RAII wrapper for file reading with size check
                SafeFileReader reader(file_path);
                if (!reader.is_valid()) {
                    Logger::log(LogLevel::WARN, "DNSHostManager",
                               "缓存文件无效或过大: " + filename +
                               " (" + std::to_string(reader.file_size()) + " bytes)");
                    failed_count++;
                    continue;
                }

                std::string json_data = reader.read_all();

                if (json_data.empty()) {
                    failed_count++;
                    continue;
                }

                // 从 JSON 反序列化
                auto host = DNSHostModel::from_json(json_data);
                if (host && host->has_valid_ip()) {
                    std::string hostname = host->get_hostname();
                    {
                        std::unique_lock<std::shared_mutex> lock(cache_mutex_);
                        host_cache_[hostname] = host;
                    }
                    loaded_count++;

                    Logger::log(LogLevel::DEBUG, "DNSHostManager",
                                "加载缓存: " + hostname + " (" +
                                std::to_string(host->get_ip_list().size()) + " 个IP)");
                } else {
                    failed_count++;
                }
            } catch (const std::exception &e) {
                failed_count++;
                Logger::log(LogLevel::ERROR, "DNSHostManager",
                            "加载缓存文件失败: " + filename +
                            ", 错误: " + e.what());
            }
        }

        // dir will be automatically closed by RAII destructor

        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "缓存加载完成，成功: " + std::to_string(loaded_count) +
                    ", 失败: " + std::to_string(failed_count));
    }

    void DNSHostManager::save_to_cache() {
        if (!data_cache_) {
            Logger::log(LogLevel::WARN, "DNSHostManager", "数据缓存未设置，无法保存");
            return;
        }

        std::vector<std::pair<std::string, std::shared_ptr<DNSHostModel>>> hosts_copy;
        {
            std::shared_lock<std::shared_mutex> lock(cache_mutex_);
            hosts_copy.reserve(host_cache_.size());
            for (const auto &pair: host_cache_) {
                hosts_copy.push_back(pair);
            }
        }

        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "开始保存缓存，共 " + std::to_string(hosts_copy.size()) + " 个主机");

        int save_count = 0;
        for (const auto &pair: hosts_copy) {
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
        std::vector<std::string> expired_hosts;
        {
            std::shared_lock<std::shared_mutex> lock(cache_mutex_);
            for (const auto& pair : host_cache_) {
                if (is_host_expired(pair.second, expire_seconds)) {
                    expired_hosts.push_back(pair.first);
                }
            }
        }

        if (!expired_hosts.empty()) {
            std::unique_lock<std::shared_mutex> lock(cache_mutex_);
            for (const auto& hostname : expired_hosts) {
                host_cache_.erase(hostname);
                if (data_cache_) {
                    data_cache_->remove(hostname);
                }
            }
            Logger::log(LogLevel::INFO, "DNSHostManager",
                        "清理过期主机 " + std::to_string(expired_hosts.size()) + " 个");
        }
    }

    DNSHostManager::CacheStats DNSHostManager::get_stats() const {
        std::shared_lock<std::shared_mutex> lock(cache_mutex_);

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

    void DNSHostManager::set_speed_check_callback(SpeedCheckCallback callback) {
        speed_check_callback_ = std::move(callback);
        Logger::log(LogLevel::INFO, "DNSHostManager", "已设置速度检测回调");
    }

    void DNSHostManager::trigger_speed_check_for_all() {
        if (!speed_check_callback_) {
            Logger::log(LogLevel::WARN, "DNSHostManager", "速度检测回调未设置");
            return;
        }

        std::vector<std::pair<std::string, std::shared_ptr<DNSHostModel>>> hosts_copy;
        {
            std::shared_lock<std::shared_mutex> lock(cache_mutex_);
            for (const auto& pair : host_cache_) {
                if (pair.second && pair.second->has_valid_ip()) {
                    hosts_copy.emplace_back(pair);
                }
            }
        }

        Logger::log(LogLevel::INFO, "DNSHostManager",
                    "触发所有缓存主机的速度检测，共 " + std::to_string(hosts_copy.size()) + " 个");

        for (const auto& pair : hosts_copy) {
            speed_check_callback_(pair.first, pair.second);
        }
    }

}
