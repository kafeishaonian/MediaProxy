//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSMONITOR_H
#define MEDIAPROXY_DNSMONITOR_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <chrono>

#include "DNSCommon.h"


namespace dns {

    class DNSMonitor {

    public:
        struct Metrics {
            uint64_t total_requests;
            uint64_t success_full_requests;
            uint64_t failed_requests;
            uint64_t cached_requests;

            double avg_resolution_time;
            double avg_speed_check_time;
            double p50_resolution_time;
            double p95_resolution_time;
            double p99_resolution_time;

            double cache_hit_rate;
            size_t cache_size;
            size_t cache_capacity;

            size_t memory_usage;
            int active_threads;
            int queued_tasks;

            std::map<std::string, uint64_t> error_counts;

            std::vector<double> recent_resolution_times;

            Metrics() : total_requests(0),
                        success_full_requests(0),
                        failed_requests(0),
                        cached_requests(0),
                        avg_resolution_time(0),
                        avg_speed_check_time(0),
                        p50_resolution_time(0),
                        p95_resolution_time(0),
                        p99_resolution_time(0),
                        cache_hit_rate(0),
                        cache_size(0),
                        cache_capacity(0),
                        memory_usage(0),
                        active_threads(0),
                        queued_tasks(0) {}
        };

        DNSMonitor();

        ~DNSMonitor() = default;

        void record_resolution(const std::string &hostname, double duration, bool success,
                               bool from_cache = false);

        void record_speed_check(const std::string &ip, double duration);

        void record_error(const std::string &error_type);

        void update_cache_stats(size_t size, size_t capacity);

        void update_resource_state(size_t memory_usage, int active_threads, int queued_tasks);

        Metrics get_metrics() const;

        void reset();

        std::string generate_report() const;

        void set_enabled(bool enabled);

        bool is_enabled() const;

    private:
        double calculate_percentile(const std::vector<double> &data, double percentile) const;

        double calculate_average(const std::vector<double>& data) const;

        void add_sample(std::vector<double>& samples, double value);


    private:
        mutable std::mutex mutex_;
        std::atomic<bool> enabled_;

        std::atomic<uint64_t> total_requests_;
        std::atomic<uint64_t> success_full_requests_;
        std::atomic<uint64_t> failed_requests_;
        std::atomic<uint64_t> cached_requests_;

        std::vector<double> resolution_times_;
        std::vector<double> speed_check_times_;
        size_t max_samples_;

        size_t current_cache_size_;
        size_t current_cache_capacity_;

        size_t current_memory_usage_;
        int current_active_threads_;
        int current_queued_tasks_;

        std::map<std::string, uint64_t> error_counts_;
    };
}

#endif //MEDIAPROXY_DNSMONITOR_H