//
// Created by Hongmingwei on 2025/11/12.
//

#include "Ping.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <cstring>
#include <chrono>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sstream>
#include <regex>

namespace dns {
    struct IcmpEchoHeader {
        uint8_t type;
        uint8_t code;
        uint16_t checksum;
        uint16_t id;
        uint16_t sequence;
    };

    Ping::Ping()
            : icmp_socket_(-1), ping_method_(PingMethod::ICMP_DGRAM) {
        identifier_ = getpid() & 0xFFFF;
    }

    Ping::~Ping() {
        close_socket();
    }

    PingMethod Ping::detect_best_method() {
        int test_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
        if (test_socket >= 0) {
            close(test_socket);
            return PingMethod::ICMP_DGRAM;
        }

        return PingMethod::SYSTEM_PING;
    }


    bool Ping::create_socket() {
        if (ping_method_ == PingMethod::ICMP_DGRAM) {
            ping_method_ = detect_best_method();
        }

        switch (ping_method_) {
            case PingMethod::ICMP_DGRAM:
                return create_dgram_socket();
            default:
                return false;
        }
    }


    bool Ping::create_dgram_socket() {
        icmp_socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
        if (icmp_socket_ < 0) {
            Logger::log(LogLevel::WARN, "Ping", "create ICMP DGRAM socket failed");
            return false;
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(icmp_socket_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        return true;
    }

    void Ping::close_socket() {
        if (icmp_socket_ >= 0) {
            close(icmp_socket_);
            icmp_socket_ = -1;
        }
    }


    PingResult Ping::ping(const std::string &ip, int count, int timeout_ms) {
        PingResult result;
        result.ip = ip;
        result.sent = count;

        if (create_socket()) {
            int total_rtt = 0;
            int min_rtt = INT_MAX;
            int max_rtt = 0;
            for (int i = 0; i < count; ++i) {
                IcmpEchoReply reply;

                if (ping_method_ == PingMethod::ICMP_DGRAM) {
                    reply = ping_once_dgram(ip, i, timeout_ms);
                }
                result.replies.push_back(reply);

                if (reply.success) {
                    result.received++;
                    total_rtt += reply.rtt;
                    if (reply.rtt < min_rtt) min_rtt = reply.rtt;
                    if (reply.rtt > max_rtt) max_rtt = reply.rtt;
                }

                if (i < count - 1) {
                    usleep(100000); //100ms
                }
            }

            close_socket();

            if (result.received > 0) {
                result.avg_rtt = total_rtt / result.received;
                result.min_rtt = min_rtt;
                result.max_rtt = max_rtt;
                result.loss_rate = ((count - result.received) * 100) / count;
            }

            return result;
        }

        Logger::log(LogLevel::INFO, "ping", "使用系统ping命令");
        return ping_using_system_command(ip, count, timeout_ms);
    }

    IcmpEchoReply Ping::ping_once_dgram(const std::string &ip, int sequence, int timeou_ms) {
        IcmpEchoReply reply;
        reply.ip = ip;
        reply.sequence = sequence;

        char send_buffer[sizeof(IcmpEchoHeader) + 56];
        memset(send_buffer, 0, sizeof(send_buffer));

        IcmpEchoHeader *icmp_header = (IcmpEchoHeader *) send_buffer;
        icmp_header->type = 8;
        icmp_header->code = 0;
        icmp_header->id = htons(identifier_);
        icmp_header->sequence = htons(sequence);
        icmp_header->checksum = 0;

        for (size_t i = sizeof(IcmpEchoHeader); i < sizeof(send_buffer); ++i) {
            send_buffer[i] = i & 0xFF;
        }

        icmp_header->checksum = calculate_checksum(send_buffer, sizeof(send_buffer));

        //发送ICMP请求
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        auto send_time = std::chrono::steady_clock::now();

        ssize_t sent_bytes = sendto(icmp_socket_, send_buffer, sizeof(send_buffer), 0,
                                    (struct sockaddr *) &addr, sizeof(addr));

        if (sent_bytes < 0) {
            Logger::log(LogLevel::ERROR, "Ping", "发送ICMP报文失败");
            return reply;
        }

        char recv_buffer[1024];
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        struct timeval tv;
        tv.tv_sec = timeou_ms / 1000;
        tv.tv_usec = (timeou_ms % 1000) * 1000;
        setsockopt(icmp_socket_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ssize_t recv_len = recvfrom(icmp_socket_, recv_buffer, sizeof(recv_buffer), 0,
                                    (struct sockaddr *) &from_addr, &from_len);

        if (recv_len >= (ssize_t) sizeof(IcmpEchoHeader)) {
            auto recv_time = std::chrono::steady_clock::now();
            auto rtt = std::chrono::duration_cast<std::chrono::milliseconds>(
                    recv_time - send_time).count();

            IcmpEchoHeader *recv_icmp_header = (IcmpEchoHeader *) recv_buffer;
            if (recv_icmp_header->type == 0 &&
                ntohs(recv_icmp_header->id) == identifier_ &&
                ntohs(recv_icmp_header->sequence) == sequence) {
                reply.rtt = rtt;
                reply.success = true;
                reply.ttl = 64;
            }
        }
        return reply;
    }


    PingResult Ping::ping_using_system_command(const std::string &ip, int count, int timeou_ms) {
        PingResult result;
        result.ip = ip;
        result.sent = count;

        std::stringstream cmd;
        cmd << "ping -c " << count << " -W " << (timeou_ms / 1000) << " " << ip << " 2>&1";

        FILE *pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) {
            Logger::log(LogLevel::ERROR, "Ping", "执行ping命令失败");
            return result;
        }

        char buffer[256];
        std::string output;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }

        pclose(pipe);

        std::regex rtt_regex(R"(time=(\d+\.?\d*)\s*ms)");
        std::regex stats_regex(R"((\d+) packets transmitted, (\d+) received)");
        std::regex rtt_summary_regex(R"(min/avg/max/mdev = ([\d.]+)/([\d.]+)/([\d.]+))");

        std::smatch match;

        std::string::const_iterator search_start(output.cbegin());
        int sequence = 0;
        while (std::regex_search(search_start, output.cend(), match, rtt_regex)) {
            IcmpEchoReply reply;
            reply.ip = ip;
            reply.sequence = sequence++;
            reply.rtt = std::stof(match[1]);
            reply.success = true;
            result.replies.push_back(reply);
            search_start = match.suffix().first;
        }

        if (std::regex_search(output, match, stats_regex)) {
            result.sent = std::stoi(match[1]);
            result.received = std::stoi(match[2]);
            result.loss_rate = ((result.sent - result.received) * 100) / result.sent;
        }

        if (std::regex_search(output, match, rtt_summary_regex)) {
            result.min_rtt = std::stof(match[1]);
            result.avg_rtt = std::stof(match[2]);
            result.max_rtt = std::stof(match[3]);
        }

        return result;
    }

    uint16_t Ping::calculate_checksum(const char *buffer, int len) {
        uint32_t sum = 0;
        const auto *ptr = (const uint16_t *) buffer;
        while (len > 1) {
            sum += *ptr++;
            len -= 2;
        }

        if (len == 1) {
            sum += *(const uint8_t *) ptr;
        }

        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);

        return ~sum;
    }
}


