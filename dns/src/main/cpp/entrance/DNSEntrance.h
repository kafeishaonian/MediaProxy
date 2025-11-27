//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSENTRANCE_H
#define MEDIAPROXY_DNSENTRANCE_H

#include <mutex>

#include "DNSCommon.h"
#include "DNSServer.h"

namespace dns {

    class DNSEntrance {
    public:
        virtual ~DNSEntrance() = default;

        virtual void init() = 0;

        virtual std::string resolve_host(const std::string& hostname) = 0;

        virtual void resolve_host_async(const std::string& hostname, TaskCallback callback) = 0;

        virtual std::vector<std::string> get_all_ips(const std::string& hostname) = 0;

        virtual void set_network_state(DNSAppNetState state) = 0;

        virtual void clear() = 0;

    };


    class DNSEntranceImpl : public DNSEntrance,
                              public std::enable_shared_from_this<DNSEntranceImpl> {

    public:
        DNSEntranceImpl();
        ~DNSEntranceImpl() override;

        void init() override;
        std::string resolve_host(const std::string& hostname) override;

        void resolve_host_async(const std::string& hostname, TaskCallback callback) override;

        std::vector<std::string> get_all_ips(const std::string& hostname) override;

        void set_network_state(DNSAppNetState state) override;

        void clear() override;

        void set_doh_server(const std::string& server);

        void enable_system_dns(bool enable);

        void enable_http_dns(bool enable);

        void enable_local_cache(bool enable);

        void set_cache_dir(const std::string& dir);

        void set_cache_size(size_t size);

        void set_cache_expire_time(int seconds);

        void set_thread_count(int count);

        DNSServer::ServerStats get_stats() const;

        static std::shared_ptr<DNSEntrance> get_instance();

        static clear_instance();

    private:
        void init_dns_handlers();


    private:
        std::shared_ptr<DNSServer> dns_server_;
        std::shared_ptr<DNSHostManager> dns_manager_;
        std::shared_ptr<DNSDataCache> data_cache_;
        std::shared_ptr<DNSSpeedChecker> speed_checker_;

        DNSAppNetState network_state_;
        std::string cache_dir_;
        std::string doh_server_;

        bool system_dns_enabled_;
        bool http_dns_enabled_;
        bool local_cache_enabled_;

        mutable std::mutex mutex_;

        static std::shared_ptr<DNSEntrance> instance_;
        static std::mutex instance_mutex_;
    };

}




#endif //MEDIAPROXY_DNSENTRANCE_H
