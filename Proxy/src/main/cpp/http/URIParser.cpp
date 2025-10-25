//
// Created by Hongmingwei on 2025/10/24.
//

#include "URIParser.h"
#include "common/Util.h"

URIParser::URIParser() : build_url_result_(true) {
}

bool URIParser::build_url(const std::string &url_str) {
    try {
        auto result = boost::urls::parse_uri_reference(url_str);
        if (!result) {
            build_url_result_ = false;
            return false;
        }

        url_ = result.value();
        build_url_result_ = url_.has_authority();
        return build_url_result_;
    } catch (std::exception &e) {
        build_url_result_ = false;
        return false;
    }
}


bool URIParser::is_ipv6() {
    if (!build_url_result_) {
        return false;
    }

    try {
        return url_.host_type() == boost::urls::host_type::ipv6;
    } catch (std::exception &e) {
        return false;
    }
}


std::string URIParser::get_schema() {
    if (!build_url_result_) {
        return "";
    }
    try {
        return std::string(url_.scheme());
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::get_host() {
    if (!build_url_result_) {
        return "";
    }
    try {
        return url_.host();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::get_port() {
    if (!build_url_result_) {
        return "";
    }
    try {
        return std::string(url_.port());
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::get_path() {
    if (!build_url_result_) {
        return "";
    }
    try {
        return url_.path();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::get_query() {
    if (!build_url_result_)
        return "";

    try {
        return url_.query();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::query(std::string key) {
    if (!build_url_result_) {
        return "";
    }

    try {
        for (auto param: url_.params()) {
            if (param.key == key) {
                return param.value;
            }
        }
        return "";
    } catch (std::exception &e) {
        return "";
    }
}


void URIParser::enum_query(std::function<void(const std::string &, const std::string &)> func) {
    if (!build_url_result_)
        return;

    try {
        auto params = url_.params();

        for (const auto &param: params) {
            func(std::string(param.key), std::string(param.value));
        }
    } catch (std::exception &e) {
    }
}

std::string URIParser::build_url(
        const std::string &schema,
        const std::string &host,
        const std::string &path
) {
    try {
        boost::urls::url url_builder;
        url_builder.set_scheme(schema)
                .set_host(host)
                .set_path(path);
        return url_builder.buffer();
    } catch (std::exception &e) {
        return "";
    }
}


std::string URIParser::build_url(
        const std::string &schema,
        const std::string &host,
        uint64_t port,
        const std::string &path
) {
    try {
        boost::urls::url url_builder;
        url_builder.set_scheme(schema)
                .set_host(host)
                .set_port_number(port)
                .set_path(path);
        return url_builder.buffer();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::build_url(
        const std::string &schema,
        const std::string &host,
        const std::string &path,
        const std::map<std::string, std::string> &user_query
) {
    try {
        boost::urls::url url_builder;
        url_builder.set_scheme(schema)
                .set_host(host)
                .set_path(path);

        auto params = url_builder.params();
        for (const auto &kv: user_query) {
            params.append({kv.first, kv.second});
        }

        return url_builder.buffer();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::build_url(
        const std::string &schema,
        const std::string &host,
        uint64_t port,
        const std::string &path,
        const std::map<std::string, std::string> &user_query
) {
    try {
        boost::urls::url url_builder;
        url_builder.set_scheme(schema)
                .set_host(host)
                .set_port_number(port)
                .set_path(path);

        auto params = url_builder.params();
        for (const auto &kv: user_query) {
            params.append({kv.first, kv.second});
        }

        return url_builder.buffer();
    } catch (std::exception &e) {
        return "";
    }
}

std::string URIParser::add_query(
        const std::string &url_str,
        const std::map<std::string, std::string> &user_query
) {
    try {
        boost::urls::url url_builder(url_str);

        // 检查是否有有效主机
        if (!url_builder.has_authority()) {
            return "";
        }

        // 添加查询参数
        auto params = url_builder.params();
        for (const auto& kv : user_query) {
            params.append({kv.first, kv.second});
        }

        return url_builder.buffer();
    } catch (std::exception& e) {
        return "";
    }
}