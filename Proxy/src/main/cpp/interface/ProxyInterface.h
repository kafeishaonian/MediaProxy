//
// Created by Hongmingwei on 2025/10/23.
//

#ifndef MEDIAPROXY_PROXYINTERFACE_H
#define MEDIAPROXY_PROXYINTERFACE_H

#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <boost/circular_buffer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <curl/curl.h>
#include <libavformat/avformat.h>


#include "PreloadManager.h"
#include "HttpServerAdvancedSessionRequestInfo.h"
#include "SpinMutex.h"
#include "LRUCache.h"


#include "SingletonShared.h"         // 共享单例
#include "URIParser2.hpp"            // URL解析
#include "HttpProxy.hpp"             // HTTP代理服务器
#include "GlobalConfig.hpp"          // 全局配置
#include "XlogAdpater.h"              // 日志系统

// 缓存管理
#include "CacheConfigFile.hpp"       // 缓存配置文件
#include "MemoryCache.hpp"           // 内存缓存
#include "CacheManager.hpp"          // 缓存管理器
#include "GlobalConstant.h"          // 全局常量

// 预加载
#include "PreloadManager2.hpp"       // 预加载管理器V2

// HTTP功能
#include "HttpClientCurlSync.hpp"    // 同步HTTP客户端(用于健康检查)
#include "HttpConnectPool.hpp"       // HTTP连接池
#include "CURLDownloader.hpp"        // CURL下载器

// 工具类
#include "Util.hpp"                  // 工具函数
#include "JsonParser.hpp"            // JSON解析
#include "JsonWriter.hpp"            // JSON写入
#include "rlCoded.hpp"               // URL编码
#include "SerialThread.h"            // 串行线程
#include "DNSRank.hpp"               // DNS优化


//可选
//#include "ProxyInfo.h"               // 代理信息（版本号等）
//#include "ProxyLoggerSetter.h"        // 日志设置
//#include "HttpClientManager.h"        // HTTP客户端管理器(用于异步获取Peer)
//#include "PreloadStatTaskInfo.h"     //任务统计信息


class ProxyInterface {

public:
    static std::shared_ptr<ProxyInterface> &get_instance();

    ProxyInterface();

    ~ProxyInterface();

    /** 初始化与生命周期 */
    // 初始化
    void init();

    // 反初始化
    void un_init();

    // 设置缓存路径
    int setupCache(const char *path);

    // 设置HTTP代理服务器
    int setupProxy(std::string configPath,
                   std::string address,
                   uint16_t port,
                   int serverThreadNumber);

    // 设置预加载管理器
    int setupPreloadManager(int threadNumber);

    void uninitPreloadManager();

    /** URL转换 */

    //转换播放地址为代理地址
    std::string switch_play_url(const char *play_url, const char *key = nullptr,
                                const char *real_host = nullptr);

    // 生成会话ID
    std::string generateSession();

    /** 服务器控制 */
    // 启动HTTP服务器
    int start();

    // 停止HTTP服务器
    int stop();

    // 重启HTTP服务器
    int restart();

    // 检查服务器运行状态
    bool isRunning();

    // 是否已启动
    bool isStarted();

    // 是否在监听
    bool isListening();

    // 获取监听状态
    bool getListenerStatus();

    /** 预加载任务管理 */
    // 添加按大小预加载的任务
    int addPreloadTaskWithRangeSize(...);

    // 添加按时长预加载的任务
    int addPreloadTaskWithPreloadDuration(...);

    // 限速
    int limitRate(int taskId, int64_t limit_rate);

    // 删除任务
    int removePreloadTaskWithId(int taskId);

    // 暂停任务
    int pausePreloadTaskWithId(int taskId);

    // 恢复任务
    int resumePreloadTaskWithId(int taskId);

    // 清除所有任务
    int clearAllPreloadTask();

    // 暂停所有任务
    int pauseAllPreloadTask();

