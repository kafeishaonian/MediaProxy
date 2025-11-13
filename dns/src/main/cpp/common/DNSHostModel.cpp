//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSHostModel.h"

#include <algorithm>
#include <sstream>
#include <chrono>

namespace dns {
    DNSHostModel::DNSHostModel()
            : update_time_(0), server_type_(DNSServerType::SYSTEM) {
        update_time_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    DNSHostModel::DNSHostModel(const std::string &hostname)
            : hostname_(hostname), server_type_(DNSServerType::SYSTEM) {
        update_time_ = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    std::string DNSHostModel::get_hostname() const {
        return hostname_;
    }

    std::vector<std::shared_ptr<dns::DNSIPModel>> DNSHostModel::get_ip_list() const {
        return ip_list_;
    }

    long DNSHostModel::get_update_time() const {
        return update_time_;
    }

    DNSServerType DNSHostModel::get_server_type() const {
        return server_type_;
    }

    void DNSHostModel::set_hostname(const std::string &hostname) {
        hostname_ = hostname;
    }

    void DNSHostModel::set_update_time(long time) {
        update_time_ = time;
    }

    void DNSHostModel::set_server_type(dns::DNSServerType type) {
        server_type_ = type;
    }


    void DNSHostModel::add_ip(std::shared_ptr<dns::DNSIPModel> ip) {
        if (ip && ip->is_valid()) {
            ip_list_.push_back(ip);
        }
    }

    void DNSHostModel::clear_ips() {
        ip_list_.clear();
    }

    bool DNSHostModel::has_valid_ip() const {
        return !ip_list_.empty() && ip_list_[0]->is_valid();
    }

    std::shared_ptr<dns::DNSIPModel> DNSHostModel::get_bast_ip() const {
        if (ip_list_.empty()) {
            return nullptr;
        }

        //返回最快的ip
        for (const auto &ip: ip_list_) {
            if (ip->is_valid() && ip->get_speed() >= 0) {
                return ip;
            }
        }

        return ip_list_[0];
    }


    std::string DNSHostModel::get_bast_ip_string() const {
        auto bast_ip = get_bast_ip();
        return bast_ip ? bast_ip->get_ip() : "";
    }

    void DNSHostModel::sort_by_speed() {
        std::sort(ip_list_.begin(), ip_list_.end(),
                  [](const std::shared_ptr<DNSIPModel> &a,
                     const std::shared_ptr<DNSIPModel> &b) {
                      if (!a->is_valid()) return false;
                      if (!b->is_valid()) return true;

                      if (a->get_speed() < 0) return false;
                      if (b->get_speed() < 0) return true;

                      return a->get_speed() < b->get_speed();
                  });
    }

    std::string DNSHostModel::to_string() const {
        std::ostringstream oss;
        oss << "Host:" << hostname_ << "\n";
        oss << "IPs (" << ip_list_.size() << "):\n";
        for (const auto &ip: ip_list_) {
            oss << "  - " << ip->to_string() << "\n";
        }
        return oss.str();
    }

    std::string DNSHostModel::to_json() const {
        std::ostringstream oss;
        oss << "{"
            << "\"hostname\":\"" << hostname_ << "\","
            << "\"update_time\":" << update_time_ << ","
            << "\"server_type\":" << static_cast<int>(server_type_) << ","
            << "\"ip_list\":[";

        for (size_t i = 0; i < ip_list_.size(); ++i) {
            if (i > 0) oss << ",";
            oss << ip_list_[i]->to_json();
        }

        oss << "]}";
        return oss.str();
    }

    std::shared_ptr<dns::DNSHostModel> DNSHostModel::from_json(const std::string &json) {
        auto model = std::make_shared<DNSHostModel>();
        
        // 解析 hostname
        size_t hostname_pos = json.find("\"hostname\":\"");
        if (hostname_pos != std::string::npos) {
            size_t start = hostname_pos + 12;
            size_t end = json.find("\"", start);
            if (end != std::string::npos) {
                model->hostname_ = json.substr(start, end - start);
            }
        }

        // 解析 update_time
        size_t update_time_pos = json.find("\"update_time\":");
        if (update_time_pos != std::string::npos) {
            size_t start = update_time_pos + 14;
            size_t end = json.find_first_of(",}", start);
            if (end != std::string::npos) {
                try {
                    model->update_time_ = std::stol(json.substr(start, end - start));
                } catch (...) {
                }
            }
        }

        // 解析 server_type
        size_t server_type_pos = json.find("\"server_type\":");
        if (server_type_pos != std::string::npos) {
            size_t start = server_type_pos + 14;
            size_t end = json.find_first_of(",}", start);
            if (end != std::string::npos) {
                try {
                    int type_value = std::stoi(json.substr(start, end - start));
                    model->server_type_ = static_cast<DNSServerType>(type_value);
                } catch (...) {
                }
            }
        }

        // 解析 ip_list 数组
        size_t ip_list_pos = json.find("\"ip_list\":[");
        if (ip_list_pos != std::string::npos) {
            size_t array_start = ip_list_pos + 11;
            size_t array_end = json.find("]", array_start);
            
            if (array_end != std::string::npos) {
                std::string ip_list_str = json.substr(array_start, array_end - array_start);
                
                // 解析数组中的每个 JSON 对象
                size_t pos = 0;
                int brace_count = 0;
                size_t obj_start = std::string::npos;
                
                while (pos < ip_list_str.length()) {
                    char ch = ip_list_str[pos];
                    
                    if (ch == '{') {
                        if (brace_count == 0) {
                            obj_start = pos;
                        }
                        brace_count++;
                    } else if (ch == '}') {
                        brace_count--;
                        if (brace_count == 0 && obj_start != std::string::npos) {
                            std::string ip_json = ip_list_str.substr(obj_start, pos - obj_start + 1);
                            
                            try {
                                auto ip_model = DNSIPModel::from_json(ip_json);
                                if (ip_model) {
                                    model->ip_list_.push_back(ip_model);
                                }
                            } catch (...) {
                            }
                            obj_start = std::string::npos;
                        }
                    }
                    pos++;
                }
            }
        }
        return model;
    }
}