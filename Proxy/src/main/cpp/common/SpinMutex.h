//
// Created by Hongmingwei on 2025/10/23.
//

#ifndef MEDIAPROXY_SPINMUTEX_H
#define MEDIAPROXY_SPINMUTEX_H

#include <atomic>
#include <mutex>

class SpinMutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    SpinMutex() = default;

    SpinMutex(const SpinMutex &) = delete;

    SpinMutex &operator=(const SpinMutex &) = delete;

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

typedef std::lock_guard<SpinMutex> spin_auto_lock;

#endif //MEDIAPROXY_SPINMUTEX_H
