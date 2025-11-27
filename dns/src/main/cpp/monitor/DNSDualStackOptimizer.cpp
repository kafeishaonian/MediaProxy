//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSDualStackOptimizer.h"

#include <thread>
#include <future>
#include <chrono>
#include <algorithm>

#include "DNSSocket.h"

namespace dns {
    DNSDualStackOptimizer::DNSDualStackOptimizer()
            : prefer_ipv6_(false),
              connection_attempt_delay_(250),
              test_timeout_(2000) {
    }

    DNSDualStackOptimizer::TestResult
    DNSDualStackOptimizer::test_single_ip(const std::string &ip, int port) {
        TestResult result;
        result.ip = ip;
        result.success = false;
        result.rtt = -1;

        auto start_time = std::chrono::steady_clock::now();

        DNSSocket socket;
        bool connected = socket.connect(ip, port, test_timeout_);

        if (connected) {
            auto end_time = std::chrono::steady_clock::now();
            result.rtt = std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time
            ).count();
            result.success = true;
            Logger::log(LogLevel::DEBUG, "DualStack",
                        "连接测试成功, IP:= " + ip + " RTT=" + std::to_string(result.rtt) + "ms");
        } else {
            Logger::log(LogLevel::DEBUG, "DualStack", "连接测试失败, ip:= " + ip);
        }

        socket.close();
        return result;
    }

    bool DNSDualStackOptimizer::is_better_result(const dns::DNSDualStackOptimizer::TestResult &a,
                                                 const dns::DNSDualStackOptimizer::TestResult &b) const {
        if (!a.success && b.success) {
            return false;
        }

        if (a.success && !b.success) {
            return true;
        }

        if (!a.success && !b.success) {
            return false;
        }

        return a.rtt < b.rtt;
    }

    std::string DNSDualStackOptimizer::select_best_ip(
            const std::vector<std::shared_ptr<dns::DNSIPModel>> &ipv4_list,
            const std::vector<std::shared_ptr<dns::DNSIPModel>> &ipv6_list, int port) {
        if (ipv4_list.empty() && !ipv6_list.empty()) {
            return ipv6_list[0]->get_ip();
        }

        if (ipv6_list.empty() && !ipv4_list.empty()) {
            return ipv4_list[0]->get_ip();
        }

        if (ipv4_list.empty() && ipv6_list.empty()) {
            return "";
        }

        Logger::log(LogLevel::INFO, "DualStack",
                    "连接数量: IPv4=" + std::to_string(ipv4_list.size()) + " IPv6=" +
                    std::to_string(ipv6_list.size()));

        // Happy Eyeballs算法实现
        // 1. 首先尝试首选协议
        // 2. 延迟一段时间后尝试另一个协议
        // 3. 选择最快响应的

        std::vector<std::future<TestResult>> futures;
        std::vector<TestResult> results;

        const auto &first_list = prefer_ipv6_ ? ipv6_list : ipv4_list;
        const auto &second_list = prefer_ipv6_ ? ipv4_list : ipv6_list;

        if (!first_list.empty()) {
            futures.push_back(std::async(std::launch::async,
                                         &DNSDualStackOptimizer::test_single_ip, this,
                                         first_list[0]->get_ip(), port));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(connection_attempt_delay_));

        if (!second_list.empty()) {
            futures.push_back(std::async(std::launch::async,
                                         &DNSDualStackOptimizer::test_single_ip, this,
                                         second_list[0]->get_ip(), port));
        }


        for (auto &future: futures) {
            try {
                results.push_back(future.get());
            } catch (const std::exception &e) {
                Logger::log(LogLevel::ERROR, "DualStack",
                            std::string("Test exception: ") + e.what());
            }
        }

        if (results.empty()) {
            return !first_list.empty() ? first_list[0]->get_ip() : (!second_list.empty()
                                                                    ? second_list[0]->get_ip()
                                                                    : "");
        }

        auto best = results[0];
        for (size_t i = 1; i < results.size(); ++i) {
            if (is_better_result(results[i], best)) {
                best = results[i];
            }
        }
        Logger::log(LogLevel::INFO, "DualStack", "最佳IP: " + best.ip +
                                                 (best.success ? " RTT=" +
                                                                 std::to_string(best.rtt) + "ms"
                                                               : " (fallback)"));

        return best.ip;
    }

    void DNSDualStackOptimizer::test_connections(
            const std::vector<std::shared_ptr<dns::DNSIPModel>> &ips, int port,
            std::function<void(const dns::DNSDualStackOptimizer::TestResult &)> callback) {

        if (!callback) {
            Logger::log(LogLevel::WARN, "DualStack", "没有callback");
            return;
        }

        std::vector<std::thread> threads;

        for (const auto &ip_model: ips) {
            threads.emplace_back([this, ip_model, port, callback]() {
                auto result = test_single_ip(ip_model->get_ip(), port);
                callback(result);
            });
        }

        for (auto &thread: threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void DNSDualStackOptimizer::set_prefer_ipv6(bool prefer) {
        prefer_ipv6_ = prefer;
    }

    void DNSDualStackOptimizer::set_connection_attempt_delay(int delay) {
        connection_attempt_delay_ = delay;
    }

    void DNSDualStackOptimizer::set_test_timeout(int timeout) {
        test_timeout_ = timeout;
    }
}
