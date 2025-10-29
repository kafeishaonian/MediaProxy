//
// Created by Hongmingwei on 2025/10/24.
//

//#include "MCacheConfigFile.hpp"
//#include "MCacheConfigFileV2.hpp"
//#include "MJsonParser.hpp"
//#include "MLogTAG.h"
//#include "MP2PConfig.hpp"
//#include "MStatisTag.hpp"
//#include "MStatisUtil.hpp"
//#include "XlogAdpater.h"

#include "DiskCache.h"

#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <ftw.h>

#include "Singleton.h"
#include "ThreadUtil.h"
#include "Util.h"
#include "ProxyInterface.h"
#include "GlobalConfig.h"
#include "GlobalConstant.h"
#include "DiskCacheUtil.h"
#include "FileCacheInterface.h"


DiskCache *DiskCache::get_instance() {
    return Singleton<DiskCache>::get_instance();
}


DiskCache::DiskCache() : files_map_(20) {}

DiskCache::~DiskCache() {}

void DiskCache::set_cache_path(const char *path) {
    if (path) {
        cache_path_string_ = std::string(path);
    }
}

std::pair<uint64_t, uint64_t>
DiskCache::query_remain_data_by_offset(const char *file_key, uint64_t offset) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }

    if (config_file) {
        return config_file->query_remain_data_by_offset(offset);
    }
    return std::make_pair(0, 0);
}

int DiskCache::query_empty_segment(const char *file_key,
                                   std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }
    if (config_file) {
        return config_file->query_empty_segment(empty_segment_vector);
    }
    return -2;
}

int64_t DiskCache::read_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                             uint64_t size) {
    if (!file_path_url) {
        return 0;
    }

    if (!buffer) {
        return 0;
    }

    int64_t read_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }
    if (config_file) {
        int exist = config_file->is_config_file_exist();
        if (exist < 0) {
            return 0;
        }
        int result = config_file->parse();
        if (result < 0) {
            proxy::TimedLock lock(read_write_lock_);
            files_map_.erase(file_path_url);
        }
        read_size = config_file->read_data(buffer, offset, size);
    } else {
        read_size = 0;
    }
    return read_size;
}

int64_t DiskCache::write_data(const char *file_path_url, uint8_t *buffer, uint64_t offset,
                             uint64_t size, uint64_t file_size) {
    if (file_path_url == nullptr) {
        return -1;
    }

    if (buffer == nullptr) {
        return -1;
    }

    int64_t write_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }

    if (config_file) {
        config_file->set_file_size(file_size);
        write_size = config_file->write_data(buffer, offset, size);
    } else {
        write_size = -1;
    }
    return write_size;
}

void
DiskCache::set_instance_parameter(const std::string &file_key, const std::string &parameter_key,
                                  int64_t value) {
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_key);
    }
    if (config_file) {
        config_file->set_instance_parameter(parameter_key, value);
    }
}

int64_t DiskCache::flush_config_file(const std::string &file_path_url) {
    if (file_path_url.empty()){
        return -1;
    }

    int64_t write_size = 0;
    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    {
        proxy::TimedLock lock(read_write_lock_);
        config_file = build_config_file_with_file_key(file_path_url);
    }

    if (config_file) {
        write_size = config_file->flush_config_file();
        if (config_file->is_cache_complete() && config_file->get_file_size() > GlobalConfig::get_instance()->get_min_file_size_upload_tracker()) {
            auto listener = cache_file_change_listener_.lock();
            if (listener) {
                listener->cache_added(config_file->get_file_key());
            }
        }
    }
    return write_size;
}


