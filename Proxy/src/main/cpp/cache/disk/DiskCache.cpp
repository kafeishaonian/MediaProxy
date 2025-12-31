//
// Created by Hongmingwei on 2025/10/24.
//

//#include "MCacheConfigFile.hpp"
//#include "MCacheConfigFileV2.hpp"
//#include "MJsonParser.hpp"
//#include "MLogTAG.h"
//#include "MP2PConfig.hpp"
//#include "MStatisTag.hpp"
//#include "MStatisUtil.hpp"
//#include "XlogAdpater.h"

#include "DiskCache.h"

#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <ftw.h>

#include "Singleton.h"
#include "ThreadUtil.h"
#include "Util.h"
#include "ProxyInterface.h"
#include "GlobalConfig.h"
#include "GlobalConstant.h"
#include "DiskCacheUtil.h"
#include "FileCacheInterface.h"


DiskCache *DiskCache::get_instance() {
    return Singleton<DiskCache>::get_instance();
}


DiskCache::DiskCache() : files_map_(20) {}

DiskCache::~DiskCache() {}

void DiskCache::set_cache_path(const char *path) {
    if (path) {
        cache_path_string_ = std::string(path);
    }
}

std::pair<uint64_t, uint64_t>
DiskCache::query_remain_data_by_offset(const char *file_key, uint64_t offset) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (config_file) {
        return config_file->query_remain_data_by_offset(offset);
    }
    return std::make_pair(0, 0);
}

int DiskCache::query_empty_segment(const char *file_key,
                                   std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }
    if (config_file) {
        return config_file->query_empty_segment(empty_segment_vector);
    }
    return -2;
}

int64_t DiskCache::read_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                             uint64_t size) {
    if (!file_path_url) {
        return 0;
    }

    if (!buffer) {
        return 0;
    }

    int64_t read_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }
    if (config_file) {
        int exist = config_file->is_config_file_exist();
        if (exist < 0) {
            return 0;
        }
        int result = config_file->parse();
        if (result < 0) {
            proxy::TimedLock lock(read_write_lock_);
            files_map_.erase(file_path_url);
        }
        read_size = config_file->read_data(buffer, offset, size);
    } else {
        read_size = 0;
    }
    return read_size;
}

int64_t DiskCache::write_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                              uint64_t size, uint64_t file_size) {
    if (file_path_url == nullptr) {
        return -1;
    }

    if (buffer == nullptr) {
        return -1;
    }

    int64_t write_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }

    if (config_file) {
        config_file->set_file_size(file_size);
        write_size = config_file->write_data(buffer, offset, size);
    } else {
        write_size = -1;
    }
    return write_size;
}

void
DiskCache::set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                  int64_t value) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }
    if (config_file) {
        config_file->set_instance_parameter(parameter_key, value);
    }
}

int64_t DiskCache::flush_config_file(const std::string &file_path_url) {
    if (file_path_url.empty()) {
        return -1;
    }

    int64_t write_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }

    if (config_file) {
        write_size = config_file->flush_config_file();
        if (config_file->is_cache_complete() && config_file->get_file_size() >
                                                GlobalConfig::get_instance()->get_min_file_size_upload_tracker()) {
            auto listener = cache_file_change_listener_.lock();
            if (listener) {
                listener->cache_added(config_file->get_file_key());
            }
        }
    }
    return write_size;
}


int64_t DiskCache::get_file_size(const char *file_path_url) {
    int64_t file_size = -1;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return 0;
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_path_url);
    }

    if (config_file) {
        file_size = config_file->get_file_size();
    }

    return file_size;

}

void DiskCache::set_preload_size(const std::string &file_key, int64_t preload_size) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return;
    }

    config_file->set_preload_size(preload_size);
}

int64_t DiskCache::get_preload_size(const std::string &file_path_url) {
    int64_t preload_size = -1;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }
    if (!config_file) {
        return -1;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return -1;
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_path_url);
    }

    if (config_file) {
        preload_size = config_file->get_preload_size();
    }

    return preload_size;
}

