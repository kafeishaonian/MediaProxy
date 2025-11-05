//
// Created by Hongmingwei on 2025/10/31.
//

#ifndef MEDIAPROXY_HTTPSERVERCOMMON_H
#define MEDIAPROXY_HTTPSERVERCOMMON_H

#include <iostream>
#include <boost/system/error_code.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>


std::string http_server_path_cat(
        boost::beast::string_view base,
        boost::beast::string_view path
        );

std::string http_server_get_preload_url(const std::string& path, bool use_https);

std::pair<int64_t, int64_t> http_server_get_request_range(boost::beast::http::request<boost::beast::http::string_body> const& request);


#endif //MEDIAPROXY_HTTPSERVERCOMMON_H