//// TODO:需要加锁，代码未测试
//int64_t MDiskCache::getFileSize(const char *filePathURL) {
//    int64_t fileSize = -1;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(filePathURL);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "isConfigFileExist not exist");
//        return 0;
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(filePathURL);
//    }
//
//    if (configFile) {
//        fileSize = configFile->getFileSize();
//    }
//
//    return fileSize;
//}
//
//void MDiskCache::setPreloadSize(const std::string &fileKey, int64_t preloadSize) {
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return;
//    }
//
//    configFile->setPreloadSize(preloadSize);
//}
//
//int64_t MDiskCache::getPreloadSize(const std::string &filePathURL) {
//    int64_t preloadSize = -1;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(filePathURL);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "isConfigFileExist = %d",
//                     exit);
//        return -1;
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(filePathURL);
//    }
//    if (configFile) {
//        preloadSize = configFile->getPreloadSize();
//    }
//    return preloadSize;
//}
//
//void MDiskCache::setAudioDuration(const std::string &fileKey, int64_t duration) {
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "isConfigFileExist = %d",
//                     exit);
//        return;
//    }
//
//    if (configFile) {
//        configFile->setPreloadAudioDuration(duration);
//    }
//}
//
//int64_t MDiskCache::getAudioDuration(const std::string &fileKey) {
//    int64_t duration = 0;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return -1;
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(fileKey);
//    }
//    if (configFile) {
//        duration = configFile->getPreloadAudioDuration();
//    }
//    return duration;
//}
//
//void MDiskCache::setVideoDuration(const std::string &fileKey, int64_t duration) {
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "isConfigFileExist = %d",
//                     exit);
//        return;
//    }
//
//    if (configFile) {
//        configFile->setPreloadVideoDuration(duration);
//    }
//}
//
//int64_t MDiskCache::getVideoDuration(const std::string &fileKey) {
//    int64_t duration = 0;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return -1;
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(fileKey);
//    }
//    if (configFile) {
//        duration = configFile->getPreloadVideoDuration();
//    }
//    return duration;
//}
//
//int64_t MDiskCache::getInstanceParameter(const std::string &fileKey,
//                                         const std::string &parameterKey) {
//    int64_t value = 0;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return -1;
//    }
//
//    if (configFile->parse() < 0) {
//        return -1;
//    }
//
//    if (configFile) {
//        value = configFile->getInstanceParameter(parameterKey);
//    }
//    return value;
//}
//
//int64_t MDiskCache::tryGetInstanceParameter(const std::string &fileKey,
//                                            const std::string &parameterKey) {
//    int64_t value = 0;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        int timeout = MGlobalConfig::getInstance()->getCacheLockTimeoutInMS();
//        auto lock =
//                MomoBase::make_unique_lock(mReadWriteLock, boost::posix_time::milliseconds(timeout));
//        if (lock) {
////            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "timed lock success");
//            configFile = buildConfigFileWithFileKey(fileKey);
//        } else {
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "timed lock timeout");
//        }
//    }
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->tryIsConfigFileExist();
//    if (exist < 0) {
//        return -1;
//    }
//
//    if (configFile->tryParse() < 0) {
//        return -1;
//    }
//
//    if (configFile) {
//        value = configFile->tryGetInstanceParameter(parameterKey);
//    }
//    return value;
//}
//
//int MDiskCache::tryGetInstanceParameterWithMap(const std::string &fileKey, Int64Map& map)
//{
//    int result = 0;
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        int timeout = MGlobalConfig::getInstance()->getCacheLockTimeoutInMS();
//        auto lock =
//                MomoBase::make_unique_lock(mReadWriteLock, boost::posix_time::milliseconds(timeout));
//        if (lock) {
//            configFile = buildConfigFileWithFileKey(fileKey);
//        } else {
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "timed lock timeout");
//        }
//    }
//    if (!configFile) {
//        return -1;
//    }
//
//    int exist = configFile->tryIsConfigFileExist();
//    if (exist < 0) {
//        return -1;
//    }
//
//    if (configFile->tryParse() < 0) {
//        return -1;
//    }
//
//    if (configFile) {
//        result = configFile->tryGetInstanceParameterWithMap(map);
//    }
//    return result;
//}
//
//
//// FIXME: 清除缓存最好不要播放视频
//int64_t MDiskCache::clearCache() {
//    MTimedLock lock(mReadWriteLock);
//    mFileRemovedList.clear();
//    mTotalFileList.clear();
//    mFilesMap.clear();
//    int fodersCount = 0;
//    fodersCount = getAllCacheSizeV2();
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "foderCount = %d, allCachedSize = %lld", fodersCount, mAllCachedSize);
//
//    //    if (isNeedClean(mAllCachedSize)) {
//    //        mFilesMap.clear();
//    //    }
//
////    int64_t currentCacheSize = 0;
////    if (MGlobalConfig::getInstance()->getCacheClearMethod() == CacheClearUseShareNum) {
////        currentCacheSize = checkIfRemoveCacheV2(mAllCachedSize);
////    }
////    else {
////        currentCacheSize = checkIfRemoveCache(mAllCachedSize);
////    }
//
//    int64_t currentCacheSize = 0;
//    currentCacheSize = checkIfRemoveCache(mAllCachedSize);
//
//    for (auto &fileKey : mFileRemovedList) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "remove full cached file = %s", fileKey.c_str());
//    }
//    removeFileNotificationWithFileKeys(mFileRemovedList);
//    return currentCacheSize;
//}
//
//void MDiskCache::clearAllCache() {
//    MTimedLock lock(mReadWriteLock);
//    mFileRemovedList.clear();
//    mFilesMap.clear();
//    mAllCachedSize = 0;
//    int fodersCount = 0;
//    fodersCount = MDiskCacheUtil::getAllCachedFileKeyAndTotalSize(mCachePathString, mAllCachedSize, mFileRemovedList);
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "foderCount = %d, allCachedSize = %lld", fodersCount, mAllCachedSize);
//    removeAllCacheV2();
//
//    for (auto &fileKey : mFileRemovedList) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "remove full cached file = %s", fileKey.c_str());
//    }
//    removeFileNotificationWithFileKeys(mFileRemovedList);
//    //    mFilesMap.clear();
//    //    removeAllCache();
//}
//
//int64_t MDiskCache::clearCacheWithKey(const std::string &fileKey) {
//    int64_t result = -1;
//    if (fileKey.empty()) {
//        return -1;
//    }
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//        mFilesMap.erase(fileKey);
//    }
//
//    if (!configFile) {
//        return -1;
//    }
//
//    result = configFile->remove();
////    int64_t result = removeCacheWithKey(fileKey);
//    return result;
//}
//
//bool MDiskCache::isCacheComplete(const std::string& fileKey)
//{
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return false;
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return false;
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(fileKey);
//        return false;
//    }
//    return configFile->isCacheComplete();
//}
//
////bool MDiskCache::queryDataRangeExist(const std::string& fileKey, int64_t start, int64_t size)
////{
////    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
////    {
////        MTimedLock lock(mReadWriteLock);
////        configFile = buildConfigFileWithFileKey(fileKey);
////    }
////
////    if (!configFile) {
////        return false;
////    }
////
////    int exist = configFile->isConfigFileExist();
////    if (exist < 0) {
////        return false;
////    }
////
////    int result = configFile->parse();
////    if (result < 0) {
////        MTimedLock lock(mReadWriteLock);
////        mFilesMap.erase(fileKey);
////        return false;
////    }
////    return configFile->queryDataRangeExist(start, size);
////}
//
//std::string MDiskCache::calcFileSign(const std::string& fileKey)
//{
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//
//    if (!configFile) {
//        return std::string("");
//    }
//
//    int exist = configFile->isConfigFileExist();
//    if (exist < 0) {
//        return std::string("");
//    }
//
//    int result = configFile->parse();
//    if (result < 0) {
//        MTimedLock lock(mReadWriteLock);
//        mFilesMap.erase(fileKey);
//        return std::string("");
//    }
//    return configFile->calcFileSign();
//}
//
//int64_t MDiskCache::getCacheInfo( std::shared_ptr<MCacheInfo>& info )
//{
//    Int64Map parameterMap;
//    parameterMap[kCachedSizeKey] = -1;
//    parameterMap[kFileSizeKey] = -1;
//    parameterMap[kPreloadSizeKey] = -1;
//    parameterMap[kPreloadAudioDurationKey] = -1;
//    parameterMap[kPreloadVideoDurationKey] = -1;
//
//
//    tryGetInstanceParameterWithMap(info->mFileKey, parameterMap);
//
//    int64_t preloadSize = parameterMap[kPreloadSizeKey];
//    int64_t cacheSize = parameterMap[kCachedSizeKey];
//    info->mFileSize = parameterMap[kFileSizeKey];
//    if( info->mFileSize > 0 && cacheSize > 0 && cacheSize >= info->mFileSize){
//        info->mIsComplete = true;
//    }
//    info->mMinPreloadDuration = parameterMap[kPreloadAudioDurationKey] <= parameterMap[kPreloadVideoDurationKey] ?
//                                parameterMap[kPreloadAudioDurationKey] : parameterMap[kPreloadVideoDurationKey];
//
//    if (preloadSize > 0) {
//        uint64_t readCount = preloadSize > MCacheInfo::ECacheSize?
//                             MCacheInfo::ECacheSize : preloadSize;
//        info->mCacheData.resize(readCount);
//        int64_t realRead = readData(info->mFileKey.c_str(),
//                                    info->mCacheData.data(), info->mOffset, readCount);
//        if(realRead > 0){
//            info->mCacheDataCount = realRead;
//        }
//    }
//
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "fileKey:%s,fileSize:%lld,preloadSize:%lld,"
//                 "cacheSize:%lld,read;%lld,offset:%lld,a:%lld,v:%lld",
//                 info->mFileKey.c_str(),
//                 info->mFileSize,
//                 preloadSize,
//                 cacheSize,
//                 info->mCacheDataCount,
//                 info->mOffset,
//                 parameterMap[kPreloadAudioDurationKey],
//                 parameterMap[kPreloadVideoDurationKey]);
//
//    return info->mCacheDataCount;
//}
//
//#pragma mark---- Private Function ----
//
////int MDiskCache::getAllCacheSize() {
////    char* cachePath = (char*)mCachePathString.c_str();
////    DIR *dir = opendir(cachePath);
////    char fileName[2048];
////    struct dirent *inFile;
////    int filesCount = 0;
////    mAllCachedSize = 0;
////    if (dir) {
////        while ((inFile = readdir(dir))) {
////            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
////                strcmp(inFile->d_name, "..")) {
////                char* cachePath = (char *)mCachePathString.c_str();
////                sprintf(fileName, "%s/%s/config.json", cachePath, inFile->d_name);
////                int64_t cachedSize = MCacheConfigFile::getCachedSize(fileName);
////                if (cachedSize >= 0) {
////                    mAllCachedSize += cachedSize;
////                    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                                 "config file name = %s, and video size = %lld", fileName,
////                                 cachedSize);
////                }
////                filesCount++;
////            }
////        }
////        closedir(dir);
////    }
////    return filesCount;
////}
//
//int64_t MDiskCache::checkIfRemoveCache(int64_t allCachedSize) {
//    int64_t currentCachedSize = allCachedSize;
//    if (MDiskCacheUtil::isNeedClean(allCachedSize)) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "cache need clean");
//        MDiskCacheUtil::removeExpireCache(mCachePathString, mTotalFileList, mFileRemovedList, currentCachedSize, true);
//    }
//    if (MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "cache still need clean after remove expired files");
//        MDiskCacheUtil::removeExpireCache(mCachePathString, mTotalFileList, mFileRemovedList, currentCachedSize, false);
//    }
//
//    return currentCachedSize;
//}
//
////int64_t MDiskCache::checkIfRemoveCacheV2(int64_t allCachedSize) {
////    int64_t currentCachedSize = allCachedSize;
////    if (MDiskCacheUtil::isNeedClean(allCachedSize)) {
////        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "cache need clean");
////        MDiskCacheUtil::removeExpireCacheV3(mFilesMap, mFileRemovedList, currentCachedSize);
////    }
////
////    return currentCachedSize;
////}
//
//
////void MDiskCache::removeExpireCache(int64_t &currentCachedSize, bool expiredFirstRemove) {
////    char* cachePath = (char *)mCachePathString.c_str();
////    DIR *dir = opendir(cachePath);
////    char fileName[2048];
////    struct dirent *inFile;
////    if (dir) {
////        int64_t fileSize = 0;
////        int64_t fileLastAccessTime;
////
////        while ((inFile = readdir(dir))) {
////            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
////                strcmp(inFile->d_name, "..")) {
////                sprintf(fileName, "%s/%s/config.json", cachePath, inFile->d_name);
////                fileSize = MCacheConfigFile::getCachedSize(fileName);
////
////                if (expiredFirstRemove) {
////                    fileLastAccessTime = MCacheConfigFile::getAccessTime(fileName);
////                    if (isExpire(fileLastAccessTime, 1)) {
////                        removeFilesAtPath(inFile->d_name);
////                        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                                     "Remove dir = %s/%s\n", cachePath, inFile->d_name);
////                        currentCachedSize -= fileSize;
////                    }
////                } else {
////                    removeFilesAtPath(inFile->d_name);
////                    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                                 "Remove dir = %s/%s\n", cachePath, inFile->d_name);
////                    currentCachedSize -= fileSize;
////                }
////            }
////            if (!isNeedClean(currentCachedSize)) {
////                __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                             "Finish clean data, and exit， current size = %lld, limit size = %lld",
////                             currentCachedSize);
////                break;
////            }
////        }
////        closedir(dir);
////    }
////}
//
////void MDiskCache::removeAllCache() {
////    char* cachePath = (char *)mCachePathString.c_str();
////    DIR *dir = opendir(cachePath);
////    struct dirent *inFile;
////    if (dir) {
////
////        while ((inFile = readdir(dir))) {
////            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
////                strcmp(inFile->d_name, "..")) {
////
////                removeFilesAtPath(inFile->d_name);
////            }
////        }
////
////        closedir(dir);
////    }
////}
//

