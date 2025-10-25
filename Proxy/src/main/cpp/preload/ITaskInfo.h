//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_ITASKINFO_H
#define MEDIAPROXY_ITASKINFO_H

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <list>
#include <utility>

#include "PreloadCommon.h"

struct ITaskInfo {
    ITaskInfo(
            int task_id,
            TASK_STATUS task_status,
            std::string key,
            TASK_PRIORITY priority,
            int64_t request_start,
            int64_t request_size,
            int64_t duration,
            int64_t limit_rate,
            int64_t add_timestamp,
            std::string session_id,
            bool is_first_network_packet,
            TRANSFER_TYPE transfer_type
    ) : task_id_(task_id),
        status_(task_status),
        key_(std::move(key)),
        priority_(priority),
        request_start_(request_start),
        request_size_(request_size),
        duration_(duration),
        download_limit_rate_(limit_rate),
        add_timestamp_(add_timestamp),
        session_id_(std::move(session_id)),
        is_first_network_packet_(is_first_network_packet),
        transfer_type_(transfer_type),

        download_size_(0),
        download_size_exclusive_parse_mp4_(0),
        current_download_size_(0),
        download_duration_(-1),
        connect_used_time_(-1),
        used_time_(0),
        complete_timestamp_(-1),
        download_rate_(0),
        average_rate_(0),
        wait_time_(-1),
        exit_timestamp_(-1),
        end_reason_code_(END_NO_END),
        end_reason_sub_code_(END_NO_END),
        end_reason_str_(),
        first_write_timestamp_(-1),
        get_file_size_cost_(0),
        start_use_timestamp_(-1),
        end_use_timestamp_(-1),
        current_use_time_(-1),
        preload_thread_number_(0),
        before_connect_timestamp_(0),
        after_connect_timestamp_(0) {};

    virtual ~ITaskInfo() = default;

    int task_id_{};
    TASK_STATUS status_;
    std::string key_;
    TASK_PRIORITY priority_;
    int64_t request_start_{};
    int64_t request_size_{};
    int64_t duration_{};

    int64_t download_size_{};
    int64_t download_size_exclusive_parse_mp4_{};
    int64_t current_download_size_{};
    int64_t download_duration_{};
    int64_t download_rate_{};
    int64_t download_limit_rate_{};
    int64_t average_rate_{};

    int64_t connect_used_time_{};
    int64_t used_time_{};
    int64_t complete_timestamp_{};
    int64_t wait_time_{};
    int64_t add_timestamp_{};
    int64_t exit_timestamp_{};
    int64_t first_write_timestamp_{};
    int64_t start_use_timestamp_{};
    int64_t end_use_timestamp_{};
    int64_t current_use_time_{};
    int64_t before_connect_timestamp_{};
    int64_t after_connect_timestamp_{};
    uint64_t get_file_size_cost_{};

    TASK_END_REASON end_reason_code_;
    TASK_END_REASON end_reason_sub_code_;
    std::string end_reason_str_;
    std::string session_id_;
    bool is_first_network_packet_{};
    TRANSFER_TYPE transfer_type_;
    int preload_thread_number_{};

    virtual void to_json(boost::property_tree::ptree& root) {
        root.put("end_reason_code", end_reason_code_);
        root.put("end_reason_sub_code", end_reason_sub_code_);
        root.put("end_reason_str", end_reason_str_);
        root.put("transfer_type", transfer_type_);
        root.put("task_id", task_id_);
        root.put("status", status_);
        root.put("key", key_);
        root.put("priority", priority_);
        root.put("request_start", request_start_);
        root.put("request_size", request_size_);
        root.put("duration", duration_);

        root.put("download_size", download_size_);
        root.put("current_download_size", current_download_size_);
        root.put("download_duration", download_duration_);
        root.put("download_rate", download_rate_);
        root.put("download_limit_rate", download_limit_rate_);
        root.put("average_rate", average_rate_);
        
        root.put("connect_used_time", connect_used_time_);
        root.put("used_time", used_time_);
        root.put("complete_timestamp", complete_timestamp_);
        root.put("wait_time", wait_time_);
        root.put("add_timestamp", add_timestamp_);
        root.put("exit_timestamp", exit_timestamp_);
        root.put("session_id", session_id_);
        
        root.put("first_write_timestamp", first_write_timestamp_);
        root.put("is_first_network_packet", is_first_network_packet_);
        root.put("preload_thread_number", preload_thread_number_);
        root.put("get_file_size_cost", get_file_size_cost_);
        root.put("start_use_timestamp", start_use_timestamp_);
        root.put("end_use_timestamp", end_use_timestamp_);
        root.put("current_use_time", current_use_time_);
        root.put("before_connect_timestamp", before_connect_timestamp_);
        root.put("after_connect_timestamp", after_connect_timestamp_);
    }


