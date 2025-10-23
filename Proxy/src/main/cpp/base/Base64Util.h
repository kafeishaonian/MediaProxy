//
// Created by Hongmingwei on 2025/10/20.
//

#ifndef MEDIAPROXY_BASE64UTIL_H
#define MEDIAPROXY_BASE64UTIL_H

#include <string>
#include <algorithm>
#include <stdexcept>

// 代码来自 https://github.com/ReneNyffenegger/cpp-base64.git


namespace PBase {
    std::string base64_encode(unsigned char const*, size_t len, bool url = false);
    std::string base64_decode(std::string const& s, bool remove_linebreaks = false);
};


#endif //MEDIAPROXY_BASE64UTIL_H
