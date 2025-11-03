//
// Created by Hongmingwei on 2025/10/29.
//

#ifndef MEDIAPROXY_HTTPPROXY_H
#define MEDIAPROXY_HTTPPROXY_H

#include <iostream>
#include <list>
#include <cstdio>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>

#include "HttpServerAdvancedBeast.h"

class HttpProxy {

public:
    HttpProxy(const std::string& config_path,
              const std::string& address,
              uint16_t port,
              int server_thread_number);


    ~HttpProxy();

    int start();

    int stop();

    int restart();



private:

    std::string config_path_;
    std::string local_address_;
    uint16_t port_;
    int server_thread_number_;

    std::shared_ptr<HttpServerAdvancedBeast> http_server_advanced_beast_;
};


#endif //MEDIAPROXY_HTTPPROXY_H


