//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSEntrance.h"


namespace dns {

    std::unordered_map<std::string, std::shared_ptr<DNSEntrance>> DNSEntranceImpl::instances_;
    std::mutex DNSEntranceImpl::instance_mutex_;

    DNSEntranceImpl::DNSEntranceImpl()
            : network_state_(DNSAppNetState::UNKNOWN),
              cache_dir_("/data/local/tmp/dns"),
              doh_server_("https://dns.google/dns-query"),
              system_dns_enabled_(true),
              http_dns_enabled_(false),
              local_cache_enabled_(true) {

        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已创建");
    }

    DNSEntranceImpl::~DNSEntranceImpl() noexcept {
        if (dns_server_) {
//            dns_server_->stop();
        }
        Logger::log(LogLevel::INFO, "DNSEntrance", "DNS入口已销毁");
    }


    std::shared_ptr<dns::DNSEntrance> DNSEntranceImpl::get_instance(const std::string &key) {
        std::lock_guard<std::mutex> local(instance_mutex_);

        auto it = instances_.find(key);
        if (it != instances_.end()) {
            return it->second;
        }

        auto instance = std::make_shared<DNSEntranceImpl>();
        instances_[key] = instance;

        Logger::log(LogLevel::INFO, "DNSEntrance",
                    "创建新实例: " + key);

        return instance;
    }


    void DNSEntranceImpl::init() {
        std::lock_guard<std::mutex> lock(mutex_);

        Logger::log(LogLevel::INFO, "DNSEntrance", "开始初始化DNS服务");

        data_cache_ = std::make_shared<DNSDataCache>(Constants::DEFAULT_CACHE_SIZE);
        data_cache_->set_cache_dir(cache_dir_);

    }

}