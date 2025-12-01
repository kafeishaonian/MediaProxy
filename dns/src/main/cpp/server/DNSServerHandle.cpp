//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSServerHandle.h"

#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <chrono>
#include <future>
#include <atomic>

#include "DNSHostManager.h"

namespace dns {

    DNSServerHandle::DNSServerHandle(dns::DNSServerType server_type) : server_type_(server_type) {

    }

    DNSServerType DNSServerHandle::get_server_type() const {
        return server_type_;
    }

    void DNSServerHandle::set_host_manager(std::shared_ptr<dns::DNSHostManager> manager) {
        host_manager_ = manager;
    }

    DNSSystemServerHandle::DNSSystemServerHandle() : DNSServerHandle(DNSServerType::SYSTEM) {

    }

    std::shared_ptr<dns::DNSHostModel> DNSSystemServerHandle::resolve(const std::string &hostname) {
        Logger::log(LogLevel::INFO, "DNSSystemServerHandle", "解析主机: " + hostname);

        std::vector<std::string> ip_list;
        if (!resolve_with_get_addr_info(hostname, ip_list)) {
            Logger::log(LogLevel::ERROR, "DNSSystemServerHandle", "解析失败: " + hostname);
            return nullptr;
        }

        auto host_model = std::make_shared<DNSHostModel>(hostname);
        host_model->set_server_type(DNSServerType::SYSTEM);

        for (const auto &ip: ip_list) {
            auto ip_model = std::make_shared<DNSIPModel>(ip);
            host_model->add_ip(ip_model);
        }

        Logger::log(LogLevel::INFO, "DNSSystemServerHandle",
                    "解析成功: " + hostname + " -> " + std::to_string(ip_list.size()) + " 个IP");

        return host_model;
    }

    bool DNSSystemServerHandle::resolve_with_get_addr_info(const std::string &hostname,
                                                           std::vector<std::string> &ip_list) {
        struct addrinfo hints;
        struct addrinfo *result = nullptr;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_ADDRCONFIG;

        int ret = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
        if (ret != 0) {
            Logger::log(LogLevel::ERROR, "DNSSystemServerHandle",
                        "getaddrinfo失败: " + std::string(gai_strerror(ret)));
            return false;
        }

        for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
            char ipstr[INET6_ADDRSTRLEN];
            void *addr;

            if (rp->ai_family == AF_INET) {
                struct sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(rp->ai_addr);
                addr = &(ipv4->sin_addr);
            } else if (rp->ai_family == AF_INET6) {
                struct sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(rp->ai_addr);
                addr = &(ipv6->sin6_addr);
            } else {
                continue;
            }

            inet_ntop(rp->ai_family, addr, ipstr, sizeof(ipstr));
            ip_list.push_back(std::string(ipstr));
        }