void DiskCache::set_audio_duration(const std::string &file_key, int64_t duration) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return;
    }

    if (config_file) {
        config_file->set_preload_audio_duration(duration);
    }
}

int64_t DiskCache::get_audio_duration(const std::string &file_key) {
    int64_t duration = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return -1;
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_key);
    }

    if (config_file) {
        duration = config_file->get_preload_audio_duration();
    }

    return duration;
}

void DiskCache::set_video_duration(const std::string &file_key, int64_t duration) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return;
    }

    if (config_file) {
        config_file->set_preload_video_duration(duration);
    }

}

int64_t DiskCache::get_video_duration(const std::string &file_key) {
    int64_t duration = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return -1;
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_key);
    }

    if (config_file) {
        duration = config_file->get_preload_video_duration();
    }

    return duration;

}

int64_t
DiskCache::get_instance_parameter(const std::string &file_key, const std::string &parameter_key) {
    int64_t value = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return -1;
    }

    if (config_file->parse() < 0) {
        return -1;
    }

    if (config_file) {
        value = config_file->get_instance_parameter(parameter_key);
    }

    return value;
}

int64_t DiskCache::try_get_instance_parameter(const std::string &file_key,
                                              const std::string &parameter_key) {
    int64_t value = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
        auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
        if (lock) {
            config_file = build_config_file_with_file_key(file_key);
        }
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->try_is_config_file_exist();
    if (exist < 0) {
        return -1;
    }

    if (config_file->try_parse() < 0) {
        return -1;
    }

    if (config_file) {
        value = config_file->try_get_instance_parameter(parameter_key);
    }
    return value;
}


int DiskCache::try_get_instance_parameter_with_map(const std::string &file_key, Int64Map &map) {
    int result = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
        auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
        if (lock) {
            config_file = build_config_file_with_file_key(file_key);
        }
    }

    if (!config_file) {
        return -1;
    }

    int exist = config_file->try_is_config_file_exist();
    if (exist) {
        return -1;
    }

    if (config_file->try_parse() < 0) {
        return -1;
    }

    if (config_file) {
        result = config_file->try_get_instance_parameter_with_map(map);
    }

    return result;
}

int64_t DiskCache::clear_cache() {
    proxy::TimedLock lock(read_write_lock_);
    file_removed_list_.clear();
    total_file_list_.clear();
    files_map_.clear();
    int foders_count = 0;
    foders_count = get_all_cache_size();

    int64_t current_cache_size = 0;
    current_cache_size = check_if_remove_cache(all_cached_size_);

    for (auto &file_key: file_removed_list_) {
        //TODO 日志
    }
    remove_file_notification_with_file_keys(file_removed_list_);

    return current_cache_size;
}


void DiskCache::clear_all_cache() {
    proxy::TimedLock lock(read_write_lock_);
    file_removed_list_.clear();
    files_map_.clear();
    all_cached_size_ = 0;
    int foders_count = 0;
    foders_count = DiskCacheUtil::get_all_cached_file_key_and_total_size(cache_path_string_,
                                                                         all_cached_size_,
                                                                         file_removed_list_);
    remove_all_cache();

    for (auto &file_key: file_removed_list_) {
        //TODO 继续执行
    }

    remove_file_notification_with_file_keys(file_removed_list_);

}

int64_t DiskCache::clear_cache_with_key(const std::string &file_key) {
    int64_t result = -1;
    if (file_key.empty()) {
        return -1;
    }

    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
        files_map_.erase(file_key);
    }

    if (!config_file) {
        return -1;
    }

    result = config_file->remove();

    return result;
}


bool DiskCache::is_cache_complete(const std::string &file_key) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }
    if (!config_file) {
        return false;
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return false;
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_key);
        return false;
    }

    return config_file->is_cache_complete();

}

