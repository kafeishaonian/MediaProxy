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

class HttpSessionHandler: public std::enable_shared_from_this<HttpSessionHandler>{
public:
    HttpSessionHandler(boost::asio::ip::tcp::socket &socket,
                       boost::beast::http::request<boost::beast::http::string_body> &request) :
            socket_(std::move(socket)),
            request_(std::move(request)),
            current_transfer_type_(TRANSFER_TYPE_HTTP) {
    }

    ~HttpSessionHandler() {

    }

    static std::shared_ptr<HttpSessionHandler> generate_session_handler(std::string& request_url);

    int process();

    void set_transmit_socket(boost::asio::ip::tcp::socket& socket) {
        socket_ = std::move(socket);
    }

    void set_http_request(boost::beast::http::request<boost::beast::http::string_body>& request) {
        request_ = std::move(request);
    }

    void close_socket() {
        boost::system::error_code error_code;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error_code);
    }


private:
    boost::asio::ip::tcp::socket socket_;

    boost::beast::http::request<boost::beast::http::string_body> request_;

    TRANSFER_TYPE current_transfer_type_;

};


#endif //MEDIAPROXY_HTTPSESSIONHANDLER_H
