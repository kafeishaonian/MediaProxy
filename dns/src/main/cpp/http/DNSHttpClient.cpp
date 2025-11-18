//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSHttpClient.h"

#include <sstream>
#include <algorithm>

namespace dns {

    DNSHttpClient::DNSHttpClient()
            : timeout_(5),
              connect_timeout_(3),
              verify_ssl_(true),
              user_agent_("DNS/1.0") {

        curl_global_init(CURL_GLOBAL_DEFAULT);
    }


    DNSHttpClient::~DNSHttpClient() {
        curl_global_cleanup();
    }

    size_t DNSHttpClient::write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
        size_t total_size = size * nmemb;
        std::string *response = static_cast<std::string *>(userp);
        response->append(static_cast<char *>(contents), total_size);
        return total_size;
    }

    CURL *DNSHttpClient::create_curl_handle() {
        CURL *curl = curl_easy_init();
        if (!curl) {
            return nullptr;
        }

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout_);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);

        curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent_.c_str());

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

        return curl;
    }

    std::string DNSHttpClient::send_doh_request(const std::string &url, const std::string &hostname,
                                                const std::string &record_type) {

        CURL *curl = create_curl_handle();
        if (!curl) {
            Logger::log(LogLevel::ERROR, "HttpClient", "CURL创建失败");
            return "";
        }

        std::string full_url = url;
        if (full_url.find('?') == std::string::npos) {
            full_url += "?";
        } else {
            full_url += "&";
        }

        full_url += "name=" + hostname + "&type=" + record_type;

        Logger::log(LogLevel::DEBUG, "HttpClient", "DoH请求:= " + full_url);

        curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/dns-json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            Logger::log(LogLevel::ERROR, "HttpClient",
                        std::string("CURL请求失败: ") + curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return "";
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);

        if (http_code != 200) {
            Logger::log(LogLevel::ERROR, "HttpClient",
                        "HTTP请求错误code:= " + std::to_string(http_code));

            return "";
        }

        Logger::log(LogLevel::DEBUG, "HttpClient",
                    "DoH获取的内容: " + std::to_string(response.length()));

        return response;
    }

    std::vector<std::string> DNSHttpClient::extract_ips_from_json(const std::string &json) {
        std::vector<std::string> ips;

        size_t pos = 0;
        const std::string data_key = "\"data\"";

        while ((pos = json.find(data_key, pos)) != std::string::npos) {
            pos += data_key.length();

            while (pos < json.length() && (json[pos] == ' ' || json[pos] == ':')) {
                pos++;
            }

            if (pos < json.length() && json[pos] == '"') {
                pos++;
                size_t end_pos = json.find('"', pos);
                if (end_pos != std::string::npos) {
                    std::string value = json.substr(pos, end_pos - pos);

                    if (value.find('.') != std::string::npos ||
                        value.find(':' != std::string::npos)) {
                        ips.push_back(value);
                        Logger::log(LogLevel::DEBUG, "HttpClient", "提取IP: " + value);
                    }
                }
            }
            pos++;
        }
        return ips;
    }

    std::vector<std::string> DNSHttpClient::parse_doh_response(const std::string &json_response) {
        if (json_response.empty()) {
            return {};
        }

        size_t status_pos = json_response.find("\"Status\"");
        if (status_pos != std::string::npos) {
            size_t colon_pos = json_response.find(':', status_pos);
            if (colon_pos != std::string::npos) {
                size_t num_start = colon_pos + 1;
                while (num_start < json_response.length() &&
                       (json_response[num_start] == ' ' || json_response[num_start] == '\t')) {
                    num_start++;
                }

                if (num_start < json_response.length() && json_response[num_start] != '0') {
                    Logger::log(LogLevel::WARN, "HttpClient", "DoH响应状态不等于0");
                }
            }
        }
        auto ips = extract_ips_from_json(json_response);

        Logger::log(LogLevel::INFO, "HttpClient",
                    "解析DOH的IP地址： " + std::to_string(ips.size()));

        return ips;
    }

    void DNSHttpClient::set_timeout(long seconds) {
        timeout_ = seconds;
    }

    void DNSHttpClient::set_connect_timeout(long seconds) {
        connect_timeout_ = seconds;
    }

    void DNSHttpClient::set_verify_ssl(bool verify) {
        verify_ssl_ = verify;
    }

    void DNSHttpClient::set_user_agent(const std::string &user_agent) {
        user_agent_ = user_agent;
    }
}