bool DiskCache::is_file_key_exist_in_map(const std::string &file_key) {
    if (files_map_.exists(file_key)) {
        return true;
    } else {
        return false;
    }
}

//#pragma mark---- File Remove V2 ----
//
//int MDiskCache::getAllCacheSizeV2() {
//    char* cachePath = (char *)mCachePathString.c_str();
//    DIR *dir = opendir(cachePath);
//    struct dirent *inFile;
//    int filesCount = 0;
//    mAllCachedSize = 0;
//    if (dir) {
//        while ((inFile = readdir(dir))) {
//            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
//                strcmp(inFile->d_name, "..")) {
//
//                CacheFileInfo cacheFileInfo;
//                int result = MDiskCacheUtil::getCacheFileInfoFromDir(mCachePathString, inFile->d_name, cacheFileInfo);
//                if (result < 0) {
//                    continue;
//                }
//                std::string fileKey = cacheFileInfo.fileKey;
//                if (fileKey.empty()) {
//                    continue;
//                }
//
//                mTotalFileList.push_back(cacheFileInfo);
//                int64_t cachedSize = (int64_t)cacheFileInfo.cacheSize;
//                if (cachedSize >= 0) {
//                    mAllCachedSize += cachedSize;
//
//                    if (filesCount < 20) {
//                        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                                     "file key = %s, and video size = %lld", fileKey.c_str(),
//                                     cachedSize);
//                    }
//                }
//
//                filesCount++;
//            }
//        }
//        closedir(dir);
//    }
//    return filesCount;
//}
//
//void MDiskCache::removeAllCacheV2() {
//    MDiskCacheUtil::removeAllFileInAbsolutePath(mCachePathString);
////    for (auto it = mFilesMap.begin(); it != mFilesMap.end();) {
////        std::shared_ptr<MFileCacheInterface> configFile = it->second;
////
////        if (configFile->isCacheComplete()) {
////            mFileRemovedList.push_back(configFile->getFileKey());
////        }
////        configFile->remove();
////        mFilesMap.erase(it++);
////    }
//}
//
//#pragma mark------  tracker support public funcation -------
//
//void MDiskCache::setCacheFileChangeListener(std::weak_ptr<ICacheFileChangeListener> listener) {
//    MTimedLock lock(mReadWriteLock);
//    mCacheFileChangeListener = listener;
//}
//
////int MDiskCache::getCacheCompleteFileList(std::vector<std::string> &fileKeys) {
////    MTimedLock lock(mReadWriteLock);
////    char* cachePath = (char *)mCachePathString.c_str();
////    DIR *dir = opendir(cachePath);
////    struct dirent *inFile;
////    int filesCount = 0;
////    if (dir) {
////        while ((inFile = readdir(dir))) {
////            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
////                strcmp(inFile->d_name, "..")) {
////
////                std::string fileKey = getFileKeyFromDir(inFile->d_name);
////                if (fileKey.empty()) {
////                    continue;
////                }
////
////                std::shared_ptr<MFileCacheInterface> configFile = buildConfigFileWithFileKey(fileKey);
////                if (!configFile) {
////                    continue;
////                }
////
////                if (configFile->parse() != 0) {
////                    continue;
////                }
////
////                if (configFile->isCacheComplete()) {
////                    fileKeys.push_back(configFile->getFileKey());
////                }
////
////                if (filesCount < 10) { // 减少日志输出
////                    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                                 "complete file:%s fsize:%llu csize:%llu",
////                                 configFile->getFileKey().c_str(), configFile->getFileSize(),
////                                 configFile->getCacheSize());
////                }
////                filesCount++;
////                if (filesCount >= MP2PConfig::getInstance()->getUpdateToTrackerFileLimit()) {
////                    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                                 "file cout exceed limit");
////                    break;
////                }
////            }
////        }
////        closedir(dir);
////    }
////    return filesCount;
////}
//
//int MDiskCache::getCacheCompleteAndLimitSizeFileList(std::vector<std::string> &fileKeys) {
//    MTimedLock lock(mReadWriteLock);
//    char* cachePath = (char *)mCachePathString.c_str();
//    DIR *dir = opendir(cachePath);
//    struct dirent *inFile;
//    int filesCount = 0;
//
//    uint64_t maxFileLen = 0;
//    int upFileCount = 0;
//
//    std::vector<PairCacheConfigFile> filesVector;
//    if (dir) {
//        while ((inFile = readdir(dir))) {
//            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
//                strcmp(inFile->d_name, "..")) {
//
//                CacheFileInfo cacheFileInfo;
//                int result = MDiskCacheUtil::getCacheFileInfoFromDir(mCachePathString, inFile->d_name, cacheFileInfo);
////                __MDLOGD_TAG("Disk", "disk:%s, getCacheFileInfoFromDir:%d", inFile->d_name, result);
//                if (result < 0) {
//                    continue;
//                }
//                std::string fileKey = cacheFileInfo.fileKey;
//                if (fileKey.empty()) {
//                    continue;
//                }
//
//                std::shared_ptr<MFileCacheInterface> configFile = buildConfigFileWithFileKey(fileKey);
//                if (!configFile) {
//                    continue;
//                }
//
//                if (configFile->parse() != 0) {
//                    continue;
//                }
//
//                StringList invalidFileList;
//                if (!MDiskCacheUtil::isValidFile(configFile, 1, invalidFileList, false)) {
//                    continue;
//                } else {
//                    filesVector.push_back(std::make_pair(configFile->getFileKey(), configFile));
//                }
//
//                filesCount++;
//            }
//        }
//
//        closedir(dir);
//
//        if (filesCount != 0) {
//            if (filesCount >= MP2PConfig::getInstance()->getUpdateToTrackerFileLimit()) {
//                if (!filesVector.empty()) {
//                    MDiskCacheUtil::sortByShareNum(filesVector, false);
//                }
//            }
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "upTrackerValidFile filesCount:%d,limit:%d",
//                         filesCount, MP2PConfig::getInstance()->getUpdateToTrackerFileLimit());
//            int limit = MP2PConfig::getInstance()->getUpdateToTrackerFileLimit();
//            for (auto &it : filesVector) {
//                if (it.second->getFileSize() > maxFileLen) {
//                    maxFileLen = it.second->getFileSize();
//                }
//
////                __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
////                             "upTrackerValidFile file:%s fsize:%llu,ShareNum:%lld,AccessTime:%llu,maxFileLen:%llu",
////                             it.first.c_str(),
////                             it.second->getFileSize(),
////                             it.second->getShareNum(),
////                             it.second->getAccessTime(),
////                             maxFileLen);
//
//                fileKeys.push_back(it.first);
//                limit--;
//                upFileCount++;
//                if (limit <= 0) {
//                    break;
//                }
//            }
//        }
//        //打点上传到tracker的文件信息
//        if (MP2PConfig::getInstance()->isUpTrackerFileInfoLog()) {
//
//            std::string id = momo::to_string(MUtilGetCurrentTimeInMilliSeconds());
//
//            MStatisUtil::getInstance()->setValue(MStatisTag::TagTrackerFile,
//                                                 TagTrackerFileKeys::keyTrackerFileMax,
//                                                 id,
//                                                 maxFileLen);
//
//            MStatisUtil::getInstance()->setValue(MStatisTag::TagTrackerFile,
//                                                 TagTrackerFileKeys::keyTrackerFileCount,
//                                                 id,
//                                                 upFileCount);
//
//            std::string jsonString = MStatisUtil::getInstance()->toJson(MStatisTag::TagTrackerFile, id);
//            if (!jsonString.empty()) {
//                MProxyInterface::getInstance()->appendProxyEvent(jsonString);
//            }
//        }
//    }
//    return fileKeys.size();
//}
//
//int MDiskCache::getCacheFileInfoWithKey(const std::string &fileKey, CacheFileInfo &fileInfo) {
//    if (fileKey.empty()) {
//        return -1;
//    }
//    std::shared_ptr<MFileCacheInterface> configFile = nullptr;
//    {
//        MTimedLock lock(mReadWriteLock);
//        configFile = buildConfigFileWithFileKey(fileKey);
//    }
//    if (!configFile) {
//        return -1;
//    }
//    if (configFile->parse() != 0) {
//        return -1;
//    }
//
//    fileInfo.fileKey = fileKey;
//    fileInfo.fileSize = configFile->getFileSize();
//    fileInfo.cacheSize = configFile->getCacheSize();
//    fileInfo.accessTime = configFile->getAccessTime();
//    fileInfo.fileSign = configFile->calcFileSign();
//    return 0;
//}
//
//#pragma mark---- Build Cache File ----
//

