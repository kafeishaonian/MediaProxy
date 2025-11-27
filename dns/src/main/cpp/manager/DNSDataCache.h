//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSDATACACHE_H
#define MEDIAPROXY_DNSDATACACHE_H

#include <mutex>

#include "DNSCommon.h"
#include "LRUCache.h"

namespace dns{


    class DNSDataCache {
    public:
        explicit DNSDataCache(size_t cache_size = Constants::DEFAULT_CACHE_SIZE);
        ~DNSDataCache();

        void save(const std::string& key, const std::string& data);
        std::string load(const std::string& key);
        void remove(const std::string& key);
        void clear();

        void save_to_disk(const std::string& filename);
        void load_from_disk(const std::string& filename);

        size_t get_cache_size() const;

        void set_cache_dir(const std::string& dir);
        std::string get_cache_dir() const;

    private:
        std::string get_cache_file_path(const std::string& key) const;
        bool write_to_file(const std::string& file_path, const std::string& data);
        std::string read_from_file(const std::string& file_path);
        std::string escape_json_string(const std::string& str);
        std::string unescape_json_string(const std::string& str) const;

    private:
        std::unique_ptr<LRUCache<std::string, std::string>> memory_cache_;
        std::string cache_dir_;
        mutable std::mutex file_mutex_;
    };

}


#endif //MEDIAPROXY_DNSDATACACHE_H
