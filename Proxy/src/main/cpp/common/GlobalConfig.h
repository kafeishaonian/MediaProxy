//
// Created by 魏红明 on 2025/10/26.
//

#ifndef MEDIAPROXY_GLOBALCONFIG_H
#define MEDIAPROXY_GLOBALCONFIG_H

#include <cstdio>
#include <string>
#include <list>

#include "ThreadUtil.h"

const uint64_t GlobalConfigPlayerPreloadSize = 1 * 1024 * 1024;
const uint64_t GlobalConfigProxyServerPort = 10101;


class GlobalConfig {

public:
    static GlobalConfig *get_instance();

    GlobalConfig();

    ~GlobalConfig();

    void set_cache_lock_timeout_in_ms(int timeout);

    int get_cache_lock_timeout_in_ms();

    void set_memory_expired_time_in_second(int second);

    int get_memory_expired_time_in_second();

    void set_enable_mapped_file_cache(bool enable);

    bool get_enable_mapped_file_cache();

    void set_min_file_size_upload_tracker(uint64_t min_size);

    uint64_t get_min_file_size_upload_tracker();

    void set_server_port(uint16_t port);

    uint16_t get_server_port();

    void add_or_decrease_handler_instance(bool add_instance);

    int get_handler_instance();

    int get_http_server_read_try_times();

    void set_http_server_read_try_times(int times);

    int get_http_server_read_sleep_time_in_milli_second();

    void set_http_server_read_sleep_time_in_milli_second(int milli_second);

    void set_http_server_player_load_size(uint64_t size);

    uint64_t get_http_server_player_load_size();

    void set_http_server_player_load_size_factor(std::shared_ptr<std::vector<float>> factor);

    std::shared_ptr<std::vector<float>> get_http_server_player_load_size_factor();

    void set_http_server_min_playable_size(int size);

    int get_http_server_min_playable_size();

private:

    uint16_t server_port_;

    std::shared_mutex read_write_lock_;

    int cache_lock_timeout_in_ms_;
    int memory_cache_expired_time_in_second_;

    bool enable_mapped_file_cache_;

    uint64_t min_file_size_upload_tracker_;

    int handler_instance_;

    int http_server_read_try_times_;

    int http_server_read_sleep_time_;

    uint64_t http_server_player_preload_size_;

    std::shared_ptr<std::vector<float>> http_server_player_preload_size_factor_;

    int http_server_min_playable_size_;
};


#endif //MEDIAPROXY_GLOBALCONFIG_H
