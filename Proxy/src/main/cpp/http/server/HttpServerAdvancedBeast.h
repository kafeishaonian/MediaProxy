//
// Created by Hongmingwei on 2025/10/29.
//

#ifndef MEDIAPROXY_HTTPSERVERADVANCEDBEAST_H
#define MEDIAPROXY_HTTPSERVERADVANCEDBEAST_H

#include <string>
#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>

#include "NamedThread.h"
#include "HttpServerAdvancedListener.h"

class HttpServerAdvancedBeast {

public:
    HttpServerAdvancedBeast(const std::string &address_string,
                            const std::string &doc_root,
                            uint16_t port,
                            int threads);

    ~HttpServerAdvancedBeast();

    void start();

    void stop();


private:
    std::string address_string_;

    uint16_t port_;

    int threads_;

    std::string doc_root_;

    std::shared_ptr<HttpServerAdvancedListener> listener_;

    boost::asio::io_context io_service_;

    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> work_;

    bool is_run_done_;

    std::vector<std::shared_ptr<proxy::NamedThread>> thread_list_;

};

#endif //MEDIAPROXY_HTTPSERVERADVANCEDBEAST_H
