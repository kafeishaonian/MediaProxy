# ThreadUtil.h 现代化优化方案

## 一、当前代码分析

### 1.1 现状
```cpp
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>

typedef boost::shared_lock<boost::shared_mutex> read_lock;
typedef boost::unique_lock<boost::shared_mutex> write_lock;
typedef boost::unique_lock<boost::timed_mutex> timed_lock;
typedef std::unique_lock<std::mutex> std_write_lock;

template<typename T, typename D>
boost::unique_lock<T> make_unique_lock(T& mutex, D d) {
    return boost::unique_lock<T>(mutex, d);
}
```

### 1.2 存在的问题
❌ **依赖 Boost 线程库** - C++17 标准库已提供相同功能  
❌ **使用旧式 typedef** - C++11+ 推荐使用 `using`  
❌ **混用 Boost 和 std** - 代码风格不统一  
❌ **缺少现代 RAII 封装** - 可以更安全  
❌ **缺少文档注释** - 不够清晰  

---

## 二、现代 C++ 特性对照表

| 功能 | Boost (旧) | C++17 标准库 (新) | 优势 |
|------|-----------|------------------|------|
| 读写锁 | `boost::shared_mutex` | `std::shared_mutex` | 标准库，无需依赖 |
| 共享锁 | `boost::shared_lock` | `std::shared_lock` | 标准库支持 |
| 独占锁 | `boost::unique_lock` | `std::unique_lock` | 已在使用 |
| 作用域锁 | N/A | `std::scoped_lock` | C++17，避免死锁 |
| 类型别名 | `typedef` | `using` | C++11+，更清晰 |

---

## 三、优化方案

### 方案 A：完全标准化（推荐 C++17+）

#### 优化后代码
```cpp
//
// Created by Hongmingwei on 2025/10/24.
// Optimized with C++17 standard library
//

#ifndef MEDIAPROXY_THREADUTIL_H
#define MEDIAPROXY_THREADUTIL_H

#include <mutex>
#include <shared_mutex>  // C++17
#include <chrono>

namespace proxy {

/**
 * @brief 读写锁类型定义（使用 C++17 标准库）
 * 
 * 使用示例：
 *   std::shared_mutex rw_mutex;
 *   {
 *       read_lock lock(rw_mutex);   // 共享读锁
 *       // 读操作...
 *   }
 *   {
 *       write_lock lock(rw_mutex);  // 独占写锁
 *       // 写操作...
 *   }
 */

// C++11 using 别名声明（推荐）
using read_lock = std::shared_lock<std::shared_mutex>;
using write_lock = std::unique_lock<std::shared_mutex>;
using timed_lock = std::unique_lock<std::timed_mutex>;
using std_write_lock = std::unique_lock<std::mutex>;

// C++17 scoped_lock - 多锁自动加锁，避免死锁
template<typename... MutexTypes>
using scoped_lock = std::scoped_lock<MutexTypes...>;

/**
 * @brief RAII 读锁守卫
 * @tparam Mutex 互斥量类型
 * 
 * 用法：
 *   ReadGuard guard(my_shared_mutex);
 *   // 自动获取共享锁，离开作用域自动释放
 */
template<typename Mutex = std::shared_mutex>
class ReadGuard {
public:
    explicit ReadGuard(Mutex& mutex) : lock_(mutex) {}
    
    // 禁止拷贝
    ReadGuard(const ReadGuard&) = delete;
    ReadGuard& operator=(const ReadGuard&) = delete;
    
    // 允许移动（C++11）
    ReadGuard(ReadGuard&&) noexcept = default;
    ReadGuard& operator=(ReadGuard&&) noexcept = default;
    
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
    explicit WriteGuard(Mutex& mutex) : lock_(mutex) {}
    
    WriteGuard(const WriteGuard&) = delete;
    WriteGuard& operator=(const WriteGuard&) = delete;
    
    WriteGuard(WriteGuard&&) noexcept = default;
    WriteGuard& operator=(WriteGuard&&) noexcept = default;
    
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
    bool try_lock_for(const std::chrono::duration<Rep, Period>& timeout) {
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
[[nodiscard]] auto make_read_lock(Mutex& mutex) {
    return std::shared_lock<Mutex>(mutex);
}

template<typename Mutex>
[[nodiscard]] auto make_write_lock(Mutex& mutex) {
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
[[nodiscard]] auto make_unique_lock(Mutex& mutex, DeferTag defer_tag) {
    return std::unique_lock<Mutex>(mutex, defer_tag);
}

} // namespace proxy

#endif // MEDIAPROXY_THREADUTIL_H
```

