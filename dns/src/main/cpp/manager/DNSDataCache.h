//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSDATACACHE_H
#define MEDIAPROXY_DNSDATACACHE_H

#include <mutex>

#include "DNSCommon.h"

namespace dns{

    class DNSDataCache {
    public:
        explicit DNSDataCache(size_t cache_size = Constants::DEFAULT_CACHE_SIZE);
        ~DNSDataCache();

        void set_cache_dir(const std::string& dir);


    private:
        std::string cache_dir_;

    };

}


#endif //MEDIAPROXY_DNSDATACACHE_H
