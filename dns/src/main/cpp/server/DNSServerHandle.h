//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSERVERHANDLE_H
#define MEDIAPROXY_DNSSERVERHANDLE_H

#include <mutex>

#include "DNSCommon.h"
#include "DNSHostModel.h"
#include "DNSHttpClient.h"

namespace dns {

    class DNSHostManager;

    class DNSHttpClient;

    class DNSServerHandle {
    public:
        explicit DNSServerHandle(DNSServerType server_type);

        virtual ~DNSServerHandle() = default;

        virtual std::shared_ptr<DNSHostModel> resolve(const std::string &hostname) = 0;

        DNSServerType get_server_type() const;

        void set_host_manager(std::shared_ptr<DNSHostManager> manager);

    protected:
        DNSServerType server_type_;
        std::shared_ptr<DNSHostManager> host_manager_;
    };


    class DNSSystemServerHandle : public DNSServerHandle {
    public:
        DNSSystemServerHandle();

        ~DNSSystemServerHandle() override = default;

        std::shared_ptr<DNSHostModel> resolve(const std::string &hostname) override;

    private:
        bool resolve_with_get_addr_info(const std::string &hostname,
                                        std::vector<std::string> &ip_list);
    };

    class DNSHttpServerHandle : public DNSServerHandle {
    public:
        explicit DNSHttpServerHandle(const std::string &doh_server);

        ~DNSHttpServerHandle() override = default;

        std::shared_ptr<DNSHostModel> resolve(const std::string &hostname) override;

        void set_doh_server(const std::string &server);

        std::string get_doh_server() const;

    private:
        std::string send_doh_request(const std::string &hostname);

        std::shared_ptr<DNSHostModel> parse_doh_response(const std::string &response,
                                                         const std::string &hostname);

    private:
        std::string doh_server_;
        std::shared_ptr<DNSHttpClient> http_client_;
    };

    class DNSLocalServerHandle : public DNSServerHandle {
    public:
        DNSLocalServerHandle();

        ~DNSLocalServerHandle() override = default;

        std::shared_ptr<DNSHostModel> resolve(const std::string &hostname) override;

        void add_to_cache(const std::string &hostname, std::shared_ptr<DNSHostModel> host);

        void remove_from_cache(const std::string &hostname);

        void clear_cache();

        void set_cache_expire_time(int seconds);

    private:
        bool is_cache_expired(const std::shared_ptr<DNSHostModel> &host) const;

    private:
        std::map<std::string, std::shared_ptr<DNSHostModel>> cache_;
        std::mutex cache_mutex_;
        int cache_expire_time_;
    };
}


#endif //MEDIAPROXY_DNSSERVERHANDLE_H