std::string DiskCache::cale_file_sign(const std::string &file_key) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return std::string("");
    }

    int exist = config_file->is_config_file_exist();
    if (exist < 0) {
        return std::string("");
    }

    int result = config_file->parse();
    if (result < 0) {
        proxy::TimedLock lock(read_write_lock_);
        files_map_.erase(file_key);
        return std::string("");
    }

    return config_file->calc_file_sign();
}

int64_t DiskCache::get_cache_info(std::shared_ptr<CacheInfo> &info) {
    Int64Map parameter_map;
    parameter_map[CachedSizeKey] = -1;
    parameter_map[FileSizeKey] = -1;
    parameter_map[PreloadSizeKey] = -1;
    parameter_map[PreloadAudioDurationKey] = -1;
    parameter_map[PreloadVideoDurationKey] = -1;

    try_get_instance_parameter_with_map(info->file_key_, parameter_map);

    int64_t preload_size = parameter_map[PreloadSizeKey];
    int64_t cache_size = parameter_map[CachedSizeKey];
    info->file_size_ = parameter_map[FileSizeKey];
    if (info->file_size_ > 0 && cache_size > 0 && cache_size >= info->file_size_) {
        info->is_complete_ = true;
    }
    info->min_preload_duration_ =
            parameter_map[PreloadAudioDurationKey] <= parameter_map[PreloadVideoDurationKey] ?
            parameter_map[PreloadAudioDurationKey] : parameter_map[PreloadVideoDurationKey];

    if (preload_size > 0) {
        uint64_t read_count =
                preload_size > CacheInfo::CacheSize ? CacheInfo::CacheSize : preload_size;
        info->cache_data_.resize(read_count);
        int64_t real_read = read_data(info->file_key_.c_str(), info->cache_data_.data(),
                                      info->offset_, read_count);

        if (real_read > 0) {
            info->cache_data_count_ = real_read;
        }
    }
    //TODO 日志打印

    return info->cache_data_count_;
}

int64_t DiskCache::check_if_remove_cache(int64_t all_cached_size) {
    int64_t current_cached_size = all_cached_size;
    if (DiskCacheUtil::is_need_clean(all_cached_size)) {
        DiskCacheUtil::remove_expire_cache(cache_path_string_, total_file_list_, file_removed_list_,
                                           current_cached_size,
                                           true);
    }

    if (DiskCacheUtil::is_need_clean(current_cached_size)) {
        DiskCacheUtil::remove_expire_cache(cache_path_string_, total_file_list_, file_removed_list_,
                                           current_cached_size,
                                           false);
    }

    return current_cached_size;
}

bool DiskCache::is_file_key_exist_in_map(const std::string &file_key) {
    if (files_map_.exists(file_key)) {
        return true;
    } else {
        return false;
    }
}

int DiskCache::get_all_cache_size() {
    char *cache_path = const_cast<char *>(cache_path_string_.c_str());
    DIR *dir = opendir(cache_path);
    struct dirent *in_file;
    int files_count = 0;
    all_cached_size_ = 0;
    if (dir) {
        while ((in_file = readdir(dir))) {
            if (in_file->d_type == DT_DIR && strcmp(in_file->d_name, ".") && strcmp(in_file->d_name, "..")) {

                CacheFileInfo cache_file_info;
                int result = DiskCacheUtil::get_cache_file_info_form_dir(cache_path_string_, in_file->d_name, cache_file_info);
                if (result < 0) {
                    continue;
                }
                std::string file_key = cache_file_info.file_key_;
                if (file_key.empty()) {
                    continue;
                }

                total_file_list_.push_back(cache_file_info);
                int64_t cached_size = static_cast<int64_t>(cache_file_info.cache_size_);
                if (cached_size >= 0) {
                    all_cached_size_ += cached_size;

                    if (files_count < 20) {
                        //TODO
                    }
                }

                files_count++;
            }
        }
        closedir(dir);
    }
    return files_count;
}


