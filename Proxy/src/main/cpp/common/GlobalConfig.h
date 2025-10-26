//
// Created by 魏红明 on 2025/10/26.
//

#ifndef MEDIAPROXY_GLOBALCONFIG_H
#define MEDIAPROXY_GLOBALCONFIG_H

#include <cstdio>
#include <string>
#include <list>

#include "ThreadUtil.h"

const uint64_t GlobalConfigPlayerPreloadSize = 1 * 1024 * 1024;
const uint64_t GlobalConfigProxyServerPort = 10101;


class GlobalConfig {

public:
    static GlobalConfig *get_instance();

    GlobalConfig();

    ~GlobalConfig();

    int get_cache_lock_timeout_in_ms();


private:


    std::shared_mutex read_write_lock_;

    int cache_lock_timeout_in_ms_;
};


#endif //MEDIAPROXY_GLOBALCONFIG_H
