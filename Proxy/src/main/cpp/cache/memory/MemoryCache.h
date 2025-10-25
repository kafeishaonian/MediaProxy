//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_MEMORYCACHE_H
#define MEDIAPROXY_MEMORYCACHE_H


class MemoryCache {

};


#endif //MEDIAPROXY_MEMORYCACHE_H


//
//#include <iostream>
//#include <map>
//#include <boost/thread/shared_mutex.hpp>
//
//#include "MMemoryMediaCache.hpp"
//#include "MCacheInfo.hpp"
//
//class MMemoryCache
//{
//public:
//    MMemoryCache();
//
//    ~MMemoryCache();
//
//    static MMemoryCache *getInstance();
//
//    /** 获取文件大小
//     * @param filePathURL 文件的url path，或者用户指定文件key
//     * @return 返回 -1 表示文件不存在
//     */
//    int64_t getFileSize(const std::string& fileKey);
//
//    /** 读取视频文件
//     * @param filePathURL 文件的url path，或者用户指定文件key
//     * @param buffer , 读取缓存
//     * @param offset 文件偏移
//     * @param bufferSize 读取大小
//     * @return 返回 -1表示文件不存在；
//     */
//    int64_t readData(const char *filePathURL,
//                     uint8_t *buffer,
//                     uint64_t offset,
//                     uint64_t bufferSize);
//
//    /** 写视频文件，
//     * @param filePathURL 文件的url path，或者用户指定文件key
//     * @param buffer , 缓存大小
//     * @param offset 文件偏移
//     * @param bufferSize 缓存大小
//     * @param fileSize 文件大小
//     * @return 返回 -1表示写文件失败；>= 0，写入的文件大小
//     */
//    int64_t writeData(const char *filePathURL,
//                      uint8_t *buffer,
//                      int64_t offset,
//                      int64_t bufferSize,
//                      int64_t fileSize = 0);
//
//    /** 显示内存数据
//     */
//    void dumpData(const char *filePathURL);
//
//    void dumpAllData();
//
//    void serializeAll();
//
//    void serialize();
//
//    void serializeExpiredCache();
//
//    /** 查找从offset开始 连续的剩余多少数据可用
//     * @param fileKey 文件的key
//     * @param offset 文件中的offset
//     * @return first:剩余可用字节数  second:可填充数据空间大小，如后续无可用数据此值为0
//     */
//    std::pair<uint64_t, uint64_t> queryRemainDataByOffset(const char *fileKey,
//                                                          uint64_t offset );
//
//    int queryEmptySegment(const char *fileKey,
//                          std::vector<std::pair<uint64_t, uint64_t>> &emptySegmentVector);
//
//    int64_t getInstanceParameter(const std::string &fileKey, const std::string& parameterKey);
//
//    void getInstanceParameterWithMap(const std::string &fileKey, Int64Map& map);
//
//    void setInstanceParameter(const std::string &fileKey, const std::string& parameterKey, int64_t value);
//
//    int64_t calculateMemoryUsage();
//
//    int64_t getMemoryUsage();
//
//    void dropAll();
//
//    bool isCacheComplete(const std::string& fileKey);
//
//    bool queryDataRangeExist(const std::string& fileKey,
//                             int64_t start,
//                             int64_t size);
//
//    int64_t getCacheInfo( std::shared_ptr<MCacheInfo>& info );
//private:
//
//    boost::shared_mutex mReadWriteLock;
//
//    std::map<std::string, std::shared_ptr<MMemoryMediaCache>> mMemoryMediaLists;
//
//    std::shared_ptr<MMemoryMediaCache> findMemoryMediaCache(const std::string& fileKey);
//
//    std::shared_ptr<MMemoryMediaCache> findOrCreateMemoryMediaCache(const std::string& fileKey);
//
//    int64_t mMemoryUsage;
//
//    const char *TAG;
//};
//
//#endif /* MMemoryCache_hpp */
