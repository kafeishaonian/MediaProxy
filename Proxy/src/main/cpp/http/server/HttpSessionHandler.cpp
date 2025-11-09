//
// Created by Hongmingwei on 2025/10/31.
//

#include "HttpSessionHandler.h"


HttpSessionHandler::HttpSessionHandler(
        boost::asio::ip::tcp::socket &socket,
        boost::beast::http::request<boost::beast::http::string_body> &request
) : socket_(std::move(socket)),
    request_(std::move(request)),
    strand_(socket_.get_executor()),
    try_count_(20),
    player_load_size_(1024 * 1024),
    pause_preload_(false),
    min_playable_size_(1024 * 1024),
    number_success_removed_task_(0),
    number_not_found_task_(0),
    number_started_task_(0),
    is_first_network_packet_(true),
    is_need_download_(false),
    file_size_(0) {

    preload_manager_ = PreloadManager::get_instance();
}


int HttpSessionHandler::process() {
    native_handle_ = socket_.native_handle();
    GlobalConfig::get_instance()->add_or_decrease_handler_instance(true);

    uint64_t req_time = util_get_current_time_in_milli_seconds(), tmp_time;

    current_session_request_info_start_ = std::make_shared<HttpServerSessionRequestInfo>(req_time);
    current_session_request_info_stop_ = std::make_shared<HttpServerSessionRequestInfo>(req_time);

    print_request();
    load_global_config();

    is_http_error_ = false;

    boost::beast::http::async_read(socket_, monitor_buffer_, monitor_request_,
                                   boost::asio::bind_executor(strand_,
                                                              std::bind(
                                                                      &HttpSessionHandler::on_read,
                                                                      shared_from_this(),
                                                                      std::placeholders::_1)));
    get_preload_param();
    get_request_range();

    init_play_time_ = 0;
    tmp_time = util_get_current_time_in_milli_seconds();
    current_session_request_info_start_->parse_req_used_time_ = (uint32_t)(tmp_time - req_time);
    current_session_request_info_stop_->parse_req_used_time_ = (uint32_t)(tmp_time - req_time);
    int read_try_count = 0;

    is_cache_complete_ = is_file_cache_complete();
    player_load_dynamic_size_ = player_load_size_;
    download_offset_ = 0;

    build_session_start_info();

    tracker_start_ = util_get_current_time_in_milli_seconds();

    uint64_t file_size_start = util_get_current_time_in_milli_seconds();
    file_size_ = get_file_size(read_try_count);
}



//uint64_t getFileSizeStart = MUtilGetCurrentTimeInMilliSeconds();
//mFileSize = getFileSize(readTryCount);
//mGetFileSizeCost = int(MUtilGetCurrentTimeInMilliSeconds() - getFileSizeStart);
//
//upErrorFileSizeLog(mPreloadURL);
//
//optimizeHttpTransferMode(mIsCacheComplete);
//
//reqTime = MUtilGetCurrentTimeInMilliSeconds();
//mCurSessionReqInfoStop->mContentSizeUsedTime = (uint32_t) (reqTime - tmpTime);
//
//uint8_t buffer[8192];
//uint64_t bufferSize = 8192;
//int64_t readSize = 0;
//int64_t totalReadSize = 0;
//readTryCount = 0;
//uint64_t offset = 0;
//int isEof = 0;
//mReadCnt = 0;
//// 一般播放器请求只有 Range: bytes=0-
//// 或者 Range: bytes=rangeStart-
////            if (rangeStart >= 0 && rangeEnd > 0) {
//if (mRangeStart > 0) {
//offset = mRangeStart;
//} else {
//offset = 0;
//}
//
//http::response<http::buffer_body> response;
//http::response_serializer<http::buffer_body> serializer{response};
//bool isNeedMsgBody = buildResponseHeader(response, offset, readTryCount);
//boost::beast::error_code errorCode;
//tmpTime = MUtilGetCurrentTimeInMilliSeconds();
//mCurSessionReqInfoStop->mBuildResUsedTime = (uint32_t) (tmpTime - reqTime);
//
//bool isFirstPkt = true;
//mIsReadCompleteDataForParse = false;

void HttpSessionHandler::set_transmit_socket(boost::asio::ip::tcp::socket &socket) {
    socket_ = std::move(socket);
}

void HttpSessionHandler::set_http_request(
        boost::beast::http::request<boost::beast::http::string_body> &request) {
    request_ = std::move(request);
}

void HttpSessionHandler::close_socket() {
    boost::system::error_code error_code;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error_code);
}

void HttpSessionHandler::print_request() {
    std::stringstream stream;
    stream << request_;
    std::string stream_string = stream.str();
    if (!stream_string.empty()) {
        //日志
    }
}

void HttpSessionHandler::load_global_config() {
    try_count_ = GlobalConfig::get_instance()->get_http_server_read_try_times();
    read_sleep_time_ = GlobalConfig::get_instance()->get_http_server_read_sleep_time_in_milli_second();
    player_load_size_ = GlobalConfig::get_instance()->get_http_server_player_load_size();
    factor_index_ = 0;
    player_load_size_factor_ = GlobalConfig::get_instance()->get_http_server_player_load_size_factor();
    min_playable_size_ = GlobalConfig::get_instance()->get_http_server_min_playable_size();
}

