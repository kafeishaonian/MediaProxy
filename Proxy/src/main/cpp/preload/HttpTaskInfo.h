//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_HTTPTASKINFO_H
#define MEDIAPROXY_HTTPTASKINFO_H

#include <cstdio>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <utility>

#include "ITaskInfo.h"
#include "STLCommon.h"

struct HttpTaskInfo : public ITaskInfo {

    HttpTaskInfo(
            int task_id,
            TASK_STATUS task_status,
            std::string url,
            std::string key,
            std::string http_header,
            TASK_PRIORITY priority,
            int64_t request_start,
            int64_t request_size,
            int64_t duration,
            int64_t download_limit_rate,
            int64_t add_timestamp,
            std::string session_id,
            bool is_first_network_packet
    ) : url_(std::move(url)),
        http_header_(std::move(http_header)),
        tcp_connect_used_time_(-1),
        dns_used_time_(-1),
        http_header_time_(-1),
        http_body_time_(-1),
        http_code_(-1),
        cdn_ip_(),
        ITaskInfo(
                task_id,
                task_status,
                std::move(key),
                priority,
                request_start,
                request_size,
                duration,
                download_limit_rate,
                add_timestamp,
                std::move(session_id),
                is_first_network_packet,
                TRANSFER_TYPE_HTTP
        ) {};

    ~HttpTaskInfo() {};


    std::string url_;
    std::string http_header_;

    int64_t tcp_connect_used_time_{};
    int64_t dns_used_time_{};
    int64_t http_header_time_{};
    int64_t http_body_time_{};
    int http_code_{};

    std::string cdn_ip_;
    StringVector resolved_address_;
    std::string vendor_;
    std::string content_type_;

    std::string to_json() override {
        boost::property_tree::ptree root;
        ITaskInfo::to_json(root);
        root.put("url", url_);
        root.put("http_header", http_header_);
        root.put("tcp_connect_used_time", tcp_connect_used_time_);
        root.put("dns_used_time", dns_used_time_);
        root.put("http_header_time", http_header_time_);
        root.put("http_body_time", http_body_time_);
        root.put("http_code", http_code_);
        root.put("cdn_ip", cdn_ip_);
        root.put("vendor", vendor_);
        root.put("content_type", content_type_);

        // 序列化DNS解析结果数组
        boost::property_tree::ptree resolved_address_array;
        for(const auto& item : resolved_address_) {
            boost::property_tree::ptree child;
            child.put("", item);
            resolved_address_array.push_back(std::make_pair("", child));
        }
        root.add_child("resolved_address", resolved_address_array);

        std::stringstream stream;
        try {
            boost::property_tree::write_json(stream, root);
        } catch (std::exception& exception) {
        }

        return stream.str();
    }

    static int to_info(const std::string& task_info_with_json, HttpTaskInfo& task_info) {
        boost::property_tree::ptree root;
        std::stringstream stream;

        stream << task_info_with_json;
        try {
            boost::property_tree::read_json(stream, root);
        } catch (std::exception& exception) {
            std::cout << exception.what() << std::endl;
            return -1;
        }

        ITaskInfo::to_info(root, task_info);

        task_info.url_ = root.get<std::string>("url", "");
        task_info.http_header_ = root.get<std::string>("http_header", "");
        task_info.tcp_connect_used_time_ = root.get<int64_t>("tcp_connect_used_time", -1);
        task_info.dns_used_time_ = root.get<int64_t>("dns_used_time", -1);
        task_info.http_header_time_ = root.get<int64_t>("http_header_time", -1);
        task_info.http_body_time_ = root.get<int64_t>("http_body_time", -1);
        task_info.http_code_ = root.get<int>("http_code", -1);
        task_info.cdn_ip_ = root.get<std::string>("cdn_ip", "");
        task_info.vendor_ = root.get<std::string>("vendor", "");
        task_info.content_type_ = root.get<std::string>("content_type", "");

        // 反序列化DNS解析结果数组
        task_info.resolved_address_.clear();
        auto resolved_address_node = root.get_child_optional("resolved_address");
        if (resolved_address_node) {
            for (const auto& item : *resolved_address_node) {
                task_info.resolved_address_.push_back(item.second.get_value<std::string>());
            }
        }

        return 0;
    }

};

#endif //MEDIAPROXY_HTTPTASKINFO_H
