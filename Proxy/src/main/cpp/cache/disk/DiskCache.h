//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_DISKCACHE_H
#define MEDIAPROXY_DISKCACHE_H

//
//#include "ICacheFileChangeListener.h"
//#include "MCacheConfigFile.hpp"
//#include "MDiskCacheCommon.h"
//

#include <list>
#include <map>
#include <cstdio>
#include <string>
#include <shared_mutex>

#include "DiskCacheCommon.h"
#include "STLCommon.h"
#include "CacheInfo.h"
#include "FileCacheInterface.h"
#include "LRUCache.h"


class DiskCache {

public:

    DiskCache();

    ~DiskCache();

    static DiskCache *get_instance();

    void set_cache_path(const char *path);

    int64_t get_file_size(const char *file_path_url);

    void set_preload_size(const std::string &file_key, int64_t preload_size);

    int64_t get_preload_size(const std::string &file_path_url);

    void set_audio_duration(const std::string &file_key, int64_t duration);

    int64_t get_audio_duration(const std::string &file_key);

    void set_video_duration(const std::string &file_key, int64_t duration);

    int64_t get_video_duration(const std::string &file_key);

    int64_t get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    int64_t
    try_get_instance_parameter(const std::string &file_key, const std::string &parameter_key);

    int try_get_instance_parameter_with_map(const std::string &file_key, Int64Map &map);

    void set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                int64_t value);

    std::pair<uint64_t, uint64_t>
    query_remain_data_by_offset(const char *file_key, uint64_t offset);

    int query_empty_segment(const char *file_key,
                            std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector);

    int64_t
    read_data(const char *file_path_url, uint8_t *buffer, uint64_t offset, uint64_t read_size);

    int64_t writeData(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                      uint64_t read_size,
                      uint64_t file_size = 0);

    int64_t flush_config_file(const std::string &file_path_url);

    int64_t clear_cache();

    void clear_all_cache();

    int64_t clear_cache_with_key(const std::string &file_key);

    int get_cache_file_info_with_key(const std::string &fileKey, CacheFileInfo &fileInfo);

    int get_cache_complete_and_limit_size_file_list(std::vector<std::string>& file_keys);
};


#endif //MEDIAPROXY_DISKCACHE_H


//
////    int getCacheCompleteFileList(std::vector<std::string> &fileKeys);
//
//    int getCacheCompleteAndLimitSizeFileList(std::vector<std::string> &fileKeys);
//
//    void setCacheFileChangeListener(std::weak_ptr<ICacheFileChangeListener> listener);
//
//    bool isCacheComplete(const std::string& fileKey);
//
////    bool queryDataRangeExist(const std::string& fileKey, int64_t start, int64_t size);
//
//    std::string calcFileSign(const std::string& fileKey);
//
//    int64_t getCacheInfo( std::shared_ptr<MCacheInfo>& info );
//private:
//
//    std::string mCachePathString;
//
//    boost::timed_mutex mReadWriteLock;
//
//    StringList mFileRemovedList;
//
////    MFileCacheInterfaceMap mFilesMap;
//
//    MomoBase::MLRUCache<std::string, std::shared_ptr<MFileCacheInterface>> mFilesMap;
//
//    std::vector<CacheFileInfo> mTotalFileList;
//
//    int64_t mAllCachedSize;
//
//private:
//
//    // 通过文件key查找当前文件是否在FileMap中
//    // 如果没有，则创建后添加到map中
//    std::shared_ptr<MFileCacheInterface> buildConfigFileWithFileKey(const std::string &fileKey,
//                                                                    bool addToMap = true);
//
////    int getAllCacheSize();
//
//    int getAllCacheSizeV2();
//
//    int64_t checkIfRemoveCache(int64_t allCachedSize);
//
////    int64_t checkIfRemoveCacheV2(int64_t allCachedSize);
//
//
////    void removeExpireCache(int64_t &currentCachedSize, bool expiredFirstRemove);
//
////    void removeExpireCacheV2(int64_t &currentCachedSize, bool expiredFirstRemove);
//
////    void removeExpireCacheV3(int64_t &currentCachedSize);
//
////    void removeAllCache();
//
//    void removeFileNotificationWithFileKeys(const StringList &fileKeys);
//
//    std::weak_ptr<ICacheFileChangeListener> mCacheFileChangeListener;
//
//    bool isFileKeyExistInMap(const std::string &fileKey);
//
//    void removeAllCacheV2();
//};
//
//#endif /* MDisKCache_hpp */
