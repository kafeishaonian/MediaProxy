//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSDUALSTACKOPTIMIZER_H
#define MEDIAPROXY_DNSDUALSTACKOPTIMIZER_H

#include <vector>
#include <memory>
#include <string>
#include <functional>

#include "DNSCommon.h"
#include "DNSIPModel.h"

namespace dns {

    class DNSDualStackOptimizer {

    public:
        struct TestResult {
            std::string ip;
            int rtt;
            bool success;
        };

        DNSDualStackOptimizer();

        ~DNSDualStackOptimizer() = default;

        std::string select_best_ip(
                const std::vector<std::shared_ptr<DNSIPModel>> &ipv4_list,
                const std::vector<std::shared_ptr<DNSIPModel>> &ipv6_list,
                int port = 80
        );

        void test_connections(
                const std::vector<std::shared_ptr<DNSIPModel>> &ips,
                int port,
                std::function<void(const TestResult &)> callback
        );

        void set_prefer_ipv6(bool prefer);

        void set_connection_attempt_delay(int delay);

        void set_test_timeout(int timeout);

    private:
        TestResult test_single_ip(const std::string &ip, int port);

        bool is_better_result(const TestResult &a, const TestResult &b) const;

    private:
        bool prefer_ipv6_;
        int connection_attempt_delay_;
        int test_timeout_;
    };
}


#endif //MEDIAPROXY_DNSDUALSTACKOPTIMIZER_H