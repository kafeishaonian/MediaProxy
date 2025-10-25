//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_HTTPCLIENT_H
#define MEDIAPROXY_HTTPCLIENT_H

#include <iostream>
#include <string>
#include <memory>

#include <URIParser.h>
#include <STLCommon.h>

enum class HttpClientStatus : int {
    ResolveFail = -1,
    ConnectFail = -2,
    CloseFail = -3,
    URLError = -4,
    NotConnect = -5,
    ReadError = -6,
    SeekError = -7,
    ResponseError = -8,
    OK = 0,
};

class HttpClient {

};


#endif //MEDIAPROXY_HTTPCLIENT_H


//class MMemoryCache;
//
//class HttpResponseInfo
//{
//public:
//    std::string mContentType;
//    int mHttpCode;
//    int64_t mFileSize;
//};
//
//class IMHttpClient
//{
//public:
//
//    IMHttpClient(std::string urlString);
//
//    virtual ~IMHttpClient();
//
//    /** 建立连接
//    * @return < 0 表示连接失败；>= 0 连接成功
//    */
//    virtual int connect(int64_t offset);
//
//    virtual int connect(int64_t offset, int64_t requestEndOffset);
//
//    /** 读取数据
//     *  @buffer 读取的缓存
//     *  @size 要读取的数据大小
//     *  @return >= 0, 读取的数据大小; < 0 错误
//     */
//    virtual int64_t readData(uint8_t *buffer, int64_t size);
//
//    using DataFunction = std::function<int(uint8_t* data, int size, HttpResponseInfo& info)>;
//
//    virtual int readData(DataFunction callback);
//
//    virtual int64_t seek();
//
//    /** 关闭连接
//    */
//    virtual int close();
//
//    /** 发送 HEAD 请求, 用于以下功能：
//     *  1)  获取视频文件大小
//     *  2） 获取视频文件是否存在
//     *  @return 返回HTTP 状态码
//     */
//    virtual int sendHeadRequest();
//
//    // 发送header request, 通过读取 content-length 获取文件大小
//    virtual uint64_t getFileSize();
//
//    virtual int isAbortRequest();
//
//    virtual void requestAbort();
//
//
//    // 设置URL
//    void setURL(const char *url);
//
//    // 设置header
//    void setHeaderHost(const char *header);
//
//    // 设置文件偏移
//    int64_t setOffset(int64_t offset);
//
//    //获取文件偏移
//    int64_t getOffset();
//
//    // 设置user agent
//    void setUserAgent(const char *userAgent);
//
//    // 设置memory cache
//    void setMemoryCache(std::shared_ptr<MMemoryCache> cache);
//
//    void setFileSize(int64_t fileSize);
//
//    // 设置timeout
//    void setTimeoutInMilliSecond(int timeout);
//
//    int getTimeoutInSecond();
//
//    void setReconnectTimes(int times);
//
//    void setReconnectSleepTimeInMilliSecond(int time);
//
//    int64_t getDnsTimestampInMs();
//
//    std::string getIpAddr();
//
//    int64_t getHttpHeaderTime();
//
//    int64_t getHttpBodyTime();
//
//    int getHttpCode();
//
//    int64_t getTcpConnectTime();
//
//    StringVector& getResolvedAddress();
//
//    std::string& getVendor();
//
//    std::string& getContentType();
//
//    int64_t getTotalDownloadTime();
//
//    int getHttpClientMethod();
//
//    void setTaskId(int taskId);
//
//    int64_t getDownloadRate();
//
//    std::string& getLocation();
//
//protected:
//    // 文件大小： 初始值， 0 表示未获取到
//    int64_t mFileSize;
//
//    // 文件偏移量
//    uint64_t mOffset;
//
//    // 文件结束EndOffset
//    // 下载的大小[mOffset, mRequestEndOffset)
//    int64_t mRequestEndOffset;
//
//    // 文件资源URI
//    std::string mURLString;
//
//    // 主机地址
//    std::string mHostURLString;
//
//    // Http path
//    std::string mTarget;
//
//    // 针对ip下载文件
//    // cdn 要求携带 "Host : img.momocdn.com"
//    std::string mHeaderHostString;
//
//    // http user agent
//    std::string mUserAgent;
//
//    // memory cache
//    std::shared_ptr<MMemoryCache> mMemoryCache;
//
//    // uri 解析
//    MURIParser2 mURIParser;
//
//    // 超时时间
//    int mTimeoutInMilliSecond;
//
//    // 重连次数
//    int mReconnectTimes;
//
//    // 重连sleep
//    int mReconnectSleepTime;
//
//    std::string mIpAddr;
//
//    int64_t mDnsUseTimeInMs;
//
//    int64_t mHttpHeaderTime;
//
//    int64_t mHttpBodyTime;
//
//    int mHttpCode;
//
//    int64_t mTcpConnectTime;
//
//    StringVector mReslovedAdress;
//
//    std::string mVendor;
//
//    std::string mContentType;
//
//    // 总下载时间，针对curl
//    int64_t mTotalDownloadTime;
//
//    const char* TAG;
//
//    int mHttpClientMethod;
//
//    int mTaskId;
//
//    bool mDisableIPv6;
//
//    int64_t mDownloadRate;
//
//    std::string mLocation;
//};
//
//#endif /* IMHttpClient_hpp */
