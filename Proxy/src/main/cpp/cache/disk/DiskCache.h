//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_DISKCACHE_H
#define MEDIAPROXY_DISKCACHE_H

//
//#include "MCacheConfigFile.hpp"
//

#include <list>
#include <map>
#include <cstdio>
#include <string>
#include <shared_mutex>

#include "DiskCacheCommon.h"
#include "STLCommon.h"
#include "CacheInfo.h"
#include "FileCacheInterface.h"
#include "LRUCache.h"
#include "ICacheFileChangeListener.h"


class DiskCache {

public:

    DiskCache();

    ~DiskCache();

    static DiskCache *get_instance();

    void set_cache_path(const char *path);

    int64_t get_file_size(const char *file_path_url);

    void set_preload_size(const std::string &file_key, int64_t preload_size);

    int64_t get_preload_size(const std::string &file_path_url);

    void set_audio_duration(const std::string &file_key, int64_t duration);

    int64_t get_audio_duration(const std::string &file_key);

    void set_video_duration(const std::string &file_key, int64_t duration);

    int64_t get_video_duration(const std::string &file_key);

    int64_t get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    int64_t
    try_get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    int try_get_instance_parameter_with_map(const std::string &file_key, Int64Map &map);

    void set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                int64_t value);

    std::pair<uint64_t, uint64_t>
    query_remain_data_by_offset(const char *file_key, uint64_t offset);

    int query_empty_segment(const char *file_key,
                            std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector);

    int64_t
    read_data(const char *file_path_url, uint8_t *buffer, uint64_t offset, uint64_t size);

    int64_t write_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                      uint64_t size,
                      uint64_t file_size = 0);

    int64_t flush_config_file(const std::string &file_path_url);

    int64_t clear_cache();

    void clear_all_cache();

    int64_t clear_cache_with_key(const std::string &file_key);

    int get_cache_file_info_with_key(const std::string &fileKey, CacheFileInfo &fileInfo);

    int get_cache_complete_and_limit_size_file_list(std::vector<std::string> &file_keys);

    void set_cache_file_changed_listener(std::weak_ptr<ICacheFileChangeListener> listener);

    bool is_cache_complete(const std::string &file_key);

    std::string cale_file_sign(const std::string &file_key);

    int64_t get_cache_info(std::shared_ptr<CacheInfo> &info);

private:

    std::shared_ptr<FileCacheInterface> build_config_file_with_file_key(const std::string &file_key,
                                                                        bool add_top_map = true);

    int get_all_cache_size();


    int64_t check_if_remove_cache(int64_t all_cached_size);

    void remove_file_notification_with_file_keys(const StringList &file_keys);

    std::weak_ptr<ICacheFileChangeListener> cache_file_change_listener_;


    bool is_file_key_exist_in_map(const std::string &file_key);

    void remove_all_cache();

private:
    std::string cache_path_string_;

    std::timed_mutex read_write_lock_;

    StringList file_removed_list_;

    proxy::LRUCache<std::string, std::shared_ptr<FileCacheInterface>> files_map_;

    std::vector<CacheFileInfo> total_file_list_;

    int64_t all_cached_size_;
};


#endif //MEDIAPROXY_DISKCACHE_H