void HttpSessionHandler::on_read(boost::system::error_code error_code) {
    if (error_code) {
        is_http_error_ = true;
        UniqueLock lock(wait_mutex_);
        wait_condition_.notify_all();
    } else {
        boost::beast::http::async_read(socket_, monitor_buffer_, monitor_request_,
                                       boost::asio::bind_executor(strand_,
                                                                  std::bind(
                                                                          &HttpSessionHandler::on_read,
                                                                          shared_from_this(),
                                                                          std::placeholders::_1)));
    }
}

void HttpSessionHandler::get_preload_param() {
    std::string origin_target{request_.target()};
    std::string preload_url_with_query = http_server_get_preload_url(origin_target, false);
    current_session_request_info_start_->req_url_ = preload_url_with_query;
    current_session_request_info_stop_->req_url_ = preload_url_with_query;

    URIParser uri_parser;
    if (!uri_parser.build_url(preload_url_with_query)) {
        //日志
    }
    preload_path_ = uri_parser.get_path();
    std::map<std::string, std::string> user_query;
    bool is_https = false;

    uri_parser.enum_query([&](const std::string &k, const std::string &v) {
        if (k == "key") {
            preload_key_ = v;
            current_session_request_info_start_->req_key_ = preload_key_;
            current_session_request_info_stop_->req_key_ = preload_key_;
        } else if (k == "header") {
            preload_header_ = v;
            current_session_request_info_start_->download_url_header_ = preload_header_;
            current_session_request_info_stop_->download_url_header_ = preload_header_;
        } else if (k == "https") {
            is_https = true;
            current_session_request_info_start_->is_https_ = is_https;
            current_session_request_info_stop_->is_https_ = is_https;
        } else if (k == "sid") {
            current_session_id_ = v;
            current_session_request_info_start_->session_id_ = current_session_id_;
            current_session_request_info_stop_->session_id_ = current_session_id_;
        } else {
            user_query[k] = v;
        }
    });

    preload_url_ = "";
    if (uri_parser.get_port().empty()) {
        preload_url_ = URIParser::build_url((is_https ? "https" : "http"), uri_parser.get_host(),
                                            uri_parser.get_path(), user_query);
    } else {
        preload_url_ = URIParser::build_url((is_https ? "https" : "http"), uri_parser.get_host(),
                                            (uint64_t) proxy::string_to_int(uri_parser.get_port()),
                                            uri_parser.get_path(), user_query);
    }

    current_session_request_info_start_->download_url_ = preload_url_;
    current_session_request_info_stop_->download_url_ = preload_url_;
    if (preload_key_.empty()) {
        preload_key_ = preload_path_;
    }
}


void HttpSessionHandler::get_request_range() {
    std::pair<int64_t, int64_t> request_range = http_server_get_request_range(request_);
    range_start_ = request_range.first;
    range_end_ = request_range.second;

    current_session_request_info_start_->req_range_start_ = range_start_;
    current_session_request_info_start_->req_range_end_ = range_end_;

    current_session_request_info_stop_->req_range_start_ = range_start_;
    current_session_request_info_stop_->req_range_end_ = range_end_;
}

bool HttpSessionHandler::is_file_cache_complete() {
    int64_t file_size = CacheManager::get_instance()->get_file_size(preload_key_.c_str());
    required_preload_size_ = GlobalConfig::get_instance()->get_preload_size();

    cached_size_ = CacheManager::get_instance()->get_instance_parameter(preload_key_.c_str(), CachedSizeKey);
    preloaded_size_ = CacheManager::get_instance()->get_instance_parameter(preload_key_.c_str(), PreloadSizeKey);

    bool is_cache_complete = CacheManager::get_instance()->is_cache_complete(preload_key_);
    return is_cache_complete;
}

void HttpSessionHandler::build_session_start_info() {
    current_session_request_info_start_-> is_cache_complete_ = is_cache_complete_;
    current_session_request_info_start_->handler_instance_ = GlobalConfig::get_instance()->get_handler_instance();
    ProxyInterface::get_instance()->append_server_result(current_session_request_info_start_);
}

int64_t HttpSessionHandler::get_file_size(int &read_try_count) {
    int64_t file_size = 0;
    int task_id = -1;
    do {
        file_size = CacheManager::get_instance()->get_file_size(preload_key_.c_str());
        if (file_size <= 0) {
            auto preload_url_cstring = (char *) preload_url_.c_str();
            auto file_key = (char *) preload_key_.c_str();
            const char *header = preload_header_.empty() ? nullptr : preload_header_.c_str();
            if (is_http_error_) {
                break;
            }

            if (!pause_preload_) {
                pause_preload_ = true;
                preload_manager_->pause_all_preload_task();
            }

            if (http_server_task_manager_->is_task_need_added()) {
                int64_t down_size = get_player_load_size();

                update_download_type(0, 0);
//                UniqueLock lock(add_task)
            }
        }
    } while (read_try_count < try_count_ && file_size <= 0);
    return file_size;
}

