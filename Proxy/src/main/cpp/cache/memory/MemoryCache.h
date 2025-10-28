//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_MEMORYCACHE_H
#define MEDIAPROXY_MEMORYCACHE_H

#include <iostream>
#include <map>
#include <shared_mutex>

#include "MemoryMediaCache.h"
#include "CacheInfo.h"

class MemoryCache {

public:
    MemoryCache();

    ~MemoryCache();

    static MemoryCache *get_instance();

    int64_t get_file_size(const std::string &file_key);

    int64_t read_data(const char *file_path_url,
                      uint8_t *buffer,
                      uint64_t offset,
                      uint64_t buffer_size);

    int64_t write_data(const char *file_path_url,
                       uint8_t *buffer,
                       int64_t offset,
                       int64_t buffer_size,
                       int64_t file_size = 0);

    void dump_data(const char *file_path_url);

    void dump_all_data();

    void serialize_all();

    void serialize();

    void serialize_expired_cache();

    std::pair<uint64_t, uint64_t> query_remain_data_by_offset(const char *file_key,
                                                              uint64_t offset);

    int query_empty_segment(const char *file_key,
                            std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector);

    int64_t get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    void get_instance_parameter_with_map(const std::string &file_key, Int64Map &map);

    void set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                int64_t value);

    int64_t calculate_memory_usage();

    int64_t get_memory_usage();

    void drop_all();

    bool is_cache_complete(const std::string &file_key);

    bool query_data_range_exist(const std::string &file_key,
                                int64_t start,
                                int64_t size);

    int64_t get_cache_info(std::shared_ptr<CacheInfo> &info);

private:
    std::shared_ptr<MemoryMediaCache> find_memory_media_cache(const std::string &file_key);

    std::shared_ptr<MemoryMediaCache>
    find_or_create_memory_media_cache(const std::string &file_key);

private:

    std::shared_mutex read_write_lock_;

    std::map<std::string, std::shared_ptr<MemoryMediaCache>> memory_media_lists_;

    int64_t memory_usage_;
};

#endif //MEDIAPROXY_MEMORYCACHE_H
