//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSOCKET_H
#define MEDIAPROXY_DNSSOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "DNSCommon.h"

namespace dns{
    class DNSSocket {

    public:
        DNSSocket();
        ~DNSSocket();

        bool connect(const std::string& ip, int port, int timeout_ms = Constants::DEFAULT_TIMEOUT_MS);
        void close();

        ssize_t send(const void* data, size_t len);
        ssize_t recv(void* buffer, size_t len);

        bool is_connected() const;
        int get_socket_fd() const;
        std::string get_remote_ip() const;
        int get_remote_port() const;

        bool set_non_blocking(bool non_blocking);
        bool set_reuse_address(bool reuse);
        bool set_send_timeout(int timeout_ms);
        bool set_recv_timeout(int timeput_ms);

    private:
        bool create_socket();
        bool set_socket_timeout(int optname, int timeout_ms);

    private:
        int socket_fd_;
        bool connected_;
        std::string remote_ip_;
        int remote_port_;
        struct sockaddr_in server_addr_;

    };
}


#endif //MEDIAPROXY_DNSSOCKET_H
