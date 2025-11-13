//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSIPMODEL_H
#define MEDIAPROXY_DNSIPMODEL_H

#include "DNSCommon.h"

namespace dns {
    class DNSIPModel {
    public:
        enum class IPVersion {
            IPV4,
            IPV6,
            UNKNOWN
        };

        DNSIPModel();
        explicit DNSIPModel(const std::string& ip, int port = 0);

        std::string get_ip() const;

        int get_port() const;

        int get_speed() const;

        long get_timestamp() const;

        bool is_valid() const;

        IPVersion get_ip_version() const;

        bool is_ipv4() const;

        bool is_ipv6() const;

        std::string get_ipv6_compressed() const;

        bool is_ipv6_link_local() const;

        bool is_ipv6_site_local() const;

        bool is_ipv6_unique_local() const;

        bool is_ipv6_global() const;

        void set_ip(const std::string& ip);

        void set_port(int port);

        void set_speed(int speed);

        void set_timestamp(long timestamp);

        void set_valid(bool valid);

        std::string to_string() const;

        std::string to_json() const;

        static std::shared_ptr<DNSIPModel> from_json(const std::string& json);

        bool operator<(const DNSIPModel& other)> const;

    private:
        void detect_ip_version();

    private:
        std::string ip_;
        int port_;
        int speed_;
        long timestamp_;
        bool is_valid_;
        IPVersion ip_version_;
    };
}


#endif //MEDIAPROXY_DNSIPMODEL_H
