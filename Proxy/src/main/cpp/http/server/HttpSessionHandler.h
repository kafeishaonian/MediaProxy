//
// Created by Hongmingwei on 2025/10/31.
//

#ifndef MEDIAPROXY_HTTPSESSIONHANDLER_H
#define MEDIAPROXY_HTTPSESSIONHANDLER_H

#include <memory>
#include <string>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

#include "PreloadCommon.h"
#include "PreloadResult.h"
#include "HttpServerSessionRequestInfo.h"
#include "PreloadManager.h"
#include "GlobalConfig.h"
#include "Util.h"
#include "HttpServerCommon.h"
#include "URIParser.h"

class HttpSessionHandler: public std::enable_shared_from_this<HttpSessionHandler>,
                          public PreloadTaskComplete {
public:
    HttpSessionHandler(boost::asio::ip::tcp::socket &socket,
                       boost::beast::http::request<boost::beast::http::string_body> &request);

    ~HttpSessionHandler() {

    }

    static std::shared_ptr<HttpSessionHandler> generate_session_handler(std::string& request_url);

    int process();

    void set_transmit_socket(boost::asio::ip::tcp::socket& socket);

    void set_http_request(boost::beast::http::request<boost::beast::http::string_body>& request);

    void close_socket();


private:

    void on_read(boost::system::error_code error_code);

    void on_tracker_result(int& code, const std::vector<std::string>& peer_ids, uint64_t& file_size, const std::string& error_string);

    void print_request();

    void load_global_config();

    int64_t get_file_size(int& read_try_count);

    void get_preload_param();

    std::string build_url(const std::string &schema, const std::string &host,
                          const std::string &path);


    std::string build_url(const std::string &schema, const std::string &host,
                          const std::string &path,
                          const std::map<std::string, std::string> &user_query);

    std::string build_url(const std::string &schema, const std::string &host,
                          const std::string &port, const std::string &path,
                          const std::map<std::string, std::string> &user_query);

    void shut_down();

    void download_data_from_offset(uint64_t offset, int64_t player_load_size);

    bool build_response_header(boost::beast::http::response<boost::beast::http::buffer_body> &response, uint64_t offset,
                               int read_try_count);

    void get_request_range();

    int64_t get_player_load_size();


    void on_task_complete(std::shared_ptr<IPreloadResult> result);

    int clear_complete_task();

    void remove_preload_tasks_when_exit();

    void optimize_play_task_start_at_begin(int64_t cache_size, int64_t preload_size);

    void optimize_http_transfer_mode(bool is_cache_complete);

    int request_node_sync();

    int request_node_async();

    void update_download_type(uint64_t data_remain_bytes, uint64_t offset);

    bool is_file_cache_complete();

    void check_if_need_download_with_buffer_left(uint64_t offset);

    int get_suggest_concurrent_server_task(TRANSFER_TYPE transfer_type);

    void wait_tracker_future();

    void wait_tracker_when_exit();

    void get_remote_peers();

    void wait_tracker_response();


private:
    boost::asio::ip::tcp::socket socket_;

    boost::beast::http::request<boost::beast::http::string_body> request_;

    boost::asio::strand<boost::asio::any_io_executor> strand_;
    int try_count_;
    int64_t player_load_size_;
    std::shared_ptr<std::vector<float>> player_load_size_factor_;
    int factor_index_;
    int read_sleep_time_;
    int min_playable_size_;

    bool is_http_error_;
    boost::beast::flat_buffer monitor_buffer_;
    boost::beast::http::request<boost::beast::http::string_body> monitor_request_;

    bool pause_preload_;

    std::string preload_key_;
    std::string preload_url_;
    std::string preload_header_;
    std::string preload_path_;
    std::string current_session_id_;

    int64_t range_start_;
    int64_t range_end_;
    int64_t content_length_;
    int64_t file_size_;

    int64_t required_preload_size_;
    int64_t preloaded_size_;
    int64_t cached_size_;

    std::shared_ptr<HttpServerSessionRequestInfo> current_session_request_info_start_;
    std::shared_ptr<HttpServerSessionRequestInfo> current_session_request_info_stop_;

    uint64_t read_cnt_;
    PreloadManager* preload_manager_;

    int number_success_removed_task_;
    int number_not_found_task_;
    int number_started_task_;

    bool is_first_network_packet_;


    bool is_need_download_;
    bool is_cache_complete_;
    uint64_t download_offset_;
    int64_t player_load_dynamic_size_;

    int native_handle_;

    std::mutex wait_mutex_;
    std::condition_variable wait_condition_;
};


#endif //MEDIAPROXY_HTTPSESSIONHANDLER_H
