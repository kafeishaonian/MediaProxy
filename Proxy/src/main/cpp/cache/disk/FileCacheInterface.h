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
#include "MappedFile.h"


typedef struct CacheSegment {
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

    int parse();

    int try_parse();

    int is_config_file_exist();

    int try_is_config_file_exist();

    void set_cache_path(const std::string &path);

    void set_file_key(const std::string &file_key);

    void set_config_file_name(const std::string &config_file_name);

    void set_segment_size(int segment_size);

    std::string generate_seg_name();

    std::string generate_index_name(int64_t offset);

    bool check_info();

    std::string get_config_file_full_name();

    std::string get_video_file_full_name(const std::string &seg_name);

    int is_cache_path_exist();

    int is_video_file_path_exist();

    int64_t flush_config_file();

    void set_file_size(uint64_t file_size);

    size_t write_data(uint8_t *buffer, uint64_t offset, uint64_t size);

    size_t read_data(uint8_t *buffer, uint64_t offset, uint64_t size);

    void print_segment();

    bool is_cache_complete();

    std::pair<uint64_t, uint64_t> query_remain_data_by_offset(uint64_t offset);

    int query_empty_segment(std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector);

    void set_video_file_path(const std::string &path);

    std::string &get_file_key();

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

    int64_t get_instance_parameter(const std::string &parameter_key);

    int64_t try_get_instance_parameter(const std::string &parameter_key);

    int try_get_instance_parameter_with_map(Int64Map &map);

    void set_instance_parameter(const std::string &parameter_key, int64_t value);

    void set_parameter(const StringMap &parameter_map);

    int remove();

    std::string &calc_file_sign();

private:

    int is_config_file_exist_internal();

    // 获取分段index
    int get_index_from_offset(int64_t offset);

    int remove_files_at_path(const std::string &path);

    bool is_cache_complete_internal();

    int parse_json_with_rapid_json(const std::string& config_file_name, const std::string& file_key);

    int64_t read_from_local_file(const std::string& seg_name, uint8_t * buffer, uint64_t offset, uint64_t size);

    void update_segment(int64_t current_offset, int64_t write_size);

    std::vector<CacheSegment> merge_interval(const std::vector<CacheSegment>& input);

    int parse_internal();

    int64_t get_update_cached_size();

    int64_t flush_config_file_internal();

    size_t read_data_internal(uint8_t *buffer, uint64_t offset, uint64_t size);

    std::string get_config_json();

    std::vector<CacheSegment> get_segment_vector();

    std::string &calc_file_sign_iternal();

private:

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

    std::timed_mutex read_write_lock_;

    std::string config_file_name_;

    std::string config_file_full_name_;

    std::map<std::string, std::shared_ptr<MappedFile>> mapped_file_list_;

    std::shared_ptr<MappedFile> mapped_config_file_;

    std::map<int, std::vector<CacheSegment>> segments_map_;
};

using PairCacheConfigFile = std::pair<std::string, std::shared_ptr<FileCacheInterface>>;

using FileCacheInterfaceMap = std::map<std::string, std::shared_ptr<FileCacheInterface>>;

typedef enum : int {
    DiskCacheStatusOK = 0,
    DiskCacheStatusPathNotExist = -1,
    DiskCacheStatusDiskFull = -2,
} DiskCacheStatus;


std::shared_ptr<FileCacheInterface>
create_file_disk_cache(const std::string &cache_path, const std::string &file_key);

#endif //MEDIAPROXY_FILECACHEINTERFACE_H
