//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSConnectionPool.h"

#include <algorithm>

namespace dns {

    DNSConnectionPool::DNSConnectionPool(
            int max_connections_per_host,
            int connection_timeout,
            int idle_timeout
    ) : max_connections_per_host_(max_connections_per_host),
        connection_timeout_(connection_timeout),
        idle_timeout_(idle_timeout) {
        Logger::log(LogLevel::INFO, "ConnectionPool",
                    "Initialized with max " + std::to_string(max_connections_per_host) +
                    " connections per host");
    }

    DNSConnectionPool::~DNSConnectionPool() {
        clear();
    }

    CURL *DNSConnectionPool::create_connection() {
        CURL *curl = curl_easy_init();
        if (!curl) {
            Logger::log(LogLevel::ERROR, "ConnectionPool", "CURL 加载失败");
            return nullptr;
        }
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        return curl;
    }

    std::string DNSConnectionPool::extract_host(const std::string &url) {
        size_t start = url.find("://");
        if (start == std::string::npos) {
            return url;
        }

        start += 3;
        size_t end = url.find('/', start);
        if (end == std::string::npos) {
            end = url.length();
        }
        return url.substr(start, end - start);
    }

    bool
    DNSConnectionPool::is_connection_expired(const dns::DNSConnectionPool::Connection &conn) const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - conn.last_used
        ).count();
        return elapsed > idle_timeout_;
    }

    CURL *DNSConnectionPool::acquire(const std::string &host) {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string host_key = extract_host(host);
        auto &pool = pools_[host_key];

        for (auto &conn: pool) {
            if (!conn.in_use && !is_connection_expired(conn)) {
                conn.in_use = true;
                conn.last_used = std::chrono::steady_clock::now();
                Logger::log(LogLevel::DEBUG, "ConnectionPool",
                            "重新连接的Key:= " + host_key);
                return conn.handle;
            }
        }

        if (pool.size() < static_cast<size_t>(max_connections_per_host_)) {
            CURL *new_handle = create_connection();
            if (new_handle) {
                Connection conn;
                conn.handle = new_handle;
                conn.in_use = true;
                conn.last_used = std::chrono::steady_clock::now();
                conn.host = host_key;
                pool.push_back(conn);

                Logger::log(LogLevel::DEBUG, "ConnectionPool",
                            "创建新的连接： " + host_key +
                            " (total: " + std::to_string(pool.size()) + ")");

                return new_handle;
            }
        }
        Logger::log(LogLevel::WARN, "ConnectionPool",
                    "没有可用的连接:= " + host_key);
        return nullptr;
    }

    void DNSConnectionPool::release(CURL *handle) {
        if (!handle) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        for (auto &pair: pools_) {
            for (auto &conn: pair.second) {
                if (conn.handle == handle) {
                    conn.in_use = false;
                    conn.last_used = std::chrono::steady_clock::now();
                    Logger::log(LogLevel::DEBUG, "ConnectionPool", "释放连接:= " + pair.first);
                    return;
                }
            }
        }
    }

    void DNSConnectionPool::cleanup() {
        std::lock_guard<std::mutex> lock(mutex_);

        int cleaned = 0;

        for (auto &pair: pools_) {
            auto &pool = pair.second;

            pool.erase(
                    std::remove_if(pool.begin(), pool.end(),
                                   [this, &cleaned](const Connection &conn) {
                                       if (!conn.in_use && is_connection_expired(conn)) {
                                           if (conn.handle) {
                                               curl_easy_cleanup(conn.handle);
                                           }
                                           cleaned++;
                                           return true;
                                       }
                                       return false;
                                   }),
                    pool.end()
            );
        }

        for (auto it = pools_.begin(); it != pools_.end();) {
            if (it->second.empty()) {
                it = pools_.erase(it);
            } else {
                ++it;
            }
        }

        if (cleaned > 0) {
            Logger::log(LogLevel::DEBUG, "ConnectionPool",
                        "清理" + std::to_string(cleaned) + "过期连接");
        }
    }

    void DNSConnectionPool::clear() {
        std::lock_guard<std::mutex> lock(mutex_);

        int count = 0;
        for (auto &pair: pools_) {
            for (auto &conn: pair.second) {
                if (conn.handle) {
                    curl_easy_cleanup(conn.handle);
                    count++;
                }
            }
        }

        pools_.clear();
        Logger::log(LogLevel::INFO, "ConnectionPool",
                    "清除所有连接 (" + std::to_string(count) + ")");
    }

    DNSConnectionPool::Stats DNSConnectionPool::get_state() const {
        std::lock_guard<std::mutex> lock(mutex_);

        Stats stats;
        stats.total_connections = 0;
        stats.active_connections = 0;
        stats.idle_connections = 0;
        stats.hosts_count = pools_.size();

        for (const auto &pair: pools_) {
            for (const auto &conn: pair.second) {
                stats.total_connections++;
                if (conn.in_use) {
                    stats.active_connections++;
                } else {
                    stats.idle_connections++;
                }
            }
        }
        return stats;
    }
}
