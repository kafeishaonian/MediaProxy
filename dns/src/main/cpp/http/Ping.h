//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_PING_H
#define MEDIAPROXY_PING_H

#include <netinet/ip_icmp.h>

#include "DNSCommon.h"

namespace dns {

    //ICMP回复结构
    struct IcmpEchoReply {
        std::string ip;
        int sequence;
        int ttl;
        int rtt;
        bool success;

        IcmpEchoReply() : sequence(0), ttl(0), rtt(-1), success(false) {}
    };

    //Ping结果
    struct PingResult {
        std::string ip;
        int sent;
        int received;
        int loss_rate;
        int min_rtt;
        int max_rtt;
        int avg_rtt;
        std::vector<IcmpEchoReply> replies;

        PingResult() : sent(0), received(0), loss_rate(100),
                       min_rtt(-1), max_rtt(-1), avg_rtt(-1) {}
    };

    enum class PingMethod {
        ICMP_DGRAM,
        SYSTEM_PING,
        TCP_CONNECT
    };

    class Ping {
    public:
        Ping();

        ~Ping();

        PingResult ping(const std::string &ip, int count = 4, int timeout_ms = 1000);

        static int get_average_rtt(const std::vector<IcmpEchoReply> &replies);

        void set_packet_size(int size);

        void set_ttl(int ttl);

        void set_ping_method(PingMethod method);

    private:
        bool create_socket();
        bool create_dgram_socket();

        void close_socket();

        IcmpEchoReply ping_once_dgram(const std::string& ip, int sequence, int timeou_ms);

        PingResult ping_using_system_command(const std::string& ip, int count, int timeou_ms);

        uint16_t calculate_checksum(const char *buffer, int len);

        PingMethod detect_best_method();

    private:
        int icmp_socket_;
        uint16_t identifier_;
        PingMethod ping_method_;
    };

}

#endif //MEDIAPROXY_PING_H