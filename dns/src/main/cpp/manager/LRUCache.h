//
// Created by Hongmingwei on 2025/11/17.
//

#ifndef MEDIAPROXY_LRUCACHE_H
#define MEDIAPROXY_LRUCACHE_H

#include <mutex>
#include <list>

#include "DNSCommon.h"

namespace dns {

    template<typename K, typename V>
    class LRUCache {
    public:
        explicit LRUCache(size_t capacity) : capacity_(capacity) {}

        std::optional<V> get(const K &key) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(key);
            if (it == cache_.end()) {
                return std::nullopt;
            }

            items_.splice(items_.begin(), items_, it->second);
            return it->second->second;
        }

        void put(const K &key, const V &value) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                it->second->second = value;
                items_.splice(items_.begin(), items_, it->second);
                return;
            }

            if (cache_.size() >= capacity_) {
                auto last = items_.back();
                cache_.erase(last.first);
                items_.pop_back();
            }
            items_.push_front({key, value});
            cache_[key] = items_.begin();
        }

        void remove(const K &key) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = cache_.find(key);
            if (it != cache_.end()) {
                items_.erase(it->second);
                cache_.erase(it);
            }
        }

        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_.clear();
            items_.clear();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return cache_.size();
        }

        template<class Func>
        void for_each(Func callback) const {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto &item: items_) {
                callback(item.first, item.second);
            }
        }


    private:
        using ListItem = std::pair<K, V>;
        using ListIterator = typename std::list<ListItem>::iterator;

        size_t capacity_;
        std::list<ListItem> items_;
        std::unordered_map<K, ListIterator> cache_;
        mutable std::mutex mutex_;
    };
}


#endif //MEDIAPROXY_LRUCACHE_H
