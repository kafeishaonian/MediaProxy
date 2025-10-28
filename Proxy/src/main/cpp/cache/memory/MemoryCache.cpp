//
// Created by Hongmingwei on 2025/10/24.
//

#include "MemoryCache.h"
#include "ThreadUtil.h"
#include "Singleton.h"
#include "GlobalConstant.h"


MemoryCache::MemoryCache() {
    memory_usage_ = 0;
}

MemoryCache::~MemoryCache() {

}


MemoryCache *MemoryCache::get_instance() {
    return Singleton<MemoryCache>::get_instance();
}

int64_t MemoryCache::get_file_size(const std::string &file_key) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->get_file_size();
    } else {
        return 0;
    }
}

int64_t MemoryCache::read_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                               uint64_t buffer_size) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_or_create_memory_media_cache(file_path_url);
    }
    if (!media_cache) {
        return 0;
    }
    int64_t read_size = 0;
    read_size = media_cache->read_data(buffer, offset, buffer_size);
    return read_size;
}

int64_t MemoryCache::write_data(const char *file_path_url, uint8_t *buffer, int64_t offset,
                                int64_t buffer_size, int64_t file_size) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_or_create_memory_media_cache(file_path_url);
        memory_usage_ += buffer_size;
    }

    int64_t write_size = 0;
    media_cache->set_file_key(file_path_url);
    media_cache->set_file_size(file_size);
    write_size = media_cache->write_data(buffer, offset, buffer_size, file_size);

    return write_size;
}

void MemoryCache::dump_data(const char *file_path_url) {
    proxy::WriteLock lock(read_write_lock_);
    std::shared_ptr<MemoryMediaCache> media_cache = find_memory_media_cache(file_path_url);
    if (media_cache) {
        media_cache->dump_data();
    }
}


void MemoryCache::dump_all_data() {
    proxy::WriteLock lock(read_write_lock_);
    for (auto it = memory_media_lists_.begin(); it != memory_media_lists_.end(); it++) {
        it->second->dump_data();
    }
}

void MemoryCache::serialize_all() {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    proxy::WriteLock lock(read_write_lock_);
    while (!memory_media_lists_.empty()) {
        auto it = memory_media_lists_.begin();
        if (it->second) {
            int64_t write_size = it->second->serialize_expired_cache();
            if (write_size > 0) {
                memory_usage_ -= write_size;
            }
        }
        memory_media_lists_.erase(it);
    }
}


void MemoryCache::serialize() {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    proxy::WriteLock lock(read_write_lock_);
    if (!memory_media_lists_.empty()) {
        auto it = memory_media_lists_.begin();
        it->second->serialize();
        memory_media_lists_.erase(it);
    }
}

void MemoryCache::serialize_expired_cache() {
    proxy::WriteLock lock(read_write_lock_);
    if (!memory_media_lists_.empty()) {
        auto it = memory_media_lists_.begin();
        if (it->second && it->second->is_cache_expired()) {
            int64_t write_size = it->second->serialize_expired_cache();
            if (write_size > 0) {
                memory_usage_ -= write_size;
            }
            memory_media_lists_.erase(it);
        }
    }
}

std::pair<uint64_t, uint64_t>
MemoryCache::query_remain_data_by_offset(const char *file_key, uint64_t offset) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->query_remain_data_by_offset(offset);
    } else {
        return std::make_pair(0, 0);
    }
}

int MemoryCache::query_empty_segment(const char *file_key,
                                     std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->query_empty_segment(empty_segment_vector);
    } else {
        return -2;
    }
}

int64_t
MemoryCache::get_instance_parameter(const std::string &file_key, const std::string &parameter_key) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->get_instance_parameter(parameter_key);
    } else {
        return -1;
    }
}

void MemoryCache::get_instance_parameter_with_map(const std::string &file_key, Int64Map &map) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        media_cache->get_instance_parameter_with_map(map);
    }
}

void
MemoryCache::set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                    int64_t value) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        media_cache->set_instance_parameter(parameter_key, value);
    }
}

int64_t MemoryCache::calculate_memory_usage() {
    proxy::WriteLock lock(read_write_lock_);
    int64_t memory_usage = 0;
    if (!memory_media_lists_.empty()) {
        auto it = memory_media_lists_.begin();
        if (it->second) {
            memory_usage += it->second->calculate_memory_usage();
        }
    }
    return memory_usage;
}

int64_t MemoryCache::get_memory_usage() {
    proxy::WriteLock lock(read_write_lock_);
    return memory_usage_;
}

void MemoryCache::drop_all() {
    proxy::WriteLock lock(read_write_lock_);
    memory_media_lists_.clear();
}

bool MemoryCache::is_cache_complete(const std::string &file_key) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->is_cache_complete();
    } else {
        return false;
    }
}

bool MemoryCache::query_data_range_exist(const std::string &file_key, int64_t start, int64_t size) {
    std::shared_ptr<MemoryMediaCache> media_cache = nullptr;
    {
        proxy::WriteLock lock(read_write_lock_);
        media_cache = find_memory_media_cache(file_key);
    }
    if (media_cache) {
        return media_cache->query_data_range_exist(start, size);
    } else {
        return false;
    }
}


std::shared_ptr<MemoryMediaCache>
MemoryCache::find_memory_media_cache(const std::string &file_key) {
    if (memory_media_lists_.find(file_key) == memory_media_lists_.end()) {
        return nullptr;
    } else {
        std::shared_ptr<MemoryMediaCache> media_cache = memory_media_lists_[file_key];
        return media_cache;
    }
}


std::shared_ptr<MemoryMediaCache>
MemoryCache::find_or_create_memory_media_cache(const std::string &file_key) {
    if (memory_media_lists_.find(file_key) == memory_media_lists_.end()) {
        std::shared_ptr<MemoryMediaCache> media_cache = std::make_shared<MemoryMediaCache>(file_key);
        memory_media_lists_[file_key] = media_cache;
        return media_cache;
    } else {
        std::shared_ptr<MemoryMediaCache> media_cache = memory_media_lists_[file_key];
        return media_cache;
    }
}

int64_t MemoryCache::get_cache_info(std::shared_ptr<CacheInfo> &info) {
    Int64Map parameter_map;
    parameter_map[CachedSizeKey] = -1;
    parameter_map[FileSizeKey] = -1;
    parameter_map[PreloadSizeKey] = -1;
    parameter_map[PreloadAudioDurationKey] = -1;
    parameter_map[PreloadVideoDurationKey] = -1;

    get_instance_parameter_with_map(info->file_key_, parameter_map);

    int64_t preload_size = parameter_map[PreloadSizeKey];
    int64_t cache_size = parameter_map[CachedSizeKey];
    info->file_size_ = parameter_map[FileSizeKey];

    if (info->file_size_ > 0 && cache_size > 0 && cache_size >= info->file_size_) {
        info->is_complete_ = true;
    }
    info->min_preload_duration_ = parameter_map[PreloadAudioDurationKey] <= parameter_map[PreloadVideoDurationKey] ?
                                  parameter_map[PreloadAudioDurationKey] : parameter_map[PreloadVideoDurationKey];

    if (preload_size > 0) {
        uint64_t read_count = preload_size > CacheInfo::CacheSize ?
                              CacheInfo::CacheSize : preload_size;

        info->cache_data_.resize(read_count);
        int64_t real_read = read_data(info->file_key_.c_str(),
                                      info->cache_data_.data(),
                                      info->offset_, read_count);
        if (real_read > 0) {
            info->cache_data_count_ = real_read;
        }

    }
    return info->cache_data_count_;
}
