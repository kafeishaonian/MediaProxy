//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_STLCOMMON_H
#define MEDIAPROXY_STLCOMMON_H

#include <vector>
#include <string>
#include <mutex>

using StringVector = std::vector<std::string>;
using UniqueLock = std::unique_lock<std::mutex>;

using StringMap = std::map<std::string, std::string>;
using Int64Map = std::map<std::string, int64_t>;
using StringList = std::list<std::string>;

#endif //MEDIAPROXY_STLCOMMON_H
