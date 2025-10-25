//
// Created by Hongmingwei on 2025/10/25.
//

#ifndef MEDIAPROXY_LRUCACHE_H
#define MEDIAPROXY_LRUCACHE_H

#include <unordered_map>
#include <list>
#include <vector>

namespace proxy {
    template<typename Key, typename Value>
    class LRUCache {
    public:
        using KeyValuePairType = std::pair<Key, Value>;
        using ListIteratorType = std::list<KeyValuePairType>;

        LRUCache(int size) {
            size_ = size;
        }


        bool exists(const Key& key) const {
            return cache_map_.find(key) != cache_map_.end();
        }

        int size() const {
            return cache_map_.size();
        }

        void put(const Key& key, const Value& value) {
            auto it = cache_map_.find(key);
            cache_list_.push_front(key_value_pair_type(key, value));
            if (it != cache_map_.end()) {
                cache_list_.erase(it->second);
                cache_map_.erase(it);
            }
            cache_map_[key] = cache_list_.begin();
            if (cache_map_.size() > size_) {
                auto last = cache_list_.end();
                last--;
                cache_map_.erase(last->first);
                cache_list_.pop_back();
            }
        }

        void erase(const Key& key) {
            auto it = cache_map_.find(key);
            if (it != cache_map_.end()){
                cache_list_.erase(it->second);
                cache_map_.erase(it);
            }
        }

        void clear() {
            cache_map_.clear();
            cache_list_.clear();
        }

        const Value& get(const Key& key) {
            auto it = cache_map_.find(key);
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
            return it->second->second;
        }

        int get_keys(std::vector<Key>& keys) {
            int size = 0;
            for (auto it = cache_list_.begin(); it != cache_list_.end(); it++) {
                keys.push_back(it->first);
                size++;
            }
            return size;
        }

    private:
        int size_;
        std::list<KeyValuePairType> cache_list_;
        std::unordered_map<Key, ListIteratorType> cache_map_;

    };
}


#endif //MEDIAPROXY_LRUCACHE_H
