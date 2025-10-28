//
// Created by Hongmingwei on 2025/10/28.
//

#ifndef MEDIAPROXY_CACHEMANAGER_H
#define MEDIAPROXY_CACHEMANAGER_H

#include <iostream>

#include "CacheInfo.h"
#include "SpinMutex.h"

class CacheManager {

public:
    static CacheManager *get_instance();

    CacheManager();

    ~CacheManager();

    void start_serialize_task();

};


#endif //MEDIAPROXY_CACHEMANAGER_H
