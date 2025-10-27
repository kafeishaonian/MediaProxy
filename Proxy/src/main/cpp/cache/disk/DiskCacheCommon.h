//
// Created by Hongmingwei on 2025/10/27.
//

#ifndef MEDIAPROXY_DISKCACHECOMMON_H
#define MEDIAPROXY_DISKCACHECOMMON_H

#include <string>

typedef struct CacheFileInfo{
    std::string  file_key_;
    uint64_t     file_size_;
    uint64_t     access_time_;
    uint64_t     cache_size_;
    std::string  file_sign_;
    int64_t      share_num_;

    uint64_t get_file_size() {
        return file_size_;
    }

    uint64_t get_access_time() {
        return access_time_;
    }

    uint64_t get_cache_size() {
        return cache_size_;
    }

    std::string get_file_key() {
        return file_key_;
    }

    bool is_cache_complete() {
        return (file_size_ > 0 && cache_size_ > 0 && cache_size_ >= file_size_);
    }

    int64_t get_share_num() {
        return share_num_;
    }

} CacheFileInfo;

#endif //MEDIAPROXY_DISKCACHECOMMON_H