    // 恢复所有任务
    int resumeAllPreloadTask();

    // 暂停T1T2任务
    int pauseT1T2PreloadTask();

    // 恢复T1T2任务
    int resumeT1T2PreloadTask();

    // 清除T1T2任务
    int clearT1T2PreloadTask();

    // 获取任务数量
    int getTaskCount(TASK_STATUS status);

    int64_t getAllDownloadedBytes();
    int64_t getCurAllDownloadRate();

    /**
     * 缓存管理
     */
    // 检查缓存是否存在
    int isCacheExist(const std::string &fileKey);

    // 获取缓存信息
    int64_t getCacheInfo(const std::string &fileKey, CacheInfoType type);

    // 检查缓存是否完成
    int isCacheCompleted(std::string fileKey);

    // 清理过期缓存
    int64_t clearCache();

    // 清空所有缓存
    void clearAllCache();

    // 清除指定缓存
    int64_t clearCacheWithKey(std::string fileKey);

    /**
     * 配置管理
     */
    // 设置代理配置
    int setProxyConfig(const std::string &configure);

    // 更新播放器预加载大小
    void updatePlayerPreloadSize(uint64_t size);

    /**
     * 结果查询
     */
    // 添加预加载结果
    void appendPreloadResult(shared_ptr <IPreloadResult> result);

    // 获取预加载结果
    shared_ptr <IPreloadResult> popPreloadResult();

    // 添加服务器结果
    void appendServerResult(...);

    // 获取服务器结果
    shared_ptr <MHttpServerSessionRequestInfo> popServerResult();

    // 转换为JSON
    std::string changeServerResutToJson(...);

    // 添加代理事件
    void appendProxyEvent(const std::string &event);

    // 获取代理事件
    std::string popProxyEvent();

    // 查询传输类型
    int queryCurrentPlayerTaskTransferType(...);

    // 获取CDN IP
    std::string getCDNIPAddress(...);

    /**
     * 播放器信息管理
     */
    // 更新播放器缓存时长
    void updatePlayerCacheDuration(...);

    // 更新点播播放器信息
    void updateVodPlayerInfo(...);

    // 清除点播播放器信息
    void clearVodPlayerInfo(...);

    // 获取点播播放器信息
    shared_ptr <VodPlayerInfo> getVodPlayerInfo(...);

    // 获取可播放时长
    long getPlayableDurationBySession(...);

    // 设置播放器信息
    void setPlayerInfo(...);

    // 获取播放器信息
    int64_t getPlayerInfo(...);

    /**
     * 工具方法
     */
    // 获取UUID
    std::string &getUUID();

    // 获取代理版本
    std::string getProxyVersion();

    // 获取应用ID
    std::string &getAppID();

    // 设置应用ID
    void setAppID(const std::string &appID);

    // 设置iOS版本
    void setIOSRelease(bool release);

    // 设置User Agent
    void setUserAgent(...);

    // 设置内部版本
    void setInternalVersion(...);

    /**
     * DNS优化
     */
    // 添加DNS结果
    void addDNSResultForDomain(...);

    // 获取域名IP
    std::string getIPWithDomain(...);

    // 清除DNS结果
    void clearAllDNSResult();


    /**
     *  调试功能
     */
    // Debug线程
    void debugPreloadThread(int intervalMs);

    // Debug任务日志
    void debugPreloadTaskLog(bool onTaskLog);

    // Debug会话日志
    void debugHttpSessionLog(bool onSessionLog);

    // 设置调试开关
    void setEnableDebug(int state);

    // 关闭日志
    void shutdownProxyLog();

private:

    enum ServerStatus {
        ServerStatus_Create,
        ServerStatus_Init,
        ServerStatus_Started,
        ServerStatus_Stoped,
        ServerStatus_UnInit,
    };

    std::atomic_int server_status_;


};


#endif //MEDIAPROXY_PROXYINTERFACE_H
