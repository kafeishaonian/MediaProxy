//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSIPModel.h"

#include <sstream>
#include <chrono>

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
        if (!is_ipv6()) {
            return ip_;
        }

        if (ip_.find("::") != std::string::npos) {
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

        //去掉每段的前导0，但至少保留一个字符
        for (auto &seg: segments) {
            while (seg.length() > 1 && seg[0] == '0') {
                seg = seg.substr(1);
            }
        }

        //找到最长的连续0段序列
        int max_zero_start = -1;
        int max_zero_len = 0;
        int curr_zero_start = -1;
        int curr_zero_len = 0;

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

        // 检查最后一个0序列
        if (curr_zero_len > max_zero_len) {
            max_zero_start = curr_zero_start;
            max_zero_len = curr_zero_len;
        }

        //构建压缩后的地址
        std::ostringstream result;

        // 只有当连续0的长度大于1时才进行压缩
        if (max_zero_len > 1) {
            for (int i = 0; i < 8; i++) {
                if (i == max_zero_start) {
                    // 压缩区域的起始位置
                    result << "::";
                    // 跳过整个压缩区域
                    i += max_zero_len - 1;
                } else {
                    // 非压缩区域
                    if (i > 0 && i > max_zero_start + max_zero_len) {
                        // 在压缩区域之后需要添加分隔符
                        result << ":";
                    } else if (i > 0 && i < max_zero_start) {
                        // 在压缩区域之前需要添加分隔符
                        result << ":";
                    }
                    result << segments[i];
                }
            }
        } else {
            // 不需要压缩，只是去掉了前导0
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
        std::ostringstream oss;
        oss << "{"
                << "\"ip\":\"" << ip_ << "\","
                << "\"port\":" << port_ << ","
                << "\"speed\":" << speed_ << ","
                << "\"timestamp\":" << timestamp_ << ","
                << "\"valid\":" << (is_valid_ ? "true" : "false")
                << "}";
        return oss.str();
    }

    std::shared_ptr<dns::DNSIPModel> DNSIPModel::from_json(const std::string &json) {
        auto model = std::make_shared<DNSIPModel>();

        // 解析 ip
        size_t ip_pos = json.find("\"ip\":\"");
        if (ip_pos != std::string::npos) {
            size_t start = ip_pos + 6;
            size_t end = json.find("\"", start);
            if (end != std::string::npos) {
                model->ip_ = json.substr(start, end - start);
                model->detect_ip_version();
            }
        }

        // 解析 port
        size_t port_pos = json.find("\"port\":");
        if (port_pos != std::string::npos) {
            size_t start = port_pos + 7;
            size_t end = json.find_first_of(",}", start);
            if (end != std::string::npos) {
                try {
                    model->port_ = std::stoi(json.substr(start, end - start));
                } catch (...) {
                }
            }
        }

        // 解析 speed
        size_t speed_pos = json.find("\"speed\":");
        if (speed_pos != std::string::npos) {
            size_t start = speed_pos + 8;
            size_t end = json.find_first_of(",}", start);
            if (end != std::string::npos) {
                try {
                    model->speed_ = std::stoi(json.substr(start, end - start));
                } catch (...) {
                }
            }
        }

        // 解析 timestamp
        size_t timestamp_pos = json.find("\"timestamp\":");
        if (timestamp_pos != std::string::npos) {
            size_t start = timestamp_pos + 12;
            size_t end = json.find_first_of(",}", start);
            if (end != std::string::npos) {
                try {
                    model->timestamp_ = std::stol(json.substr(start, end - start));
                } catch (...) {
                }
            }
        }

        // 解析 valid
        size_t valid_pos = json.find("\"valid\":");
        if (valid_pos != std::string::npos) {
            size_t start = valid_pos + 8;
            if (json.substr(start, 4) == "true") {
                model->is_valid_ = true;
            } else if (json.substr(start, 5) == "false") {
                model->is_valid_ = false;
            }
        }

        return model;
    }

    //重写比较运算符用于排序
    bool DNSIPModel::operator<(const dns::DNSIPModel &other) {
        return speed_ < other.speed_;
    }
}