std::shared_ptr<FileCacheInterface>
DiskCache::build_config_file_with_file_key(const std::string &file_key, bool add_top_map) {
    int status = DiskCacheUtil::is_cache_path_exist(cache_path_string_);
    if (status < 0) {
        return nullptr;
    }

    char md5_key[40];
    util_generate_md5_value((uint8_t *)md5_key, (uint8_t *)file_key.c_str(), (int)strlen(file_key.c_str()));
    status = DiskCacheUtil::is_file_path_exist(cache_path_string_, md5_key);
    if (status < 0) {
        return nullptr;
    }

    std::shared_ptr<FileCacheInterface> config_file = nullptr;
    if (is_file_key_exist_in_map(file_key)) {
        config_file = files_map_.get(file_key);
    }
    if (!config_file) {
        config_file = create_file_disk_cache(cache_path_string_, file_key);

        if (add_top_map) {
            files_map_.put(file_key, config_file);
        }
    }
    return config_file;
}



//#pragma mark---- tracker support private function ----
//
//void MDiskCache::removeFileNotificationWithFileKeys(const StringList &fileKeys) {
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "remove full cached file number = %d", int(fileKeys.size()));
//    auto listener = mCacheFileChangeListener.lock();
//    if (listener) {
//        listener->cacheRemoved(fileKeys);
//    }
//}
//
//
