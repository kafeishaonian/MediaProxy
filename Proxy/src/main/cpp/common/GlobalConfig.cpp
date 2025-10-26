//
// Created by 魏红明 on 2025/10/26.
//

#include "GlobalConfig.h"
#include "Singleton.h"
#include "PreloadCommon.h"

GlobalConfig::GlobalConfig() {

    cache_lock_timeout_in_ms_ = 500;
}

GlobalConfig::~GlobalConfig() {

}

GlobalConfig *GlobalConfig::get_instance() {
    return Singleton<GlobalConfig>::get_instance();
}

int GlobalConfig::get_cache_lock_timeout_in_ms() {
    proxy::WriteLock lock(read_write_lock_);
    return cache_lock_timeout_in_ms_;
}