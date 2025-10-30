//
// Created by Hongmingwei on 2025/10/23.
//

#include "ProxyInterface.h"

#include <utility>
#include "CacheManager.h"

#include "boost/make_unique.hpp"

ProxyInterface::ProxyInterface() {
    server_status_ = ServerStatus_Create;
}

ProxyInterface::~ProxyInterface() {

}

std::shared_ptr<ProxyInterface> &ProxyInterface::get_instance() {
    return SingletonShared<ProxyInterface>::get_instance();
}

void ProxyInterface::init() {
    server_status_ = ServerStatus_Init;
}

void ProxyInterface::un_init() {
    server_status_ = ServerStatus_UnInit;


}

int ProxyInterface::setup_cache(const char *path) {
    DiskCache::get_instance()->set_cache_path(path);
    CacheManager::get_instance()->start_serialize_task();
    return 0;
}

int ProxyInterface::setup_proxy(
        const std::string config_path,
        const std::string address,
        uint16_t port,
        int server_thread_number) {

    server_config_path_ = std::move(config_path);
    server_address_ = std::move(address);
    server_port_ = port;
    server_thread_number = server_thread_number;
    http_server_ = boost::make_unique<HttpProxy>(config_path, address, port, server_thread_number);
    return 0;
}

std::string ProxyInterface::switch_play_url(
        const char *play_url,
        const char *key,
        const char *real_host) {

    URIParser parser;
    parser.build_url(play_url);

    std::string path = parser.get_path();
    std::string schema = parser.get_schema();
    std::string origin_host = parser.get_host();

    std::map<std::string, std::string> user_query;
    parser.enum_query([&](const std::string &k, const std::string &v) {
        user_query[k] = v;
    });

    std::stringstream stream;
    stream << "/" << origin_host << path;
    std::string new_path;
    new_path = stream.str().c_str();
    if (schema == "https") {
        user_query["https"] = "1";
    }
    if (key) {
        user_query["key"] = key;
    }

    if (real_host) {
        user_query["header"] = real_host;
    }
    if (parser.is_ipv6()) {
        user_query["ipv6"] = "1";
    }

    std::string host = server_address_;
    std::string session = generate_session();
    user_query["sid"] = session;

    auto port = GlobalConfig::get_instance()->get_server_port();
    std::string proxy_url = URIParser::build_url("http", host, port, new_path, user_query);

    return proxy_url;
}

std::string ProxyInterface::generate_session() {
    uint64_t current = util_get_current_time_in_milli_seconds();
    srand((uint32_t)time(nullptr));
    int random = rand();
    char md5_key[40];
    std::stringstream stream;
    stream << proxy::to_string(current);
    stream << proxy::to_string(random);
    std::string seed = stream.str();
    util_generate_md5_value((uint8_t *)md5_key, (uint8_t *)seed.c_str(), (int) strlen(seed.c_str()));
    return std::string(md5_key);
}

int ProxyInterface::start() {
    server_status_ = ServerStatus_Started;
    http_server_->start(true);
    return 0;
}

