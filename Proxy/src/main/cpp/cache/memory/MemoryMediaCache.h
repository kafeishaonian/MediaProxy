//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_MEMORYMEDIACACHE_H
#define MEDIAPROXY_MEMORYMEDIACACHE_H

#include <iostream>
#include <list>
#include <vector>
#include <shared_mutex>

#include "STLCommon.h"

#define TAG "MemoryMediaCache"

struct MemoryMediaSlice {
    uint64_t offset_;
    uint64_t size_;
    uint64_t timestamp_;
    std::vector<uint8_t> buffer_;
};

class Int64Map;

class MemoryMediaCache {

public:
    MemoryMediaCache(const std::string &file_key);

    // 读取数据
    int64_t read_data(uint8_t *buffer, uint64_t offset, uint64_t size);

    //写入数据
    int64_t write_data(uint8_t *buffer, int64_t offset, uint64_t size, int64_t file_size = 0);

    // 获取文件大小
    int64_t get_file_size();

    // 设置文件的大小
    void set_file_size(uint64_t file_size);

    void dump_data();

    void set_file_key(std::string file_key);

    void serialize();

    int64_t serialize_expired_cache();

    bool is_cache_expired();

    std::pair<uint64_t, uint64_t> query_remain_data_by_offset(uint64_t offset);

    int query_empty_segment(std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector);

    int64_t get_instance_parameter(const std::string &parameter_key);

    void get_instance_parameter_with_map(Int64Map &map);

    void set_instance_parameter(const std::string &parameter_key, int64_t value);

    int64_t calculate_memory_usage();

    bool is_cache_complete();

    bool query_data_range_exist(int64_t start, int64_t size);


private:
    // 查找slice
    int find_slice(uint64_t offset, uint64_t size, MemoryMediaSlice **slice);


private:
    // 视频文件key
    std::string file_key_;

    // 对应的文件大小
    int64_t file_size_;

    //已预加载 audio duration
    int64_t audio_duration_;

    // 已预加载 video duration
    int64_t video_duration_;

    // 已缓存的大小
    int64_t cached_size_;

    // 预加载的大小
    int64_t preload_size_;

    // 文件的分片信息和数据
    std::list<MemoryMediaSlice> media_file_slices_;

    std::shared_mutex read_write_lock_;

    uint64_t access_time_;

    int64_t memory_usage_;

    bool is_dump_;


};

#endif //MEDIAPROXY_MEMORYMEDIACACHE_H

