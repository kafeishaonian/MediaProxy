//
// Created by 魏红明 on 2025/10/26.
//

#include "GlobalConfig.h"

#include <utility>
#include "Singleton.h"
#include "PreloadCommon.h"

GlobalConfig::GlobalConfig() {

    http_server_player_preload_size_ = GlobalConfigPlayerPreloadSize;

    server_port_ = 0;

    http_server_read_try_times_ = 400;

    http_server_read_sleep_time_ = 50;

    cache_lock_timeout_in_ms_ = 500;

    memory_cache_expired_time_in_second_ = 10;

    enable_mapped_file_cache_ = false;

    min_file_size_upload_tracker_ = 600 * 1024;

    handler_instance_ = 0;

    http_server_min_playable_size_ = 1048576;
}

GlobalConfig::~GlobalConfig() {

}

GlobalConfig *GlobalConfig::get_instance() {
    return Singleton<GlobalConfig>::get_instance();
}

void GlobalConfig::set_cache_lock_timeout_in_ms(int timeout) {
    proxy::WriteLock lock(read_write_lock_);
    cache_lock_timeout_in_ms_ = timeout;
}

int GlobalConfig::get_cache_lock_timeout_in_ms() {
    proxy::WriteLock lock(read_write_lock_);
    return cache_lock_timeout_in_ms_;
}

void GlobalConfig::set_memory_expired_time_in_second(int second) {
    proxy::WriteLock lock(read_write_lock_);
    memory_cache_expired_time_in_second_ = second;
}

int GlobalConfig::get_memory_expired_time_in_second() {
    proxy::WriteLock lock(read_write_lock_);
    return memory_cache_expired_time_in_second_;
}

void GlobalConfig::set_enable_mapped_file_cache(bool enable) {
    proxy::WriteLock lock(read_write_lock_);
    enable_mapped_file_cache_ = enable;
}

bool GlobalConfig::get_enable_mapped_file_cache() {
    proxy::WriteLock lock(read_write_lock_);
    return enable_mapped_file_cache_;
}

void GlobalConfig::set_min_file_size_upload_tracker(uint64_t min_size) {
    proxy::WriteLock lock(read_write_lock_);
    min_file_size_upload_tracker_ = min_size;
}

uint64_t GlobalConfig::get_min_file_size_upload_tracker() {
    proxy::WriteLock lock(read_write_lock_);
    return min_file_size_upload_tracker_;
}

void GlobalConfig::set_server_port(uint16_t port) {
    proxy::WriteLock lock(read_write_lock_);
    server_port_ = port;
}

uint16_t GlobalConfig::get_server_port() {
    proxy::WriteLock lock(read_write_lock_);
    return server_port_;
}


void GlobalConfig::add_or_decrease_handler_instance(bool add_instance) {
    proxy::WriteLock lock(read_write_lock_);
    if (add_instance) {
        handler_instance_++;
    } else {
        handler_instance_--;
    }
}

int GlobalConfig::get_handler_instance() {
    proxy::WriteLock lock(read_write_lock_);
    return handler_instance_;
}

int GlobalConfig::get_http_server_read_try_times() {
    proxy::WriteLock lock(read_write_lock_);
    return http_server_read_try_times_;
}

void GlobalConfig::set_http_server_read_try_times(int times) {
    proxy::WriteLock lock(read_write_lock_);
    http_server_read_try_times_ = times;
}


int GlobalConfig::get_http_server_read_sleep_time_in_milli_second() {
    proxy::WriteLock lock(read_write_lock_);
    return http_server_read_sleep_time_;
}

void GlobalConfig::set_http_server_read_sleep_time_in_milli_second(int milli_second) {
    proxy::WriteLock lock(read_write_lock_);
    http_server_read_sleep_time_ = milli_second;
}

void GlobalConfig::set_http_server_player_load_size(uint64_t size) {
    proxy::WriteLock lock(read_write_lock_);
    http_server_player_preload_size_ = size;
    min_file_size_upload_tracker_ = size;
}

uint64_t GlobalConfig::get_http_server_player_load_size() {
    proxy::WriteLock lock(read_write_lock_);
    return http_server_player_preload_size_;
}


std::shared_ptr<std::vector<float>> GlobalConfig::get_http_server_player_load_size_factor() {
    proxy::WriteLock lock(read_write_lock_);
    return http_server_player_preload_size_factor_;
}

void
GlobalConfig::set_http_server_player_load_size_factor(std::shared_ptr<std::vector<float>> factor) {
    proxy::WriteLock lock(read_write_lock_);
    http_server_player_preload_size_factor_ = std::move(factor);
}

int GlobalConfig::get_http_server_min_playable_size() {
    proxy::WriteLock lock(read_write_lock_);
    return http_server_min_playable_size_;
}

void GlobalConfig::set_http_server_min_playable_size(int size) {
    proxy::WriteLock lock(read_write_lock_);
    http_server_min_playable_size_ = size;
}