---

### 方案 B：保持 Boost 兼容（过渡方案）

如果项目仍需支持 C++14 或保持 Boost 依赖：

```cpp
#ifndef MEDIAPROXY_THREADUTIL_H
#define MEDIAPROXY_THREADUTIL_H

#include <mutex>

// 条件编译：优先使用标准库
#if __cplusplus >= 201703L  // C++17+
    #include <shared_mutex>
    namespace proxy {
        using read_lock = std::shared_lock<std::shared_mutex>;
        using write_lock = std::unique_lock<std::shared_mutex>;
    }
#else  // C++14 及以下
    #include <boost/thread/shared_mutex.hpp>
    namespace proxy {
        using read_lock = boost::shared_lock<boost::shared_mutex>;
        using write_lock = boost::unique_lock<boost::shared_mutex>;
    }
#endif

namespace proxy {
    using timed_lock = std::unique_lock<std::timed_mutex>;
    using std_write_lock = std::unique_lock<std::mutex>;
}

#endif // MEDIAPROXY_THREADUTIL_H
```

---

## 四、C++17/20 高级特性

### 4.1 std::scoped_lock（C++17）

**优势**：同时锁定多个互斥量，自动避免死锁

```cpp
// ❌ 旧方式 - 可能死锁
std::unique_lock<std::mutex> lock1(mutex1);
std::unique_lock<std::mutex> lock2(mutex2);

// ✅ C++17 新方式 - 避免死锁
std::scoped_lock lock(mutex1, mutex2);  // 原子性地锁定多个互斥量
```

**实际应用**：
```cpp
class Cache {
    std::shared_mutex read_mutex_;
    std::mutex write_mutex_;
    
public:
    void sync_operation() {
        // 同时锁定读写互斥量，避免死锁
        std::scoped_lock lock(read_mutex_, write_mutex_);
        // 临界区代码
    }
};
```

---

### 4.2 [[nodiscard]] 属性（C++17）

**用途**：防止忽略重要返回值

```cpp
// 强制检查返回值
[[nodiscard]] bool try_lock_with_timeout(std::mutex& m, int timeout_ms) {
    return m.try_lock_for(std::chrono::milliseconds(timeout_ms));
}

// ❌ 编译器警告：返回值未使用
try_lock_with_timeout(my_mutex, 100);

// ✅ 正确使用
if (try_lock_with_timeout(my_mutex, 100)) {
    // 成功获取锁
}
```

---

### 4.3 CTAD 类模板参数推导（C++17）

```cpp
// C++17 前需要显式指定模板参数
std::unique_lock<std::mutex> lock1(mutex);

// ✅ C++17 自动推导
std::unique_lock lock2(mutex);  // 自动推导为 std::unique_lock<std::mutex>
std::shared_lock lock3(shared_mutex);  // 推导为 std::shared_lock<std::shared_mutex>
```

---

### 4.4 if constexpr（C++17）

**用于编译期优化**：

```cpp
template<typename Mutex>
class SmartLock {
public:
    explicit SmartLock(Mutex& m) : mutex_(m) {
        if constexpr (std::is_same_v<Mutex, std::shared_mutex>) {
            // 共享锁逻辑
            lock_ = std::shared_lock(mutex_);
        } else {
            // 独占锁逻辑
            lock_ = std::unique_lock(mutex_);
        }
    }
    
private:
    Mutex& mutex_;
    std::variant<std::shared_lock<Mutex>, std::unique_lock<Mutex>> lock_;
};
```

---

### 4.5 std::jthread（C++20）

**自动 join 的线程**：

```cpp
// ❌ C++11 std::thread 需要手动 join
{
    std::thread t([]{ /* work */ });
    t.join();  // 必须显式调用
}

// ✅ C++20 std::jthread 自动 join
{
    std::jthread t([]{ /* work */ });
    // 析构时自动 join，支持中断
}
```

---

## 五、最佳实践示例

### 5.1 读写锁使用模式

