//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSIPModel.h"

#include <sstream>
#include <chrono>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

namespace dns {

    DNSIPModel::DNSIPModel()
            : port_(0), speed_(-1), timestamp_(0), is_valid_(true),
              ip_version_(IPVersion::UNKNOWN) {
        timestamp_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    DNSIPModel::DNSIPModel(const std::string &ip, int port)
            : ip_(ip), port_(port), speed_(-1), is_valid_(true),
              ip_version_(IPVersion::UNKNOWN) {
        timestamp_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
        detect_ip_version();
    }

    std::string DNSIPModel::get_ip() const {
        return ip_;
    }

    int DNSIPModel::get_port() const {
        return port_;
    }

    int DNSIPModel::get_speed() const {
        return speed_;
    }

    long DNSIPModel::get_timestamp() const {
        return timestamp_;
    }

    bool DNSIPModel::is_valid() const {
        return is_valid_;
    }

    DNSIPModel::IPVersion DNSIPModel::get_ip_version() const {
        return ip_version_;
    }

    bool DNSIPModel::is_ipv4() const {
        return ip_version_ == IPVersion::IPV4;
    }

    bool DNSIPModel::is_ipv6() const {
        return ip_version_ == IPVersion::IPV6;
    }

    void DNSIPModel::detect_ip_version() {
        if (ip_.empty()) {
            ip_version_ = IPVersion::UNKNOWN;
            return;
        }

        bool has_dot = ip_.find('.') != std::string::npos;
        bool has_colon = ip_.find(':') != std::string::npos;

        if (has_dot && !has_colon) {
            ip_version_ = IPVersion::IPV4;
        } else if (has_colon) {
            ip_version_ = IPVersion::IPV6;
        } else {
            ip_version_ = IPVersion::UNKNOWN;
        }
    }

    std::string DNSIPModel::get_ipv6_compressed() const {
        if (!is_ipv6() || ip_.find("::") != std::string::npos) {
            return ip_;
        }

        std::vector<std::string> segments;
        std::stringstream ss(ip_);
        std::string segment;

        while (std::getline(ss, segment, ':')) {
            segments.push_back(segment);
        }

        if (segments.size() != 8) {
            return ip_;
        }

        for (auto &seg: segments) {
            while (seg.length() > 1 && seg[0] == '0') {
                seg = seg.substr(1);
            }
        }

        int max_zero_start = -1, max_zero_len = 0;
        int curr_zero_start = -1, curr_zero_len = 0;

        for (int i = 0; i < 8; i++) {
            if (segments[i] == "0") {
                if (curr_zero_start == -1) {
                    curr_zero_start = i;
                    curr_zero_len = 1;
                } else {
                    curr_zero_len++;
                }
            } else {
                if (curr_zero_len > max_zero_len) {
                    max_zero_start = curr_zero_start;
                    max_zero_len = curr_zero_len;
                }
                curr_zero_start = -1;
                curr_zero_len = 0;
            }
        }

        if (curr_zero_len > max_zero_len) {
            max_zero_start = curr_zero_start;
            max_zero_len = curr_zero_len;
        }

        std::ostringstream result;
        if (max_zero_len > 1) {
            for (int i = 0; i < 8; i++) {
                if (i == max_zero_start) {
                    result << "::";
                    i += max_zero_len - 1;
                } else {
                    if (i > 0 && i > max_zero_start + max_zero_len) {
                        result << ":";
                    } else if (i > 0 && i < max_zero_start) {
                        result << ":";
                    }
                    result << segments[i];
                }
            }
        } else {
            for (int i = 0; i < 8; i++) {
                if (i > 0) result << ":";
                result << segments[i];
            }
        }

        return result.str();
    }

    bool DNSIPModel::is_ipv6_link_local() const {
        if (!is_ipv6()) {
            return false;
        }

        return ip_.find("fe80:") == 0 || ip_.find("FE80:") == 0;
    }

    bool DNSIPModel::is_ipv6_site_local() const {
        if (!is_ipv6()) {
            return false;
        }
        //site local 基本不再使用
        return ip_.find("fec0:") == 0 || ip_.find("FEC0:") == 0;
    }

    bool DNSIPModel::is_ipv6_unique_local() const {
        if (!is_ipv6()) {
            return false;
        }
        return ip_.find("fc") == 0 || ip_.find("FC") == 0 ||
               ip_.find("fd") == 0 || ip_.find("FD") == 0;
    }

    bool DNSIPModel::is_ipv6_global() const {
        if (!is_ipv6()) {
            return false;
        }

        return !is_ipv6_link_local() && !is_ipv6_site_local() &&
               !is_ipv6_unique_local() && ip_ != "::1";
    }

    void DNSIPModel::set_ip(const std::string &ip) {
        ip_ = ip;
    }

    void DNSIPModel::set_port(int port) {
        port_ = port;
    }

    void DNSIPModel::set_speed(int speed) {
        speed_ = speed;
    }

    void DNSIPModel::set_timestamp(long timestamp) {
        timestamp_ = timestamp;
    }

    void DNSIPModel::set_valid(bool valid) {
        is_valid_ = valid;
    }

    std::string DNSIPModel::to_string() const {
        std::ostringstream oss;
        oss << "IP: " << ip_;
        if (port_ > 0) {
            oss << ":" <<port_;
        }
        oss << ", Speed: " << speed_ << "ms";
        oss << ", Timestamp: " << timestamp_;
        oss << ", Valid: " << (is_valid_ ? "true" : "false");
        return oss.str();
    }

    std::string DNSIPModel::to_json() const {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        doc.AddMember("ip", rapidjson::Value(ip_.c_str(), allocator), allocator);
        doc.AddMember("port", port_, allocator);
        doc.AddMember("speed", speed_, allocator);
        doc.AddMember("timestamp", timestamp_, allocator);
        doc.AddMember("valid", is_valid_, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }

    std::shared_ptr<dns::DNSIPModel> DNSIPModel::from_json(const std::string &json) {
        rapidjson::Document doc;
        if (doc.Parse(json.c_str()).HasParseError()) {
            return nullptr;
        }

        auto model = std::make_shared<DNSIPModel>();

        if (doc.HasMember("ip") && doc["ip"].IsString()) {
            model->ip_ = doc["ip"].GetString();
            model->detect_ip_version();
        }

        if (doc.HasMember("port") && doc["port"].IsInt()) {
            model->port_ = doc["port"].GetInt();
        }

        if (doc.HasMember("speed") && doc["speed"].IsInt()) {
            model->speed_ = doc["speed"].GetInt();
        }

        if (doc.HasMember("timestamp") && doc["timestamp"].IsInt64()) {
            model->timestamp_ = doc["timestamp"].GetInt64();
        }

        if (doc.HasMember("valid") && doc["valid"].IsBool()) {
            model->is_valid_ = doc["valid"].GetBool();
        }

        return model;
    }

    bool DNSIPModel::operator<(const dns::DNSIPModel &other) const {
        return speed_ < other.speed_;
    }
}