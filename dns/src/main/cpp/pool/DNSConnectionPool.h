//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSCONNECTIONPOOL_H
#define MEDIAPROXY_DNSCONNECTIONPOOL_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <curl/curl.h>

#include "DNSCommon.h"

namespace dns {

    class DNSConnectionPool {
    public:
        struct Connection {
            CURL* handle;
            bool in_use;
            std::chrono::steady_clock::time_point last_used;
            std::string host;

            Connection() : handle(nullptr), in_use(false) {}
        };

        explicit DNSConnectionPool(
                int max_connections_per_host = 6,
                int connection_timeout = 30000,
                int idle_timeout = 60000
                );

        ~DNSConnectionPool();

        CURL* acquire(const std::string& host);

        void release(CURL* handle);

        void cleanup();

        void clear();

        struct Stats{
            size_t total_connections;
            size_t active_connections;
            size_t idle_connections;
            size_t hosts_count;
        };

        Stats get_state() const;


    private:
        CURL* create_connection();

        std::string extract_host(const std::string& url);

        bool is_connection_expired(const Connection& conn) const;


    private:
        std::map<std::string, std::vector<Connection>> pools_;
        mutable std::mutex mutex_;
        int max_connections_per_host_;
        int connection_timeout_;
        int idle_timeout_;

    };
}




#endif //MEDIAPROXY_DNSCONNECTIONPOOL_H