        freeaddrinfo(result);
        return !ip_list.empty();
    }


    DNSHttpServerHandle::DNSHttpServerHandle(const std::string &doh_server)
            : DNSServerHandle(DNSServerType::HTTP_DNS),
              doh_server_(doh_server),
              http_client_(std::make_shared<DNSHttpClient>()) {

        http_client_->set_timeout(5);
        http_client_->set_connect_timeout(3);
        http_client_->set_verify_ssl(true);
    }

    std::shared_ptr<dns::DNSHostModel> DNSHttpServerHandle::resolve(const std::string &hostname) {
        Logger::log(LogLevel::INFO, "DNSHttpServerHandle", "解析主机: " + hostname);

        std::string response = send_doh_request(hostname);
        if (response.empty()) {
            Logger::log(LogLevel::ERROR, "DNSHttpServerHandle", "DoH请求失败");
            return nullptr;
        }

        return parse_doh_response(response, hostname);
    }

    void DNSHttpServerHandle::set_doh_server(const std::string &server) {
        doh_server_ = server;
    }

    std::string DNSHttpServerHandle::get_doh_server() const {
        return doh_server_;
    }

    std::string DNSHttpServerHandle::send_doh_request(const std::string &hostname) {
        if (!http_client_) {
            Logger::log(LogLevel::ERROR, "DNSHttpServerHandle", "HTTP客户端未初始化");
            return "";
        }
        Logger::log(LogLevel::INFO, "DNSHttpServerHandle", "发送DoH请求: " + hostname);

        std::string response_v4, response_v6;
        std::atomic<bool> v4_done{false}, v6_done{false};

        auto future_v4 = std::async(std::launch::async, [&]() {
            response_v4 = http_client_->send_doh_request(doh_server_, hostname, "A");
            v4_done.store(true);
        });

        auto future_v6 = std::async(std::launch::async, [&]() {
            response_v6 = http_client_->send_doh_request(doh_server_, hostname, "AAAA");
            v6_done.store(true);
        });

        future_v4.wait_for(std::chrono::seconds(3));
        future_v6.wait_for(std::chrono::seconds(3));

        return !response_v4.empty() ? response_v4 : response_v6;
    }

    std::shared_ptr<dns::DNSHostModel>
    DNSHttpServerHandle::parse_doh_response(const std::string &response,
                                            const std::string &hostname) {
        if (response.empty()) {
            Logger::log(LogLevel::ERROR, "DNSHttpServerHandle", "DoH响应为空");
            return nullptr;
        }

        if (!http_client_) {
            Logger::log(LogLevel::ERROR, "DNSHttpServerHandle", "HTTP客户端未初始化");
            return nullptr;
        }

        auto ip_list = http_client_->parse_doh_response(response);

        if (ip_list.empty()) {
            Logger::log(LogLevel::WARN, "DNSHttpServerHandle", "未从DoH响应中提取到IP地址");
            return nullptr;
        }

        auto host_model = std::make_shared<DNSHostModel>(hostname);
        host_model->set_server_type(DNSServerType::HTTP_DNS);

        for (const auto &ip: ip_list) {
            auto ip_mode = std::make_shared<DNSIPModel>(ip);
            host_model->add_ip(ip_mode);
        }

        Logger::log(LogLevel::INFO, "DNSHttpServerHandle",
                    "DoH解析成功: " + hostname + " -> " + std::to_string(ip_list.size()) + " 个IP");

        return host_model;
    }


    DNSLocalServerHandle::DNSLocalServerHandle()
            : DNSServerHandle(DNSServerType::LOCAL),
              cache_expire_time_(3600) {

    }

    std::shared_ptr<dns::DNSHostModel> DNSLocalServerHandle::resolve(const std::string &hostname) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = cache_.find(hostname);

        if (it != cache_.end()) {
            auto host = it->second;
            if (!is_cache_expired(host)) {
                Logger::log(LogLevel::DEBUG, "DNSLocalServerHandle", "缓存命中: " + hostname);
                return host;
            } else {
                Logger::log(LogLevel::DEBUG, "DNSLocalServerHandle", "缓存已过期: " + hostname);
                cache_.erase(it);
            }
        }

        Logger::log(LogLevel::DEBUG, "DNSLocalServerHandle", "缓存未命中: " + hostname);
        return nullptr;
    }

    void DNSLocalServerHandle::add_to_cache(const std::string &hostname,
                                            std::shared_ptr<dns::DNSHostModel> host) {
        if (!host) {
            return;
        }
        std::lock_guard<std::mutex> lock(cache_mutex_);
        cache_[hostname] = host;
        Logger::log(LogLevel::DEBUG, "DNSLocalServerHandle", "添加到缓存: " + hostname);
    }

    void DNSLocalServerHandle::remove_from_cache(const std::string &hostname) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        cache_.erase(hostname);
        Logger::log(LogLevel::DEBUG, "DNSLocalServerHandle", "从缓存删除: " + hostname);
    }

    void DNSLocalServerHandle::clear_cache() {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        cache_.clear();
        Logger::log(LogLevel::INFO, "DNSLocalServerHandle", "清空缓存");
    }

    void DNSLocalServerHandle::set_cache_expire_time(int seconds) {
        cache_expire_time_ = seconds;
    }

    bool
    DNSLocalServerHandle::is_cache_expired(const std::shared_ptr<dns::DNSHostModel> &host) const {
        if (!host) {
            return true;
        }

        auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();

        long age = now - host->get_update_time();
        return age > cache_expire_time_;
    }
}