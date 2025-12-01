//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSSpeedChecker.h"

#include <future>
#include <algorithm>
#include <chrono>

namespace dns {
    DNSSpeedChecker::DNSSpeedChecker()
            : timeout_(Constants::DEFAULT_TIMEOUT_MS),
              retry_count_(Constants::DEFAULT_RETRY_COUNT) {
        pinger_ = std::make_shared<Ping>();
    }

    int DNSSpeedChecker::check_speed(const std::string &ip, int port) {
        int socket_speed = check_speed_with_socket(ip, port);
        if (socket_speed >= 0) {
            return socket_speed;
        }
        return check_speed_with_ping(ip);
    }

    void DNSSpeedChecker::set_timeout(int timeout_ms) {
        timeout_ = timeout_ms;
    }

    void DNSSpeedChecker::set_retry_count(int count) {
        retry_count_ = count;
    }

    int DNSSpeedChecker::get_timeout() const {
        return timeout_;
    }

    int DNSSpeedChecker::get_retry_count() const {
        return retry_count_;
    }

    void DNSSpeedChecker::check_multiple(std::vector<std::shared_ptr<dns::DNSIPModel>> &ip_list,
                                         int port) {
        if (ip_list.empty()) {
            return;
        }

        const int MAX_TEST = std::min(Constants::MAX_IP_SPEED_TEST, static_cast<int>(ip_list.size()));
        Logger::log(LogLevel::INFO, "DNSSpeedChecker",
                    "智能测速: 只测前 " + std::to_string(MAX_TEST) + " 个IP");

        std::vector<std::future<void>> futures;
        for (int i = 0; i < MAX_TEST; i++) {
            auto ip_model = ip_list[i];

            if (auto cached_speed = get_cached_speed(ip_model->get_ip())) {
                ip_model->set_speed(*cached_speed);
                Logger::log(LogLevel::DEBUG, "DNSSpeedChecker",
                            "使用缓存速度: " + ip_model->get_ip() + " = " +
                            std::to_string(*cached_speed) + "ms");
                continue;
            }

            futures.push_back(std::async(std::launch::async, [this, ip_model, port]() {
                int speed = check_speed(ip_model->get_ip(), port);
                ip_model->set_speed(speed);

                if (speed >= 0) {
                    cache_speed(ip_model->get_ip(), speed);
                    Logger::log(LogLevel::DEBUG, "DNSSpeedChecker",
                                "IP: " + ip_model->get_ip() + " 速度: " +
                                std::to_string(speed) + "ms");
                }
            }));
        }

        for (auto &future: futures) {
            future.wait();
        }

        for (size_t i = MAX_TEST; i < ip_list.size(); i++) {
            ip_list[i]->set_speed(-1);
        }

        std::sort(ip_list.begin(), ip_list.end(),
                  [](const std::shared_ptr<DNSIPModel> &a,
                     const std::shared_ptr<DNSIPModel> &b) {
                      if (!a->is_valid()) return false;
                      if (!b->is_valid()) return true;
                      int speed_a = a->get_speed();
                      int speed_b = b->get_speed();
                      if (speed_a < 0) return false;
                      if (speed_b < 0) return true;
                      return speed_a < speed_b;
                  });

        if (!ip_list.empty() && ip_list[0]->get_speed() >= 0) {
            Logger::log(LogLevel::INFO, "DNSSpeedChecker",
                        "最快IP: " + ip_list[0]->get_ip() + " (" +
                        std::to_string(ip_list[0]->get_speed()) + "ms)");
        }
    }

    int DNSSpeedChecker::check_speed_with_socket(const std::string &ip, int port) {
        DNSSocket socket;
        auto start_time = std::chrono::steady_clock::now();
        bool connected = socket.connect(ip, port, timeout_);  // Fixed: typo 'contented' -> 'connected'
        auto end_time = std::chrono::steady_clock::now();
        if (!connected) {
            return -1;
        }

        int latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time
        ).count();

        socket.close();
        return latency;
    }

    int DNSSpeedChecker::check_speed_with_ping(const std::string &ip) {
        PingResult result = pinger_->ping(ip, 3, timeout_);
        return result.avg_rtt;
    }

    std::optional<int> DNSSpeedChecker::get_cached_speed(const std::string& ip) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = speed_cache_.find(ip);
        if (it != speed_cache_.end()) {
            time_t now = time(nullptr);
            if (now - it->second.second < Constants::SPEED_CACHE_TTL_SECONDS) {
                return it->second.first;
            }
            speed_cache_.erase(it);
        }
        return std::nullopt;
    }

    void DNSSpeedChecker::cache_speed(const std::string& ip, int speed) {
        std::lock_guard<std::mutex> lock(cache_mutex_);

        // Fixed: Add cache size limit to prevent unbounded growth
        constexpr size_t MAX_CACHE_SIZE = 1000;
        if (speed_cache_.size() >= MAX_CACHE_SIZE) {
            // Remove oldest entries (simple FIFO eviction)
            time_t oldest_time = time(nullptr);
            std::string oldest_ip;

            for (const auto& entry : speed_cache_) {
                if (entry.second.second < oldest_time) {
                    oldest_time = entry.second.second;
                    oldest_ip = entry.first;
                }
            }

            if (!oldest_ip.empty()) {
                speed_cache_.erase(oldest_ip);
            }
        }

        speed_cache_[ip] = {speed, time(nullptr)};
    }
}