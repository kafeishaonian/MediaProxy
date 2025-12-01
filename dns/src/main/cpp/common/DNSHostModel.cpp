//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSHostModel.h"

#include <algorithm>
#include <sstream>
#include <chrono>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

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

    std::shared_ptr<dns::DNSIPModel> DNSHostModel::get_best_ip() const {
        if (ip_list_.empty()) {
            return nullptr;
        }

        for (const auto &ip: ip_list_) {
            if (ip->is_valid() && ip->get_speed() >= 0) {
                return ip;
            }
        }

        return ip_list_[0];
    }


    std::string DNSHostModel::get_best_ip_string() const {
        auto best_ip = get_best_ip();
        return best_ip ? best_ip->get_ip() : "";
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
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        doc.AddMember("hostname", rapidjson::Value(hostname_.c_str(), allocator), allocator);
        doc.AddMember("update_time", update_time_, allocator);
        doc.AddMember("server_type", static_cast<int>(server_type_), allocator);

        rapidjson::Value ip_array(rapidjson::kArrayType);
        for (const auto& ip : ip_list_) {
            rapidjson::Document ip_doc;
            ip_doc.Parse(ip->to_json().c_str());
            rapidjson::Value ip_val(ip_doc, allocator);
            ip_array.PushBack(ip_val, allocator);
        }
        doc.AddMember("ip_list", ip_array, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }

    std::shared_ptr<dns::DNSHostModel> DNSHostModel::from_json(const std::string &json) {
        rapidjson::Document doc;
        if (doc.Parse(json.c_str()).HasParseError()) {
            return nullptr;
        }

        auto model = std::make_shared<DNSHostModel>();

        if (doc.HasMember("hostname") && doc["hostname"].IsString()) {
            model->hostname_ = doc["hostname"].GetString();
        }

        if (doc.HasMember("update_time") && doc["update_time"].IsInt64()) {
            model->update_time_ = doc["update_time"].GetInt64();
        }

        if (doc.HasMember("server_type") && doc["server_type"].IsInt()) {
            model->server_type_ = static_cast<DNSServerType>(doc["server_type"].GetInt());
        }

        if (doc.HasMember("ip_list") && doc["ip_list"].IsArray()) {
            const auto& ip_array = doc["ip_list"].GetArray();
            for (const auto& ip_json : ip_array) {
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                ip_json.Accept(writer);

                auto ip_model = DNSIPModel::from_json(buffer.GetString());
                if (ip_model) {
                    model->ip_list_.push_back(ip_model);
                }
            }
        }

        return model;
    }
}