```cpp
#include "ThreadUtil.h"

class DataCache {
private:
    std::shared_mutex rw_mutex_;
    std::map<std::string, std::string> cache_;

public:
    // 读操作 - 共享锁
    std::string get(const std::string& key) {
        proxy::read_lock lock(rw_mutex_);  // RAII 自动释放
        auto it = cache_.find(key);
        return it != cache_.end() ? it->second : "";
    }
    
    // 写操作 - 独占锁
    void set(const std::string& key, const std::string& value) {
        proxy::write_lock lock(rw_mutex_);
        cache_[key] = value;
    }
    
    // C++17 scoped_lock 示例
    void batch_update(const std::map<std::string, std::string>& updates) {
        std::scoped_lock lock(rw_mutex_);  // 自动推导类型
        cache_.insert(updates.begin(), updates.end());
    }
};
```

---

### 5.2 RAII 守卫使用

```cpp
#include "ThreadUtil.h"

class ThreadSafeCounter {
private:
    std::shared_mutex mutex_;
    int count_ = 0;

public:
    int increment() {
        proxy::WriteGuard guard(mutex_);  // 自动加锁/解锁
        return ++count_;
    }
    
    int get() const {
        proxy::ReadGuard guard(mutex_);
        return count_;
    }
    
    // 带超时的操作
    std::optional<int> try_increment(std::chrono::milliseconds timeout) {
        proxy::WriteGuard guard(mutex_);
        if (guard.try_lock_for(timeout)) {
            return ++count_;
        }
        return std::nullopt;
    }
};
```

---

### 5.3 延迟锁定模式

```cpp
void delayed_lock_example() {
    std::mutex mutex1, mutex2;
    
    // 创建锁但不立即锁定
    auto lock1 = proxy::make_unique_lock(mutex1, std::defer_lock);
    auto lock2 = proxy::make_unique_lock(mutex2, std::defer_lock);
    
    // 同时锁定，避免死锁
    std::lock(lock1, lock2);
    
    // 临界区代码
}
```

---

## 六、性能对比

| 特性 | Boost | C++17 标准库 | 性能差异 |
|------|-------|-------------|---------|
| shared_mutex | ✓ | ✓ | 相近 |
| 编译时间 | 较慢（需解析 Boost） | 快 | **标准库更快** |
| 二进制大小 | 较大 | 小 | **标准库更小** |
| 跨平台兼容性 | 优秀 | 优秀 | 相同 |
| 维护成本 | 需要 Boost 依赖 | 无依赖 | **标准库更低** |

---

## 七、迁移建议

### 7.1 渐进式迁移步骤

1. ✅ **第一步**：使用 `using` 替代 `typedef`
2. ✅ **第二步**：添加 RAII 封装类（ReadGuard/WriteGuard）
3. ✅ **第三步**：替换 Boost 为 std::shared_mutex（需 C++17）
4. ✅ **第四步**：使用 [[nodiscard]] 标记返回值
5. ✅ **第五步**：引入 std::scoped_lock

### 7.2 兼容性检查

```cpp
// 检查 C++ 版本
#if __cplusplus < 201703L
    #error "This project requires C++17 or later"
#endif
```

---

## 八、CMakeLists.txt 配置

确保启用 C++17：

```cmake
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 或者
target_compile_features(your_target PRIVATE cxx_std_17)
```

---

## 九、总结与建议

### ✅ 推荐改进

1. **立即可做**（C++11+）：
   - ✅ `typedef` → `using`
   - ✅ 添加移动构造/赋值
   - ✅ 添加注释文档

2. **C++17 升级**（推荐）：
   - ✅ `boost::shared_mutex` → `std::shared_mutex`
   - ✅ 使用 `std::scoped_lock`
   - ✅ 使用 CTAD 简化代码
   - ✅ 添加 `[[nodiscard]]`

3. **C++20 特性**（未来）：
   - 🔄 考虑 `std::jthread`
   - 🔄 使用 Concepts 约束模板

### ⚠️ 注意事项

- **Android NDK**：确认 NDK 版本支持 C++17（r19+）
- **编译器**：Clang 5+, GCC 7+
- **性能**：标准库与 Boost 性能相近，但编译更快
- **兼容性**：如需支持旧系统，保留 Boost 过渡方案

---

**优化优先级**：中等  
**风险等级**：低（类型别名修改）  
**建议时机**：下次重构或版本升级