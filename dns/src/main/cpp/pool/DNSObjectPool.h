//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSOBJECTPOOL_H
#define MEDIAPROXY_DNSOBJECTPOOL_H


#include <vector>
#include <memory>
#include <mutex>
#include <functional>

#include "DNSCommon.h"

namespace dns {

    template<typename T>
    class ObjectPool {
    public:
        explicit ObjectPool(
                size_t initial_size = 10,
                size_t max_size = 100,
                std::function<std::shared_ptr<T>()> factory = nullptr,
                std::function<void(std::shared_ptr<T> &)> resetter = nullptr
        ) : max_size_(max_size),
            factory_(factory),
            resetter_(resetter),
            total_created_(0) {

            for (size_t i = 0; i < initial_size && i < max_size; ++i) {
                pool_.push_back(create_object());
                total_created_++;
            }

            Logger::log(LogLevel::DEBUG, "ObjectPool",
                        "初始化对象：=" + std::to_string(initial_size));
        }

        ~ObjectPool() {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.clear();
            Logger::log(LogLevel::DEBUG, "ObjectPool", "destroy");
        }

        std::shared_ptr<T> acquire() {
            std::lock_guard<std::mutex> lock(mutex_);

            if (!pool_.empty()) {
                auto obj = pool_.back();
                pool_.pop_back();
                return obj;
            }

            if (total_created_ < max_size_) {
                total_created_++;
                return create_object();
            }

            Logger::log(LogLevel::WARN, "ObjectPool",
                        "对象池没有对象数据了: " + std::to_string(max_size_));

            return nullptr;
        }


        void release(std::shared_ptr<T> obj) {
            if (!obj) {
                return;
            }

            std::lock_guard<std::mutex> lock(mutex_);
            if (resetter_) {
                resetter_(obj);
            }

            if (pool_.size() < max_size_) {
                pool_.push_back(obj);
            } else {
                total_created_--;
            }
        }

        size_t availableCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return pool_.size();
        }

        size_t totalCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return total_created_;
        }


        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            pool_.clear();
            total_created_ = 0;
            Logger::log(LogLevel::DEBUG, "ObjectPool", "Cleared");
        }

    private:
        std::shared_ptr<T> create_object() {
            if (factory_) {
                return factory_();
            }
            return std::make_shared<T>();
        }

    private:
        std::vector<std::shared_ptr<T>> pool_;
        mutable std::mutex mutex_;
        size_t max_size_;
        size_t total_created_;
        std::function<std::shared_ptr<T>()> factory_;
        std::function<void(std::shared_ptr<T> &)> resetter_;
    };

    class MMDNSHostModel;

    class MMDNSIPModel;

    class MMDNSSocket;

    using HostModelPool = ObjectPool<MMDNSHostModel>;
    using IPModelPool = ObjectPool<MMDNSIPModel>;
    using SocketPool = ObjectPool<MMDNSSocket>;

}

#endif //MEDIAPROXY_DNSOBJECTPOOL_H