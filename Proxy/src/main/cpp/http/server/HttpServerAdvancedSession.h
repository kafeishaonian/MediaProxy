//
// Created by Hongmingwei on 2025/10/31.
//

#ifndef MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H
#define MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <iostream>

#include "HttpServerSessionRequestInfo.h"
#include "IHttpSessionHandler.h"
#include "Util.h"


class HttpServerAdvancedSession : public std::enable_shared_from_this<HttpServerAdvancedSession> {

public:
    explicit HttpServerAdvancedSession(
            boost::asio::ip::tcp::socket socket
    ) : socket_(std::move(socket)),
        strand_(socket_.get_executor()),
        handler_(nullptr) {


    }

    ~HttpServerAdvancedSession() {

    }

    void do_close() {

    }

    void run() {
        do_read();
    }

    void do_read() {
        boost::beast::http::async_read(socket_, buffer_, request_,
                                       boost::asio::bind_executor(strand_,
                                           std::bind(&HttpServerAdvancedSession::on_read, shared_from_this(),
                                               std::placeholders::_1, std::placeholders::_2)));
    }

    void on_read(boost::beast::error_code ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        if (ec == boost::beast::http::error::end_of_stream) {
            return do_close();
        }

        if (ec) {
            // TODO: 实现错误处理日志 MHttpServerFail(ec, "read");
            return;
        }
        
        switch (request_.method()) {
            case boost::beast::http::verb::get: {
                uint64_t req_time = util_get_current_time_in_milli_seconds(), tmp_time;
//                current_session_request_info_ = std::make_shared<HttpServerSessionRequestInfo>(req_time);
                {
//                    getPreloadParam2();
//                    handler_ = CreateHTTPSessionHandler();
//                    handler_->process();
                    return;
                }
            }
            case boost::beast::http::verb::head: {
                // handleHeadRequest();
                break;
            }
            default: {
                break;
            }
        }
    }

private:
    boost::asio::ip::tcp::socket socket_;
    boost::beast::flat_buffer buffer_;
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    boost::beast::http::request<boost::beast::http::string_body> request_;

    std::string preload_key_;
    std::string preload_url_;
    std::string preload_header_;
    std::string preload_path_;
    std::string cur_session_id_;

    std::shared_ptr<HttpServerSessionRequestInfo> current_session_request_info_;
    std::shared_ptr<IHttpSessionHandler> handler_;
};

#endif //MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H
