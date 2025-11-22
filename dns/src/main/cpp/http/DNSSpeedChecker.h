//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSPEEDCHECKER_H
#define MEDIAPROXY_DNSSPEEDCHECKER_H

#include "DNSCommon.h"
#include "DNSIPModel.h"
#include "DNSSocket.h"
#include "Ping.h"

namespace dns {
    class DNSSpeedChecker{
    public:
        DNSSpeedChecker();
        ~DNSSpeedChecker() = default;

        int check_speed(const std::string& ip, int port = Constants::HTTPS_PORT);

        void set_timeout(int timeout_ms);
        void set_retry_count(int count);

        int get_timeout() const;
        int get_retry_count() const;

        void check_multiple(std::vector<std::shared_ptr<DNSIPModel>>& ip_list, int port = Constants::HTTPS_PORT);

        int check_speed_with_ping(const std::string& ip);

    private:
        int check_speed_with_socket(const std::string& ip, int port);

    private:
        int timeout_;
        int retry_count_;
        std::shared_ptr<Ping> pinger_;

    };
}


#endif //MEDIAPROXY_DNSSPEEDCHECKER_H
