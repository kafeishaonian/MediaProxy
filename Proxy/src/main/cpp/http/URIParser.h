//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_URIPARSER_H
#define MEDIAPROXY_URIPARSER_H

#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <boost/url.hpp>
#include <boost/url/url_view.hpp>
#include <boost/url/params_view.hpp>

class URIParser {

public:
    URIParser();
    bool build_url(const std::string& url_str);

    bool is_ipv6();

    std::string get_schema();

    std::string get_host();

    std::string get_port();

    std::string get_path();

    std::string get_query();

    std::string query(std::string key);

    void enum_query(std::function<void(const std::string &key, const std::string &val)> func);

    static std::string build_url(const std::string& schema, const std::string& host, const std::string& path);

    static std::string build_url(const std::string& schema, const std::string& host, uint64_t port, const std::string& path);

    static std::string build_url(const std::string& schema, const std::string& host, const std::string& path, const std::map<std::string, std::string>& user_query);

    static std::string build_url(const std::string& schema, const std::string& host, uint64_t port, const std::string& path, const std::map<std::string, std::string>& user_query);

    static std::string add_query(const std::string& url_str, const std::map<std::string, std::string>& user_query);

private:
    boost::urls::url url_;

    bool build_url_result_;
};


#endif //MEDIAPROXY_URIPARSER_H