//
// Created by Hongmingwei on 2025/10/24.
//

#include "Util.h"

#include <sys/time.h>
#include <cstring>
#include <libavutil/md5.h>
#include <libavutil/base64.h>

void util_generate_md5_value(uint8_t *dst, uint8_t *src, int len) {
    if (!dst || !src || !len) {
        return;
    }

    uint8_t md5_key[16];
    memset(md5_key, 0, sizeof(md5_key));
    av_md5_sum(md5_key, src, len);
    int i;
    for (i = 0; i < 16; ++i) {
        uint8_t value = md5_key[i] >> 4;

        if (value < 0x0a) {
            dst[i * 2] = '0' + value;
        } else {
            dst[i * 2] = 'A' + value - 0x0a;
        }

        value = md5_key[i] & 0x0f;

        if (value < 0x0a) {
            dst[i * 2 + 1] = '0' + value;
        } else {
            dst[i * 2 + 1] = 'A' +value - 0x0a;
        }
    }

    dst[i * 2] = '\0';
}


uint64_t util_get_current_time_in_milli_seconds() {
    struct timeval now{};
    gettimeofday(&now, nullptr);
    uint64_t clock = now.tv_sec * 1000 + now.tv_usec / 1000;
    return clock;
}

uint64_t util_get_current_time_in_micro_seconds() {
    struct timeval now{};
    gettimeofday(&now, nullptr);
    uint64_t clock = now.tv_sec * 1000 * 1000 + now.tv_usec;
    return clock;
}

void remove_item_for_string_vector(std::vector<std::string>& vector, const std::string& item) {
    if (!vector.empty() && !item.empty()) {
        auto it = std::remove(vector.begin(), vector.end(), item);
        if (it != vector.end()) {
            vector.erase(it, vector.end());
        }
    }
}

char* util_base64_encode(
        char* out,
        int out_size,
        const uint8_t* input,
        int input_size
) {
    char* result = av_base64_encode(out, out_size, input, input_size);
    return result;
}

std::string join_string_from_queue(std::queue<std::string>& in_queue) {
    std::string ret_str;
    while (!in_queue.empty()) {
        ret_str += in_queue.front() + "_";
        in_queue.pop();
    }
    return ret_str;
}

std::string join_int_from_queue(std::queue<int>& in_queue) {
    std::string ret_str;
    while (!in_queue.empty()) {
        ret_str += std::to_string(in_queue.front()) + "_";
        in_queue.pop();
    }
    return ret_str;
}