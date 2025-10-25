//
// Created by Hongmingwei on 2025/10/20.
//

#include "BaseUtil.h"

namespace proxy {
    bool is_number(const std::string& string) {
        if (string.empty()) {
            return false;
        }

        auto it = std::find_if(string.begin(), string.end(), [](char c) {
            return !std::isdigit(c);
        });

        if (it != string.end()) {
            return false;
        } else {
            return true;
        }
    }
}