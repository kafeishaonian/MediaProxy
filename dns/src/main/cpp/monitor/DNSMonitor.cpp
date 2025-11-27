//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSMonitor.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace dns {
    DNSMonitor::DNSMonitor()
            : enabled_(true),
              total_requests_(0),
              success_full_requests_(0),
              failed_requests_(0),
              cached_requests_(0),
              max_samples_(1000),
              current_cache_size_(0),
              current_cache_capacity_(0),
              current_memory_usage_(0),
              current_active_threads_(0),
              current_queued_tasks_(0) {

    }

    void DNSMonitor::record_resolution(const std::string &hostname, double duration, bool success,
                                       bool from_cache) {
        if (!enabled_) {
            return;
        }
        total_requests_++;

        if (success) {
            success_full_requests_++;
        } else {
            failed_requests_++;
        }

        if (from_cache) {
            cached_requests_++;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        add_sample(resolution_times_, duration);
        Logger::log(LogLevel::DEBUG, "Monitor",
                    "分辨率: " + hostname + " " + std::to_string(duration) + "ms " +
                    (success ? "success" : "failed") + (from_cache ? " (cached)" : ""));
    }

    void DNSMonitor::record_speed_check(const std::string &ip, double duration) {
        if (!enabled_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        add_sample(speed_check_times_, duration);

        Logger::log(LogLevel::DEBUG, "Monitor",
                    "速度检查: " + ip + " " + std::to_string(duration) + "ms");
    }

    void DNSMonitor::record_error(const std::string &error_type) {
        if (!enabled_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        error_counts_[error_type]++;

        Logger::log(LogLevel::DEBUG, "Monitor", "error: " + error_type);
    }

    void DNSMonitor::update_cache_stats(size_t size, size_t capacity) {
        if (!enabled_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        current_cache_size_ = size;
        current_cache_capacity_ = capacity;
    }

    void
    DNSMonitor::update_resource_state(size_t memory_usage, int active_threads, int queued_tasks) {
        if (!enabled_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        current_memory_usage_ = memory_usage;
        current_active_threads_ = active_threads;
        current_queued_tasks_ = queued_tasks;
    }

    double
    DNSMonitor::calculate_percentile(const std::vector<double> &data, double percentile) const {
        if (data.empty()) {
            return 0.0;
        }

        std::vector<double> sorted = data;
        std::sort(sorted.begin(), sorted.end());

        size_t index = static_cast<size_t>((percentile / 100.0) * sorted.size());
        if (index >= sorted.size()) {
            index = sorted.size() - 1;
        }

        return sorted[index];
    }

    double DNSMonitor::calculate_average(const std::vector<double> &data) const {
        if (data.empty()) {
            return 0.0;
        }

        double sum = 0.0;
        for (double value: data) {
            sum += value;
        }
        return sum / data.size();
    }

    void DNSMonitor::add_sample(std::vector<double> &samples, double value) {
        samples.push_back(value);

        if (samples.size() > max_samples_) {
            samples.erase(samples.begin());
        }
    }

    DNSMonitor::Metrics DNSMonitor::get_metrics() const {
        std::lock_guard<std::mutex> lock(mutex_);

        Metrics metrics;
        metrics.total_requests = total_requests_.load();
        metrics.success_full_requests = success_full_requests_.load();
        metrics.failed_requests = failed_requests_.load();
        metrics.cached_requests = cached_requests_.load();

        metrics.avg_resolution_time = calculate_average(resolution_times_);
        metrics.avg_speed_check_time = calculate_average(speed_check_times_);
        metrics.p50_resolution_time = calculate_percentile(resolution_times_, 50.0);
        metrics.p95_resolution_time = calculate_percentile(resolution_times_, 95.0);
        metrics.p99_resolution_time = calculate_percentile(resolution_times_, 99.0);

        if (metrics.total_requests > 0) {
            metrics.cache_hit_rate = (metrics.cached_requests * 100.0) / metrics.total_requests;
        }
        metrics.cache_size = current_cache_size_;
        metrics.cache_capacity = current_cache_capacity_;

        metrics.memory_usage = current_memory_usage_;
        metrics.active_threads = current_active_threads_;
        metrics.queued_tasks = current_queued_tasks_;

        metrics.error_counts = error_counts_;

        size_t recent_count = std::min(resolution_times_.size(), size_t(100));
        if (recent_count > 0) {
            metrics.recent_resolution_times.assign(
                    resolution_times_.end() - recent_count,
                    resolution_times_.end()
            );
        }

        return metrics;
    }

    void DNSMonitor::reset() {
        std::lock_guard<std::mutex> lock(mutex_);

        total_requests_ = 0;
        success_full_requests_ = 0;
        failed_requests_ = 0;
        cached_requests_ = 0;

        resolution_times_.clear();
        speed_check_times_.clear();
        error_counts_.clear();

        current_cache_size_ = 0;
        current_cache_capacity_ = 0;
        current_memory_usage_ = 0;
        current_active_threads_ = 0;
        current_queued_tasks_ = 0;

        Logger::log(LogLevel::INFO, "Monitor", "数据重制");
    }

    std::string DNSMonitor::generate_report() const {
        auto metrics = get_metrics();

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        oss << "=== MMDNS 性能监控报告 ===\n\n";

        oss << "DNS解析统计:\n";
        oss << "  总请求数: " << metrics.total_requests << "\n";
        oss << "  成功数: " << metrics.success_full_requests << "\n";
        oss << "  失败数: " << metrics.failed_requests << "\n";
        oss << "  缓存命中: " << metrics.cached_requests << "\n";
        if (metrics.total_requests > 0) {
            oss << "  成功率: " << (metrics.success_full_requests * 100.0 / metrics.total_requests)
                << "%\n";
        }
        oss << "\n";

        oss << "性能指标:\n";
        oss << "  平均解析时间: " << metrics.avg_resolution_time << "ms\n";
        oss << "  平均测速时间: " << metrics.avg_speed_check_time << "ms\n";
        oss << "  P50解析时间: " << metrics.p50_resolution_time << "ms\n";
        oss << "  P95解析时间: " << metrics.p95_resolution_time << "ms\n";
        oss << "  P99解析时间: " << metrics.p99_resolution_time << "ms\n";
        oss << "\n";


        // 缓存统计
        oss << "缓存统计:\n";
        oss << "  缓存命中率: " << metrics.cache_hit_rate << "%\n";
        oss << "  当前缓存大小: " << metrics.cache_size << "/" << metrics.cache_capacity << "\n";
        oss << "\n";

        // 资源使用
        oss << "资源使用:\n";
        oss << "  内存使用: " << (metrics.memory_usage / 1024.0 / 1024.0) << "MB\n";
        oss << "  活跃线程: " << metrics.active_threads << "\n";
        oss << "  队列任务: " << metrics.queued_tasks << "\n";
        oss << "\n";

        // 错误统计
        if (!metrics.error_counts.empty()) {
            oss << "错误统计:\n";
            for (const auto &pair: metrics.error_counts) {
                oss << "  " << pair.first << ": " << pair.second << "\n";
            }
            oss << "\n";
        }

        oss << "===========================\n";

        return oss.str();
    }

    void DNSMonitor::set_enabled(bool enabled) {
        enabled_ = enabled;
    }

    bool DNSMonitor::is_enabled() const {
        return enabled_;
    }
}