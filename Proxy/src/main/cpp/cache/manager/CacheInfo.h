//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_CACHEINFO_H
#define MEDIAPROXY_CACHEINFO_H

#include <cstdio>
#include <string>
#include <vector>

class CacheInfo {

    CacheInfo(
            const std::string &key,
            const int64_t offset
    ) : file_key_(key),
        file_size_(0),
        offset_(offset),
        cache_data_count_(0),
        min_preload_duration_(-1),
        is_complete_(false) {};

    enum {
        CacheSize = 1024 * 1024,
    };

    std::string file_key_;
    int64_t file_size_;
    int64_t offset_;
    std::vector<uint8_t> cache_data_;
    int64_t cache_data_count_;
    int64_t min_preload_duration_;
    bool is_complete_;
};


#endif //MEDIAPROXY_CACHEINFO_H
