//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSHTTPCLIENT_H
#define MEDIAPROXY_DNSHTTPCLIENT_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <curl/curl.h>

#include "DNSCommon.h"

namespace dns {
    /**
     * 使用libcurl请求Http,通过Doh获取DNS
     */
    class DNSHttpClient {
    public:
        DNSHttpClient();
        ~DNSHttpClient();

        std::string send_doh_request(
                const std::string &url,
                const std::string &hostname,
                const std::string &record_type = "A"
        );

        std::vector<std::string> parse_doh_response(const std::string& json_response);

        void set_timeout(long seconds);

        void set_connect_timeout(long seconds);

        void set_verify_ssl(bool verify);

        void set_user_agent(const std::string& user_agent);

    private:
        static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

        CURL* create_curl_handle();

        std::vector<std::string> extract_ips_from_json(const std::string& json);

    private:
        long timeout_;
        long connect_timeout_;
        bool verify_ssl_;
        std::string user_agent_;

    };
}

#endif //MEDIAPROXY_DNSHTTPCLIENT_H