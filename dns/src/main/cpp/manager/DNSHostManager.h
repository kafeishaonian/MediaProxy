//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSHOSTMANAGER_H
#define MEDIAPROXY_DNSHOSTMANAGER_H

#include <shared_mutex>

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

        // 设置定期速度检测回调
        using SpeedCheckCallback = std::function<void(const std::string&, std::shared_ptr<DNSHostModel>)>;
        void set_speed_check_callback(SpeedCheckCallback callback);

        // 触发所有缓存主机的速度检测
        void trigger_speed_check_for_all();

        struct CacheStats {
            int total_hosts;
            int valid_hosts;
            int expired_hosts;
            long oldest_timestamp;
            long newest_timestamp;
        };

        CacheStats get_stats() const;

    private:
        static bool is_host_expired(const std::shared_ptr<DNSHostModel>& host, int expire_seconds);

    private:
        std::unordered_map<std::string, std::shared_ptr<DNSHostModel>> host_cache_;
        mutable std::shared_mutex cache_mutex_;
        std::shared_ptr<DNSDataCache> data_cache_;
        SpeedCheckCallback speed_check_callback_;
    };
}


#endif //MEDIAPROXY_DNSHOSTMANAGER_H
