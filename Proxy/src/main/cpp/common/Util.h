//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_UTIL_H
#define MEDIAPROXY_UTIL_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>

// 获取md5
void util_generate_md5_value(uint8_t *dst, uint8_t *src, int len);

// 获取当前的毫秒数
uint64_t util_get_current_time_in_milli_seconds();

uint64_t util_get_current_time_in_micro_seconds();

void remove_item_for_string_vector(std::vector<std::string>& vector, const std::string& item);

//base64编码
char* util_base64_encode(
        char* out,
        int out_size,
        const uint8_t* input,
        int input_size
        );

std::string join_string_from_queue(std::queue<std::string>& in_queue);

std::string join_int_from_queue(std::queue<int>& in_queue);

namespace proxy{
    template<typename T>
    std::string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }

    template<typename T>
    int string_to_int(T value) {
        int data;
        std::istringstream is(value);
        is >> data;
        return data;
    }

    template<typename T>
    int64_t string_to_int64(const T& value) {
        int64_t data;
        std::istringstream is(value);
        is >> data;
        return data;
    }

    template<typename T>
    uint64_t string_to_uint64(const T& value) {
        uint64_t data;
        std::istringstream is(value);
        is >> data;
        return data;
    }
}


#endif //MEDIAPROXY_UTIL_H

