//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSHOSTMANAGER_H
#define MEDIAPROXY_DNSHOSTMANAGER_H

#include <mutex>

#include "DNSCommon.h"
#include "DNSHostModel.h"
#include "DNSDataCache.h"

namespace dns {

    class DNSHostManager {
    public:
        DNSHostManager();
        ~DNSHostManager();

        std::shared_ptr<DNSHostModel> get_host(const std::string& hostname);
        void update_host(std::shared_ptr<DNSHostModel> host);
        void remove_host(const std::string& hostname);
        void clear_cache();


        std::vector<std::shared_ptr<DNSHostModel>> get_all_hosts();
        int get_host_count() const;

        void set_data_cache(std::shared_ptr<DNSDataCache> cache);
        void load_from_cache();
        void save_to_cache();

        void clean_expired_hosts(int expire_seconds = 3600);

        struct CacheStats {
            int total_hosts;
            int valid_hosts;
            int expired_hosts;
            long oldest_timestamp;
            long newest_timestamp;
        };

        CacheStats get_stats() const;

    private:
        bool is_host_expired(const std::shared_ptr<DNSHostModel>& host, int expire_seconds);

    private:
        std::map<std::string, std::shared_ptr<DNSHostModel>> host_cache_;
        mutable std::mutex cache_mutex_;
        std::shared_ptr<DNSDataCache> data_cache_;
    };
}


#endif //MEDIAPROXY_DNSHOSTMANAGER_H
