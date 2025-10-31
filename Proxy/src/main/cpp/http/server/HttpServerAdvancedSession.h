//
// Created by Hongmingwei on 2025/10/31.
//

#ifndef MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H
#define MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <iostream>

#include "HttpServerSessionRequestInfo.h"
#include "IHttpSessionHandler.h"


class HttpServerAdvancedSession : public std::enable_shared_from_this<HttpServerAdvancedSession> {

public:
    explicit HttpServerAdvancedSession(
            boost::asio::ip::tcp::socket socket,
            std::string const &doc_root
    ) : socket_(std::move(socket)),
        strand_(socket_.get_executor()),
        handler_(nullptr) {


    }

    ~HttpServerAdvancedSession() {

    }

    void do_close() {

    }

    void run() {

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
