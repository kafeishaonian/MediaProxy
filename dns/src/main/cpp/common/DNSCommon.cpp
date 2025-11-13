//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSCommon.h"

#include <android/log.h>

namespace dns{

    LogLevel Logger::min_level_ = LogLevel::INFO;

    void Logger::log(dns::LogLevel level, const std::string &tag, const std::string &message) {
        if (level < min_level_) {
            return;
        }

        android_LogPriority priority;
        switch (level) {
            case LogLevel::DEBUG:
                priority = ANDROID_LOG_DEBUG;
                break;
            case LogLevel::INFO:
                priority = ANDROID_LOG_INFO;
                break;
            case LogLevel::WARN:
                priority = ANDROID_LOG_WARN;
                break;
            case LogLevel::ERROR:
                priority = ANDROID_LOG_ERROR;
                break;
            default:
                priority = ANDROID_LOG_INFO;
                break;
        }
        __android_log_print(priority, tag.c_str(), "%s", message.c_str());
    }

    void Logger::setLevel(dns::LogLevel min_level) {
        min_level_ = min_level;
    }
}
