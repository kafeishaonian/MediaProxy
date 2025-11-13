//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSBLOCKINGQUEUE_H
#define MEDIAPROXY_DNSBLOCKINGQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>

#include "DNSCommon.h"

namespace dns {
    template<typename T>
    class DNSBlockingQueue {
    public:
        explicit DNSBlockingQueue(size_t max_size = Constants::DEFAULT_QUEUE_SIZE)
                : max_size_(max_size) {

        }

        ~DNSBlockingQueue() {
            clear();
        }

        void put(const T& item) {
            std::unique_lock<std::mutex> lock(mutex_);
            not_full_.wait(lock, [this] {return queue_.size() < max_size_;});
            queue_.push_back(item);
            not_full_.notify_one();
        }

        T take() {
            std::unique_lock<std::mutex> lock(mutex_);
            not_empty_.wait(lock, [this] {return !queue_.empty();});
            T item = queue_.front();
            queue_.pop_front();
            not_full_.notify_one();
            return item;
        }

        std::optional<T> task(std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (!not_empty_.wait_for(lock, timeout, [this] {return !queue_.empty();})) {
                return std::nullopt;
            }
            T item = queue_.front();
            queue_.pop_front();
            not_full_.notify_one();
            return item;
        }

        // 非阻塞放入
        bool try_put(const T& item) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.size() > max_size_) {
                return false;
            }
            queue_.push_back(item);
            not_empty_.notify_one();
            return true;
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }

        bool is_empty() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.clear();
            not_full_.notify_all();
        }

    private:
        std::deque<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable not_empty_;
        std::condition_variable not_full_;
        size_t max_size_;
    };
}


#endif //MEDIAPROXY_DNSBLOCKINGQUEUE_H