//
// Created by Hongmingwei on 2025/10/28.
//

#include "CacheManager.h"

#include "Singleton.h"
#include "SerializeTask.h"


CacheManager *CacheManager::get_instance() {
    return Singleton<CacheManager>::get_instance();
}

CacheManager::CacheManager() {

}

CacheManager::~CacheManager() {

}

void CacheManager::start_serialize_task() {
    SerializeTask::get_instance()->start();
}

int64_t CacheManager::get_file_size(const char *file_path_url) {
    int64_t file_size = MemoryCache::get_instance()->get_file_size(file_path_url);
    if (file_size > 0) {
        return file_size;
    }
    file_size = DiskCache::get_instance()->get_file_size(file_path_url);
    return file_size;
}

int64_t CacheManager::get_instance_parameter(const std::string &file_key,
                                             const std::string &parameter_key) {
    int64_t value = MemoryCache::get_instance()->get_instance_parameter(file_key, parameter_key);
    if (value > 0) {
        return value;
    }
    value = DiskCache::get_instance()->get_instance_parameter(file_key, parameter_key);
    return value;
}

bool CacheManager::is_cache_complete(const std::string& file_key) {
    bool cache_complete = false;
    cache_complete = MemoryCache::get_instance()->is_cache_complete(file_key);
    if (!cache_complete) {
        cache_complete = DiskCache::get_instance()->is_cache_complete(file_key);
    }
    return cache_complete;
}
