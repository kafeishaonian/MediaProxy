//
// Created by Hongmingwei on 2025/10/23.
//

#ifndef MEDIAPROXY_FILECACHEINTERFACE_H
#define MEDIAPROXY_FILECACHEINTERFACE_H

#include <string>
#include <list>
#include <map>
#include <mutex>
#include <vector>

#include "STLCommon.h"


typedef struct CacheSegment{
    std::string seg_name_;
    uint64_t seg_start_position_;
    uint64_t seg_file_size_;

    CacheSegment() {

    }

    CacheSegment(const std::string &seg_name, uint64_t seg_start_position, uint64_t seg_file_size) {
        seg_name_ = seg_name;
        seg_start_position_ = seg_start_position;
        seg_file_size_ = seg_file_size;
    }
} CacheSegment;



class FileCacheInterface {

public:
    using CacheSegmentIteratorType = std::list<std::shared_ptr<CacheSegment>>::iterator;

    FileCacheInterface();

    virtual int parse();

    virtual int try_parse();

    int is_config_file_exist();

    int try_is_config_file_exist();

    void set_cache_path(const std::string & path);

    void set_file_key(const std::string& file_key);

    void set_config_file_name(const std::string& config_file_name);

    void set_segment_size(int segment_size);

    std::string generate_seg_name();

    std::string generate_index_name(int64_t offset);

    bool check_info();

    std::string get_config_file_full_name();

    std::string get_video_file_full_name(const std::string& seg_name);

    int is_cache_path_exist();

    int is_video_file_path_exist();

    virtual int64_t flush_config_file() = 0;

    void set_file_size(uint64_t file_size);

    virtual size_t write_data(uint8_t *buffer, uint64_t offset, uint64_t size);

    virtual size_t read_data(uint8_t *buffer, uint64_t offset, uint64_t size);

    virtual void print_segment();

    virtual bool is_cache_complete() = 0;

    virtual std::pair<uint64_t, uint64_t> query_remain_data_by_offset(uint64_t offset) = 0;

    virtual int query_empty_segment(std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector) = 0;

    void set_video_file_path(const std::string& path);

    virtual std::pair<uint64_t, uint64_t> find_data(uint64_t offset, uint64_t size) = 0;

    std::string& get_file_key();

    uint64_t get_file_size();

    uint64_t get_access_time();

    uint64_t get_cache_size();

    void add_share_num();

    int64_t get_share_num();

    void set_preload_size(int64_t size);

    int64_t get_preload_size();

    void set_preload_audio_duration(int64_t duration);

    int64_t get_preload_audio_duration();

    void set_preload_video_duration(int64_t duration);

    int64_t get_preload_video_duration();

    int64_t get_instance_parameter(const std::string& parameter_key);

    int64_t try_get_instance_parameter(const std::string& parameter_key);

    int try_get_instance_parameter_with_map(Int64Map& map);

    void set_instance_parameter(const std::string& parameter_key, int64_t value);

    void set_parameter(const StringMap& parameter_map);

    virtual int remove() = 0;

    virtual bool query_data_range_exist(int64_t start, int64_t size) = 0;

    virtual std::string& calc_file_sign() = 0;

private:

    int is_config_file_exist_internal();

protected:

    // 获取分段index
    int get_index_from_offset(int64_t offset);

    int remove_files_at_path(const std::string& path);

    bool is_cache_complete_internal();

protected:

    std::string cache_path_;

    std::string file_key_;

    std::string file_key_md5_;

    std::string video_file_path_;


    int mapped_file_segment_size_;

    int mapped_config_file_segment_size_;

    bool parsed_;

    uint64_t file_size_;

    int64_t preload_size_;

    int64_t share_num_;

    int seg_num_;

    uint64_t access_time_;

    int64_t audio_duration_;

    int64_t video_duration_;

    std::string file_sign_;

    int64_t cached_size_;

    int config_version_;

    std::timed_mutex read_write_lock_;

private:
    std::string config_file_name;

    std::string config_file_full_name;
};

using PairCacheConfigFile = std::pair<std::string, std::shared_ptr<FileCacheInterface>>;

using FileCacheInterfaceMap = std::map<std::string, std::shared_ptr<FileCacheInterface>>;

typedef enum : int {
    DiskCacheStatusOK = 0,
    DiskCacheStatusPathNotExist = -1,
    DiskCacheStatusDiskFull = -2,
} DiskCacheStatus;

#endif //MEDIAPROXY_FILECACHEINTERFACE_H
