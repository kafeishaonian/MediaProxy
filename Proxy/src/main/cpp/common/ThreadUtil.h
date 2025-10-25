//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_THREADUTIL_H
#define MEDIAPROXY_THREADUTIL_H

#include <cstdio>
#include <mutex>
#include <shared_mutex>

namespace proxy {

    /**
     * 读写锁定义
     */
    using read_lock = std::shared_lock<std::shared_mutex>;
    using write_lock = std::unique_lock<std::shared_mutex>;
    using timed_lock = std::unique_lock<std::timed_mutex>;
    using std_write_lock = std::unique_lock<std::mutex>;

    // C++17 多锁自动加锁，避免死锁
    template<typename... MutexTypes>
    using scoped_lock = std::scoped_lock<MutexTypes...>;

    /**
     * @brief RAII 读锁守卫
     * @tparam Mutex 互斥量类型
     *
     * 用法：
     *   ReadGuard guard(my_shared_mutex);
     *   自动获取共享锁，离开作用域自动释放
     */
    template<typename Mutex = std::shared_mutex>
    class ReadGuard {
    public:
        explicit ReadGuard(Mutex &mutex) : lock_(mutex) {}

        // 禁止拷贝
        ReadGuard(const ReadGuard &) = delete;

        ReadGuard &operator=(const ReadGuard &) = delete;

        // 允许移动（C++11）
        ReadGuard(ReadGuard &&) noexcept = default;

        ReadGuard &operator=(ReadGuard &&) noexcept = default;

    private:
        std::shared_lock<Mutex> lock_;
    };


    /**
     * @brief RAII 写锁守卫
     * @tparam Mutex 互斥量类型
     */
    template<typename Mutex = std::shared_mutex>
    class WriteGuard {
    public:
        explicit WriteGuard(Mutex &mutex) : lock_(mutex) {}

        WriteGuard(const WriteGuard &) = delete;

        WriteGuard &operator=(const WriteGuard &) = delete;

        WriteGuard(WriteGuard &&) noexcept = default;

        WriteGuard &operator=(WriteGuard &&) noexcept = default;

        /**
         * @brief 尝试加锁（非阻塞）
         * @return 是否成功获取锁
         */
        bool try_lock() {
            return lock_.try_lock();
        }

        /**
         * @brief 带超时的加锁
         * @param timeout 超时时间
         * @return 是否在超时前获取锁
         */
        template<typename Rep, typename Period>
        bool try_lock_for(const std::chrono::duration<Rep, Period> &timeout) {
            return lock_.try_lock_for(timeout);
        }

    private:
        std::unique_lock<Mutex> lock_;
    };

    /**
     * @brief C++17 类模板参数推导辅助函数（可选）
     *
     * 用法：
     *   auto lock = make_lock(my_mutex);
     */
    template<typename Mutex>
    [[nodiscard]] auto make_read_lock(Mutex &mutex) {
        return std::shared_lock<Mutex>(mutex);
    }

    template<typename Mutex>
    [[nodiscard]] auto make_write_lock(Mutex &mutex) {
        return std::unique_lock<Mutex>(mutex);
    }

    /**
     * @brief 带延迟参数的锁创建（C++17 推导）
     * @tparam Mutex 互斥量类型
     * @tparam Duration 延迟类型
     * @param mutex 互斥量引用
     * @param defer_tag 延迟标签（std::defer_lock 等）
     */
    template<typename Mutex, typename DeferTag>
    [[nodiscard]] auto make_unique_lock(Mutex &mutex, DeferTag defer_tag) {
        return std::unique_lock<Mutex>(mutex, defer_tag);
    }

}

#endif //MEDIAPROXY_THREADUTIL_H

