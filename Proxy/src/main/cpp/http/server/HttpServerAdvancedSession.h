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
#include "HttpSessionHandler.h"
#include "Util.h"


class HttpServerAdvancedSession : public std::enable_shared_from_this<HttpServerAdvancedSession> {

public:
    explicit HttpServerAdvancedSession(
            boost::asio::ip::tcp::socket socket
    );

    ~HttpServerAdvancedSession();

    void do_close();

    void run();

    void do_read();

    void on_read(boost::beast::error_code ec);

private:
    void get_preload_param();

    //处理head请求
    void handle_head_request();


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
    std::shared_ptr<HttpSessionHandler> handler_;
};

#endif //MEDIAPROXY_HTTPSERVERADVANCEDSESSION_H
