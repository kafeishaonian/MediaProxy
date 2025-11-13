//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSHOSTMODEL_H
#define MEDIAPROXY_DNSHOSTMODEL_H

#include "DNSCommon.h"
#include "DNSIPModel.h"

namespace dns {
    class DNSHostModel {

    public:
        DNSHostModel();
        explicit DNSHostModel(const std::string& hostname);

        std::string get_hostname() const;

        std::vector<std::shared_ptr<DNSIPModel>> get_ip_list() const;

        long get_update_time() const;

        DNSServerType get_server_type() const;

        void set_hostname(const std::string& hostname);

        void set_update_time(long time);

        void set_server_type(DNSServerType type);

        //IP管理
        void add_ip(std::shared_ptr<DNSIPModel> ip);

        void clear_ips();

        bool has_valid_ip() const;

        //获取最快IP
        std::shared_ptr<DNSIPModel> get_bast_ip() const;

        std::string get_bast_ip_string() const;

        void sort_by_speed();

        std::string to_string() const;

        std::string to_json() const;

        static std::shared_ptr<DNSHostModel> from_json(const std::string& json);


    private:
        std::string hostname_;
        std::vector<std::shared_ptr<DNSIPModel>> ip_list_;
        long update_time_;
        DNSServerType server_type_;
    };
}


#endif //MEDIAPROXY_DNSHOSTMODEL_H