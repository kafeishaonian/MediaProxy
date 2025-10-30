//
// Created by Hongmingwei on 2025/10/29.
//

#include "HttpProxy.h"

HttpProxy::HttpProxy(const std::string &config_path, const std::string &address, uint16_t port,
                     int server_thread_number) {
    config_path_ = config_path;
    local_address_ = address;
    port_ = port;
    server_thread_number_ = server_thread_number;
}

HttpProxy::~HttpProxy() {

}


int HttpProxy::start(bool spin) {

}