//
// Created by Hongmingwei on 2025/10/28.
//

#ifndef MEDIAPROXY_CACHEMANAGER_H
#define MEDIAPROXY_CACHEMANAGER_H

#include <iostream>

#include "CacheInfo.h"
#include "SpinMutex.h"

#include "MemoryCache.h"
#include "DiskCache.h"

class CacheManager {

public:
    static CacheManager *get_instance();

    CacheManager();

    ~CacheManager();

    void start_serialize_task();

    int64_t get_file_size(const char *file_path_url);

    int64_t get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    bool is_cache_complete(const std::string& file_key);
};


#endif //MEDIAPROXY_CACHEMANAGER_H
