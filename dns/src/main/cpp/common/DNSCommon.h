//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSCOMMON_H
#define MEDIAPROXY_DNSCOMMON_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace dns {

    //DNS服务器类型
    enum class DNSServerType {
        SYSTEM = 0,      // 系统DNS
        HTTP_DNS = 1,    // HTTP DNS
        LOCAL = 2        // 本地缓存
    };

    //DNS任务类型
    enum class DNSServerTaskType {
        RESOLVE_HOST = 0,    // 解析主机
        SPEED_CHECK = 1,     // 速度检测
        CACHE_UPDATE = 2     // 缓存更新
    };

    //网络状态
    enum class DNSAppNetState {
        UNKNOWN = 0,
        WIFI = 1,
        MOBILE = 2,
        NONE = 3
    };

    //HTTP请求方法
    enum class RequestMethod {
        GET = 0,
        POST = 1,
        PUT = 2,
        DELETE = 3
    };

    //日志级别
    enum class LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };

    //常量定义
    namespace Constants {
        constexpr int DEFAULT_TIMEOUT_MS = 5000;        //超时时间
        constexpr int DEFAULT_RETRY_COUNT = 3;          //重试次数
        constexpr int DEFAULT_THREAD_COUNT = 4;         //线程数
        constexpr int DEFAULT_QUEUE_SIZE = 1000;        //任务队列容量
        constexpr int DEFAULT_CACHE_SIZE = 100;         //缓存容量
        constexpr int DNS_PORT = 53;                    //DNS端口号
        constexpr int HTTP_PORT = 80;                   //HTTP端口号
        constexpr int HTTPS_PORT = 443;                 //HTTPS端口号
    }

    class Logger {
    public:
        static void log(LogLevel level, const std::string& tag, const std::string& message);
        static void setLevel(LogLevel min_level);

    private:
        static LogLevel min_level_;
    };

}


#endif //MEDIAPROXY_DNSCOMMON_H
