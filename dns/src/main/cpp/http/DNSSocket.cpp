//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSSocket.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstring>
#include <chrono>


namespace dns {
    DNSSocket::DNSSocket()
            : socket_fd_(-1), connected_(false), remote_port_(0) {
        memset(&server_addr_, 0, sizeof(server_addr_));
    }

    DNSSocket::~DNSSocket() {
        close();
    }

    bool DNSSocket::create_socket() {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            Logger::log(LogLevel::ERROR, "DNSSocket", "socket 创建失败");
            return false;
        }
        return true;
    }

    bool DNSSocket::connect(const std::string &ip, int port, int timeout_ms) {
        if (connected_) {
            close();
        }

        if (!create_socket()) {
            return false;
        }

        set_non_blocking(true);

        server_addr_.sin_family = AF_INET;
        server_addr_.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &server_addr_.sin_addr) <= 0) {
            Logger::log(LogLevel::ERROR, "DNSSocket", "IP 地址无效: " + ip);
            close();
            return false;
        }

        auto start_time = std::chrono::steady_clock::now();
        int result = ::connect(socket_fd_, (struct sockaddr*)&server_addr_, sizeof(server_addr_));

        if (result < 0) {
            if (errno != EINPROGRESS) {
                Logger::log(LogLevel::ERROR, "DNSSocket", "连接失败");
                close();
                return false;
            }

            fd_set write_set;
            FD_ZERO(&write_set);
            FD_SET(socket_fd_, &write_set);

            struct timeval timeout;
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;

            result = select(socket_fd_ + 1, nullptr, &write_set, nullptr, &timeout);
            if (result <= 0) {
                Logger::log(LogLevel::ERROR, "DNSSocket", "连接超时");
                close();
                return false;
            }

            int error = 0;
            socklen_t len = sizeof(error);
            if (getsockopt(socket_fd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                Logger::log(LogLevel::ERROR, "DNSSocket", "连接失败: " + std::string(strerror(error)));
                close();
                return false;
            }
        }
        set_non_blocking(false);

        connected_ = true;
        remote_ip_ = ip;
        remote_port_ = port;

        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        Logger::log(LogLevel::DEBUG, "DNSSocket",
                    "连接成功，ip:=  " + ip + ":" + std::to_string(port) + " 耗时:= " + std::to_string(duration) + "ms");
        return true;
    }

    void DNSSocket::close() {
        if (socket_fd_ >= 0) {
            ::close(socket_fd_);
            socket_fd_ = -1;
        }
        connected_ = false;
        remote_ip_.clear();
        remote_port_ = 0;
    }

    ssize_t DNSSocket::send(const void *data, size_t len) {
        if (!connected_ || socket_fd_ < 0) {
            return -1;
        }
        return ::send(socket_fd_, data, len, 0);
    }

    ssize_t DNSSocket::recv(void *buffer, size_t len) {
        if (!connected_ || socket_fd_ < 0) {
            return -1;
        }
        return ::recv(socket_fd_, buffer, len, 0);
    }

    bool DNSSocket::is_connected() const {
        return connected_;
    }

    int DNSSocket::get_socket_fd() const {
        return socket_fd_;
    }

    std::string DNSSocket::get_remote_ip() const {
        return remote_ip_;
    }

    int DNSSocket::get_remote_port() const {
        return remote_port_;
    }

    bool DNSSocket::set_non_blocking(bool non_blocking) {
        if (socket_fd_ < 0) {
            return false;
        }

        int flags = fcntl(socket_fd_, F_GETFL, 0);
        if (flags < 0) {
            return false;
        }

        if (non_blocking) {
            flags |= O_NONBLOCK;
        } else {
            flags &= ~O_NONBLOCK;
        }
        return fcntl(socket_fd_, F_SETFL, flags) >= 0;
    }

    bool DNSSocket::set_reuse_address(bool reuse) {
        if (socket_fd_ < 0) {
            return false;
        }

        int optval = reuse ? 1 : 0;
        return setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) >= 0;
    }

    bool DNSSocket::set_send_timeout(int timeout_ms) {
        return set_socket_timeout(SO_SNDTIMEO, timeout_ms);
    }

    bool DNSSocket::set_recv_timeout(int timeput_ms) {
        return set_socket_timeout(SO_RCVTIMEO, timeput_ms);
    }

    bool DNSSocket::set_socket_timeout(int optname, int timeout_ms) {
        if (socket_fd_ < 0) {
            return false;
        }

        struct timeval timeout;
        timeout.tv_sec = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;

        return setsockopt(socket_fd_, SOL_SOCKET, optname, &timeout, sizeof(timeout)) >= 0;
    }
}