void DiskCache::remove_all_cache() {
    DiskCacheUtil::remove_all_file_in_absolute_path(cache_path_string_);
}

void DiskCache::set_cache_file_changed_listener(std::weak_ptr<ICacheFileChangeListener> listener) {
    proxy::TimedLock lock(read_write_lock_);
    cache_file_change_listener_ = listener;
}

int DiskCache::get_cache_complete_and_limit_size_file_list(std::vector<std::string> &file_keys) {
    proxy::TimedLock lock(read_write_lock_);
    char* cache_path = const_cast<char *>(cache_path_string_.c_str());
    DIR *dir = opendir(cache_path);
    struct dirent *in_file;
    int files_count = 0;

    uint64_t max_file_length = 0;
    int up_file_count = 0;

    std::vector<PairCacheConfigFile> files_vector;
    if (dir) {
        while ((in_file = readdir(dir))) {
            if (in_file->d_type == DT_DIR && strcmp(in_file->d_name, ".") &&
                    strcmp(in_file->d_name, "..")) {

                CacheFileInfo cache_file_info;
                int result = DiskCacheUtil::get_cache_file_info_form_dir(cache_path_string_, in_file->d_name, cache_file_info);
                if (result < 0) {
                    continue;
                }
                std::string file_key = cache_file_info.file_key_;
                if (file_key.empty()) {
                    continue;
                }

                std::shared_ptr<FileCacheInterface> config_file = build_config_file_with_file_key(file_key);
                if (!config_file) {
                    continue;
                }

                if (config_file->parse() != 0) {
                    continue;
                }

                StringList invalid_file_list;
                if (!DiskCacheUtil::is_valid_file(config_file, 1, invalid_file_list, false)) {
                    continue;
                } else {
                    files_vector.push_back(std::make_pair(config_file->get_file_key(), config_file));
                }
                files_count++;
            }
        }

        closedir(dir);

        if (files_count != 0) {
            if (!files_vector.empty()) {
                DiskCacheUtil::sort_by_share_num(files_vector, false);
            }

            for (auto &it : files_vector) {
                if (it.second->get_file_size() > max_file_length) {
                    max_file_length = it.second->get_file_size();
                }
                file_keys.push_back(it.first);
                up_file_count++;
            }
        }
    }

    return file_keys.size();
}

int DiskCache::get_cache_file_info_with_key(const std::string &file_key, CacheFileInfo &file_info) {
    if (file_key.empty()) {
        return -1;
    }

    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (!config_file) {
        return -1;
    }

    if (config_file->parse() != 0) {
        return -1;
    }

    file_info.file_key_ = file_key;
    file_info.file_size_ = config_file->get_file_size();
    file_info.cache_size_ = config_file->get_cache_size();
    file_info.access_time_ = config_file->get_access_time();
    file_info.file_sign_ = config_file->calc_file_sign();
    return 0;
}

std::shared_ptr<FileCacheInterface>
DiskCache::build_config_file_with_file_key(const std::string &file_key, bool add_top_map) {
    int status = DiskCacheUtil::is_cache_path_exist(cache_path_string_);
    if (status < 0) {
        return nullptr;
    }

    char md5_key[40];
    util_generate_md5_value((uint8_t *) md5_key, (uint8_t *) file_key.c_str(),
                            (int) strlen(file_key.c_str()));
    status = DiskCacheUtil::is_file_path_exist(cache_path_string_, md5_key);
    if (status < 0) {
        return nullptr;
    }

    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    if (is_file_key_exist_in_map(file_key)) {
        config_file = files_map_.get(file_key);
    }
    if (!config_file) {
        config_file = create_file_disk_cache(cache_path_string_, file_key);

        if (add_top_map) {
            files_map_.put(file_key, config_file);
        }
    }
    return config_file;
}


void DiskCache::remove_file_notification_with_file_keys(const StringList &file_keys) {
    auto listener = cache_file_change_listener_.lock();
    if (listener) {
        listener->cache_removed(file_keys);
    }
}
