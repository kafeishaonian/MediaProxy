//
// Created by Hongmingwei on 2025/10/31.
//

#include "HttpServerCommon.h"
#include "GlobalConfig.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


std::string http_server_path_cat(
        boost::beast::string_view base,
        boost::beast::string_view path
) {
    if (base.empty()) {
        return std::string{path};
    }

    std::string result{base};

#if BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
    for (auto& c : result) {
        if (c == '/') {
            c = path_separator;
        }
    }
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator) {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
#endif
    return result;
}

std::string http_server_get_preload_url(const std::string& path, bool use_https) {
    std::string url;
    std::stringstream stream;
    bool is_ipv6 = path.find("ipv6=1") != std::string::npos;
    std::string final_path = path;
    if (is_ipv6) {
        size_t pos = path.find("/", 2);
        std::string host = path.substr(1, pos - 1);
        std::string sub_path = path.substr(pos, path.length() - pos);
        final_path = "/[" + host + "]" + sub_path;
    }
    if (use_https) {
        stream << "https:/" << final_path;
        url = stream.str().c_str();
    } else {
        stream << "http:/" << final_path;
        url = stream.str().c_str();
    }
    return url;
}

std::pair<int64_t, int64_t> http_server_get_request_range(boost::beast::http::request<boost::beast::http::string_body> const& request) {
    std::string bytes = "bytes=";
    int64_t range_start = -1;
    int64_t range_end = -1;
    std::pair<int64_t, int64_t> pair;

    try {
        boost::beast::string_view view_rang = request[boost::beast::http::field::range];
        if (!view_rang.empty()) {
            std::string range{view_rang};
            if (boost::starts_with(range, bytes)) {
                std::string value_string = range.substr(bytes.size());

                std::vector<std::string> split_vector;
                boost::split(split_vector, value_string, boost::is_any_of("-"));

                for (size_t i = 0; i < split_vector.size(); i++) {
                    if (split_vector[i].empty()) {
                        break;
                    }
                    if (i == 0) {
                        range_start = boost::lexical_cast<int64_t>(split_vector[i]);
                    } else {
                        range_end = boost::lexical_cast<int64_t>(split_vector[i]);
                    }
                }
            }
        }
    } catch (std::exception& exception) {

    }
    pair.first = range_start;
    pair.second = range_end;
    return pair;
}
