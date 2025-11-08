//
// Created by Hongmingwei on 2025/10/31.
//

#include "HttpServerAdvancedSession.h"
#include "HttpServerCommon.h"
#include "URIParser.h"

HttpServerAdvancedSession::HttpServerAdvancedSession(
        boost::asio::ip::tcp::socket socket
) : socket_(std::move(socket)),
    strand_(socket_.get_executor()),
    handler_(nullptr) {

}

HttpServerAdvancedSession::~HttpServerAdvancedSession() {}

void HttpServerAdvancedSession::do_close() {

}

void HttpServerAdvancedSession::run() {
    do_read();
}

void HttpServerAdvancedSession::do_read() {
    boost::beast::http::async_read(socket_, buffer_, request_,
                                   boost::asio::bind_executor(strand_,
                                                              std::bind(
                                                                      &HttpServerAdvancedSession::on_read,
                                                                      shared_from_this(),
                                                                      std::placeholders::_1)));
}

void HttpServerAdvancedSession::on_read(boost::beast::error_code ec) {
    if (ec == boost::asio::error::operation_aborted) {
        return;
    }

    if (ec == boost::beast::http::error::end_of_stream) {
        return do_close();
    }

    if (ec) {
        //打印日志
        return;
    }

    switch (request_.method()) {
        case boost::beast::http::verb::get: {
            uint64_t req_time = util_get_current_time_in_milli_seconds(), tmp_time;
            current_session_request_info_ = std::make_shared<HttpServerSessionRequestInfo>(
                    req_time);
            {
                get_preload_param();
                handler_ = std::make_shared<HttpSessionHandler>(socket_, request_);
                handler_->process();
                return;
            }
        }
        case boost::beast::http::verb::head: {
            handle_head_request();
            break;
        }
        default: {
            break;
        }
    }
}

void HttpServerAdvancedSession::get_preload_param() {
    std::string origin_target{request_.target()};
    std::string preload_url_with_query = http_server_get_preload_url(origin_target, false);
    current_session_request_info_->req_url_ = preload_url_with_query;
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
            current_session_request_info_->req_key_ = preload_key_;
        } else if (k == "header") {
            preload_header_ = v;
            current_session_request_info_->download_url_header_ = preload_header_;
        } else if (k == "https") {
            is_https = true;
            current_session_request_info_->is_https_ = is_https;
        } else if (k == "sid") {
            cur_session_id_ = v;
            current_session_request_info_->session_id_ = cur_session_id_;
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
                                            (uint16_t) proxy::string_to_int(uri_parser.get_port()),
                                            uri_parser.get_path(), user_query);
    }
    current_session_request_info_->download_url_ = preload_url_;
    if (preload_key_.empty()) {
        preload_key_ = preload_path_;
    }
}


void HttpServerAdvancedSession::handle_head_request() {
    boost::beast::http::response<boost::beast::http::empty_body> response{
            boost::beast::http::status::ok, request_.version()};
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);

    boost::beast::error_code error_code;
    response.keep_alive(request_.keep_alive());
    response.content_length(100);

    boost::beast::http::response_serializer<boost::beast::http::empty_body> response_serializer{
            response};
    boost::beast::http::write(socket_, response_serializer, error_code);
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, error_code);
}
