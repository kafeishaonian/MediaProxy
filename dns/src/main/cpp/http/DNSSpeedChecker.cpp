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

        Logger::log(LogLevel::INFO, "DNSSpeedChecker",
                    "开始检测 " + std::to_string(ip_list.size()) + " 个IP的速度");

        std::vector<std::future<void>> futures;
        for (auto &ip_model: ip_list) {
            futures.push_back(std::async(std::launch::async, [this, ip_model, port]() {
                int speed = check_speed(ip_model->get_ip(), port);
                ip_model->set_speed(speed);

                if (speed >= 0) {
                    Logger::log(LogLevel::DEBUG, "DNSSpeedChecker",
                                "IP: " + ip_model->get_ip() + " 速度: " + std::to_string(speed) +
                                "ms");
                }
            }));
        }

        for (auto &future: futures) {
            future.wait();
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
        bool contented = socket.connect(ip, port, timeout_);
        auto end_time = std::chrono::steady_clock::now();
        if (!contented) {
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
}