    virtual std::string to_json() {
        boost::property_tree::ptree root;
        to_json(root);
        std::stringstream ss;
        boost::property_tree::write_json(ss, root);
        return ss.str();
    }

    static int to_info(boost::property_tree::ptree& root, ITaskInfo& taskInfo) {
        taskInfo.end_reason_code_ = (TASK_END_REASON)root.get<int>("end_reason_code", 0);
        taskInfo.end_reason_sub_code_ = (TASK_END_REASON)root.get<int>("end_reason_sub_code", 0);
        taskInfo.end_reason_str_ = root.get<std::string>("end_reason_str", "");
        taskInfo.transfer_type_ = (TRANSFER_TYPE)root.get<int>("transfer_type", -1);
        taskInfo.task_id_ = root.get<int>("task_id", 0);
        taskInfo.status_ = (TASK_STATUS)root.get<int>("status", 0);
        
        taskInfo.key_ = root.get<std::string>("key", "");
        taskInfo.priority_ = (TASK_PRIORITY)root.get<int>("priority", 0);
        taskInfo.request_start_ = root.get<int64_t>("request_start", 0);
        taskInfo.request_size_ = root.get<int64_t>("request_size", 0);
        taskInfo.duration_ = root.get<int64_t>("duration", 0);
        
        taskInfo.download_size_ = root.get<int64_t>("download_size", 0);
        taskInfo.current_download_size_ = root.get<int64_t>("current_download_size", 0);
        taskInfo.download_duration_ = root.get<int64_t>("download_duration", 0);
        taskInfo.download_rate_ = root.get<int64_t>("download_rate", 0);
        taskInfo.download_limit_rate_ = root.get<int64_t>("download_limit_rate", 0);
        taskInfo.average_rate_ = root.get<int64_t>("average_rate", 0);
        
        taskInfo.connect_used_time_ = root.get<int64_t>("connect_used_time", 0);
        taskInfo.used_time_ = root.get<int64_t>("used_time", 0);
        taskInfo.complete_timestamp_ = root.get<int64_t>("complete_timestamp", 0);
        taskInfo.wait_time_ = root.get<int64_t>("wait_time", 0);
        taskInfo.add_timestamp_ = root.get<int64_t>("add_timestamp", 0);
        taskInfo.exit_timestamp_ = root.get<int64_t>("exit_timestamp", 0);
        taskInfo.session_id_ = root.get<std::string>("session_id", "");
        
        taskInfo.first_write_timestamp_ = root.get<int64_t>("first_write_timestamp", 0);
        taskInfo.is_first_network_packet_ = root.get<bool>("is_first_network_packet", false);
        taskInfo.preload_thread_number_ = root.get<int>("preload_thread_number", -1);
        taskInfo.get_file_size_cost_ = root.get<uint64_t>("get_file_size_cost", 0);
        taskInfo.start_use_timestamp_ = root.get<int64_t>("start_use_timestamp", -1);
        taskInfo.end_use_timestamp_ = root.get<int64_t>("end_use_timestamp", -1);
        taskInfo.current_use_time_ = root.get<int64_t>("current_use_time", -1);
        taskInfo.before_connect_timestamp_ = root.get<int64_t>("before_connect_timestamp", 0);
        taskInfo.after_connect_timestamp_ = root.get<int64_t>("after_connect_timestamp", 0);

        return 0;
    }

    TRANSFER_TYPE get_transfer_type() {
        return transfer_type_;
    }

    TASK_PRIORITY get_task_priority() {
        return priority_;
    }

    std::string& get_key() {
        return key_;
    }

    int64_t get_request_start() const {
        return request_start_;
    }

    int64_t get_request_size() const {
        return request_size_;
    }


    virtual void setURL(const std::string& url) {

    }

    int64_t get_task_add_timestamp() const {
        return add_timestamp_;
    }

    bool get_is_first_network_packet_() const {
        return is_first_network_packet_;
    }

};


#endif //MEDIAPROXY_ITASKINFO_H
