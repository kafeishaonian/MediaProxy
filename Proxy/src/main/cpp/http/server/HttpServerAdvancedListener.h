//
// Created by Hongmingwei on 2025/10/29.
//

#ifndef MEDIAPROXY_HTTPSERVERADVANCEDLISTENER_H
#define MEDIAPROXY_HTTPSERVERADVANCEDLISTENER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/circular_buffer.hpp>

#include <iostream>
#include <vector>

#include "HttpServerAdvancedSession.h"
#include "HttpServerCommon.h"


class HttpServerAdvancedListener : public std::enable_shared_from_this<HttpServerAdvancedListener> {

public:
    HttpServerAdvancedListener(
            boost::asio::io_service &ios,
            boost::asio::ip::tcp::endpoint endpoint,
            const std::string &dec_root
    ) : strand_(ios.get_executor()), acceptor_(ios),
        socket_(ios), doc_root_(dec_root),
        session_list_(10) {

        status_ = false;
        is_stop_ = true;
        port_ = endpoint.port();
        address_ = endpoint.address();
        init();
    }

    void init() {
        boost::system::error_code ec;
        acceptor_.open(boost::asio::ip::tcp::v4(), ec);
        if (ec) {
            status_ = false;
            return;
        }
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

        int try_binding_times = 5;
        do {
            boost::asio::ip::tcp::endpoint endpoint{address_, port_};
            acceptor_.bind(endpoint, ec);
            port_ = acceptor_.local_endpoint().port();
            if (ec) {
                //打印日志
            } else {
                break;
            }
            try_binding_times--;
            port_ += 10000;
        } while (try_binding_times > 0);

        if (ec) {
            status_ = false;
            return;
        }

        acceptor_.listen(boost::asio::socket_base::max_connections, ec);
        if (ec) {
            status_ = false;
            return;
        }
        status_ = true;
    }

    ~HttpServerAdvancedListener() {
        un_init();
    }

    void un_init() {
        boost::system::error_code ec;
        acceptor_.close(ec);
        status_ = false;
        for (auto session : session_list_) {
            session->do_close();
        }
        session_list_.clear();
    }

    void start() {
        if (!acceptor_.is_open()) {
            status_ = false;
            return;
        }
        do_accept();
        is_stop_ = false;
    }

    void do_accept() {
        acceptor_.async_accept(socket_, std::bind(&HttpServerAdvancedListener::on_accept, this, std::placeholders::_1));
    }

    void on_accept(boost::system::error_code ec) {
        if (is_stop_) {
            return;
        }

        if (ec) {
            //日志__LOGE_TAG(TAG, "http server listener accept error:%d", ec.value());
        } else {
            std::shared_ptr<HttpServerAdvancedSession> session =
                    std::make_shared<HttpServerAdvancedSession>(std::move(socket_), doc_root_);
            session->run();
            session_list_.push_back(session);
        }

        bool is_open = acceptor_.is_open();

        if (is_open) {
            //日志 __LOGE_TAG(TAG, "http server listener stop...");
        } else {
            do_accept();
        }
    }

    uint16_t get_port() {
        return port_;
    }

    bool get_status() {
        return status_;
    }

    void stop() {
        is_stop_ = true;
        boost::system::error_code ec;
        acceptor_.cancel(ec);
    }

    bool is_listening() {
        return acceptor_.is_open();
    }

private:
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::string const &doc_root_;
    boost::asio::ip::address address_;
    uint16_t port_;
    bool status_;
    bool is_stop_;
    boost::circular_buffer<std::shared_ptr<HttpServerAdvancedSession>> session_list_;

};


#endif //MEDIAPROXY_HTTPSERVERADVANCEDLISTENER_H
