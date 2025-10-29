//
// Created by Hongmingwei on 2025/10/29.
//

#include "DiskCacheUtil.h"

#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#include "GlobalConfig.h"
#include "Util.h"
#include "JsonParser.h"
#include "GlobalConstant.h"


DiskCacheUtil::DiskCacheUtil() {

}

DiskCacheUtil::~DiskCacheUtil() {

}

void
DiskCacheUtil::sort_by_share_num(std::vector<PairCacheConfigFile> &files_vector, bool is_small) {
    if (is_small) {
        std::sort(files_vector.begin(), files_vector.end(),
                  [](const PairCacheConfigFile &a, const PairCacheConfigFile &b) {
                      bool ret = false;
                      if (a.second->get_share_num() < b.second->get_share_num()) {
                          ret = true;
                      } else if (a.second->get_share_num() == b.second->get_share_num()) {
                          if (a.second->get_access_time() < b.second->get_access_time()) {
                              ret = true;
                          }
                      }
                      return ret;
                  });
    } else {
        std::sort(files_vector.begin(), files_vector.end(),
                  [](const PairCacheConfigFile &a, const PairCacheConfigFile &b) {
                      bool ret = false;
                      if (a.second->get_share_num() > b.second->get_share_num()) {
                          ret = true;
                      } else if (a.second->get_share_num() == b.second->get_share_num()) {
                          if (a.second->get_access_time() > b.second->get_access_time()) {
                              ret = true;
                          }
                      }
                      return ret;
                  });
    }
}

void DiskCacheUtil::sort_by_share_num(std::vector<CacheFileInfo> &files_vector, bool is_small) {

}



//void MDiskCacheUtil::sortByShareNum(std::vector<CacheFileInfo> &filesVector, bool isSmall)
//{
//    if (isSmall) {
//        //按分享数由少到多排序，分享数相同，按访问时间排序，时间远的排前面
//        std::sort(filesVector.begin(), filesVector.end(),
//                  [](CacheFileInfo &a, CacheFileInfo &b)
//                  {
//
//                      bool ret = false;
//                      if (a.getShareNum() < b.getShareNum()) {
//                          ret = true;
//                      }
//                      else if (a.getShareNum() == b.getShareNum()) {
//                          if (a.getAccessTime() < b.getAccessTime()) {
//                              ret = true;
//                          }
//                      }
//                      return ret;
//                  });
//    } else {
//        //按分享数由多到少排序，分享数相同，按访问时间排序，时间近的排前面
//        std::sort(filesVector.begin(), filesVector.end(),
//                  [](CacheFileInfo &a, CacheFileInfo &b)
//                  {
//
//                      bool ret = false;
//                      if (a.getShareNum() > b.getShareNum()) {
//                          ret = true;
//                      }
//                      else if (a.getShareNum() == b.getShareNum()) {
//                          if (a.getAccessTime() > b.getAccessTime()) {
//                              ret = true;
//                          }
//                      }
//                      return ret;
//                  });
//    }
//}
//
//bool MDiskCacheUtil::isExpire(int64_t lastAccessTime, int factor) {
//    int64_t currentTime = MUtilGetCurrentTimeInMilliSeconds();
//    int64_t expireTimeInMilliSecond = MGlobalConfig::getInstance()->getCacheExipredTimeInHours() * 60 * 60 * 1000;
//    int64_t delta = currentTime - lastAccessTime;
//
//    if (factor*delta >= expireTimeInMilliSecond) {
//        return true;
//    } else {
//        return false;
//    }
//}
//
//bool MDiskCacheUtil::isValidFile(std::shared_ptr<MFileCacheInterface> configFile, int shareNumExpireFactor,
//                                 StringList &fileRemovedList, bool isCheckExpire)
//{
//
//    int64_t fileLastAccessTime = configFile->getAccessTime();
//    int64_t shareNum = configFile->getShareNum();
//
//    if (isCheckExpire) {
//        if (MDiskCacheUtil::isExpire(fileLastAccessTime, 1)
//            || (shareNum == 0 && MDiskCacheUtil::isExpire(fileLastAccessTime, shareNumExpireFactor))) {
//            if (configFile->isCacheComplete()) {
//                fileRemovedList.push_back(configFile->getFileKey());
//            }
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "inValid file, key:%s, isExpire = %d, (shareNum:%lld,isExpire/%d:%d)",
//                         configFile->getFileKey().c_str(),
//                         MDiskCacheUtil::isExpire(fileLastAccessTime, 1) ? 1 : 0,
//                         shareNum,
//                         shareNumExpireFactor,
//                         MDiskCacheUtil::isExpire(fileLastAccessTime, shareNumExpireFactor) ? 1 : 0);
//            return false;
//        }
//    }
//
//    if ((!configFile->isCacheComplete()) ||
//        configFile->getFileSize() <= MGlobalConfig::getInstance()->getMinFileSizeUploadTracker()) {
//
//        if (configFile->isCacheComplete()) {
//            fileRemovedList.push_back(configFile->getFileKey());
//        }
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "inValid file, key:%s, isUnComplete:%d, isSmaller:%d,filesize:%lld(K)",
//                     configFile->getFileKey().c_str(),
//                     configFile->isCacheComplete(),
//                     configFile->getFileSize() <= MGlobalConfig::getInstance()->getMinFileSizeUploadTracker(),
//                     configFile->getFileSize() / 1024);
//        return false;
//    }
//
//    return true;
//}
//
//bool MDiskCacheUtil::isValidFile(CacheFileInfo &cacheFileInfo, int shareNumExpireFactor,
//                                 StringList &fileRemovedList, bool isCheckExpire)
//{
//    int64_t fileLastAccessTime = cacheFileInfo.getAccessTime();
//    int64_t shareNum = cacheFileInfo.getShareNum();
//
//    if (isCheckExpire) {
//        if (MDiskCacheUtil::isExpire(fileLastAccessTime, 1)
//            || (shareNum == 0 && MDiskCacheUtil::isExpire(fileLastAccessTime, shareNumExpireFactor))) {
//            if (cacheFileInfo.isCacheComplete()) {
//                fileRemovedList.push_back(cacheFileInfo.getFileKey());
//            }
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "inValid file, key:%s, isExpire = %d, (shareNum:%lld,isExpire/%d:%d)",
//                         cacheFileInfo.getFileKey().c_str(),
//                         MDiskCacheUtil::isExpire(fileLastAccessTime, 1) ? 1 : 0,
//                         shareNum,
//                         shareNumExpireFactor,
//                         MDiskCacheUtil::isExpire(fileLastAccessTime, shareNumExpireFactor) ? 1 : 0);
//            return false;
//        }
//    }
//
//    if ((!cacheFileInfo.isCacheComplete()) ||
//        cacheFileInfo.getFileSize() <= MGlobalConfig::getInstance()->getMinFileSizeUploadTracker()) {
//
//        if (cacheFileInfo.isCacheComplete()) {
//            fileRemovedList.push_back(cacheFileInfo.getFileKey());
//        }
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "inValid file, key:%s, isUnComplete:%d, isSmaller:%d,filesize:%lld(K)",
//                     cacheFileInfo.getFileKey().c_str(),
//                     cacheFileInfo.isCacheComplete(),
//                     cacheFileInfo.getFileSize() <= MGlobalConfig::getInstance()->getMinFileSizeUploadTracker(),
//                     cacheFileInfo.getFileSize() / 1024);
//        return false;
//    }
//
//    return true;
//}
//
//std::string MDiskCacheUtil::getConfigFileFullPathWithVersion(const std::string& cachePath, const std::string& md5Key, int version) {
//    std::string fileConfigPath;
//    std::stringstream stream;
//
//    if (cachePath.empty() || md5Key.empty()) {
//        return std::string("");
//    }
//    if (version == 1) {
//        stream << cachePath << "/" << md5Key << "/config.json";
//    } else if (version == 2) {
//        stream << cachePath << "/" << md5Key << "/config2.json";
//    }
//    fileConfigPath = stream.str();
//    return fileConfigPath;
//}
//
//int MDiskCacheUtil::getCacheFileInfoFromDir(const std::string &cachePath, const char *dir, CacheFileInfo &cacheFileInfo)
//{
//    std::string configFileName = MDiskCacheUtil::getConfigFileFullPathWithVersion(cachePath, dir, 1);
//    if (configFileName.empty()) {
//        return -1;
//    }
//
//    int result = isConfigFileWithFullNameExist(configFileName);
//    if (result < 0) {
//        configFileName = MDiskCacheUtil::getConfigFileFullPathWithVersion(cachePath, dir, 2);
//        result = isConfigFileWithFullNameExist(configFileName);
//        if (result < 0) {
//            return kDiskCacheConfigV2NotExist;
//        }
//    }
//
//    std::ifstream infile(configFileName);
//    if (!infile.is_open()) {
//        return kDiskCacheConfigFileOpenFail;
//    }
//    std::stringstream stream;
//    stream << infile.rdbuf();
//    std::string jsonString = stream.str();
//
//    MomoBase::MJsonParser jsonParser(jsonString);
//    if (jsonParser.parse() != 0) {
//        return kDiskCacheConfigJsonFail;
//    }
//
//    std::string fileKey = jsonParser.optGetString(kFileKey, "");
//    if (fileKey.empty()) {
//        return kDiskCacheConfigJsonFail;
//    }
//
//    uint64_t fileSize = jsonParser.optGetUint64(kCachedSizeKey, 0);
//    uint64_t accessTime = jsonParser.optGetUint64(kAccessTimeKey, 0);
//    uint64_t cachedSize = jsonParser.optGetUint64(kCachedSizeKey, 0);
//    int64_t shareNum = jsonParser.optGetInt64(kShareNumKey, 0);
//
//    cacheFileInfo.fileKey = fileKey;
//    cacheFileInfo.accessTime = accessTime;
//    cacheFileInfo.fileSize = fileSize;
//    cacheFileInfo.cacheSize = cachedSize;
//    cacheFileInfo.shareNum = shareNum;
//
//    return 0;
//}
//
//int MDiskCacheUtil::isConfigFileWithFullNameExist(const std::string& fileName)
//{
//    int pathExist = DiskCacheStatusOK;
//    if (fileName.empty()) {
//        return DiskCacheStatusPathNotExist;
//    }
//    if (access(fileName.c_str(), F_OK) != 0) {
//        pathExist = DiskCacheStatusPathNotExist;
//        return pathExist;
//    }
//    return pathExist;
//}
//
//int MDiskCacheUtil::getAllCachedFileKeyAndTotalSize(const std::string& cachePathString, int64_t& totalCachedSize, StringList &fileList)
//{
//    char* cachePath = (char *)cachePathString.c_str();
//    DIR *dir = opendir(cachePath);
//    struct dirent *inFile;
//    int filesCount = 0;
//    totalCachedSize = 0;
//    if (dir) {
//        while ((inFile = readdir(dir))) {
//            if (inFile->d_type == DT_DIR && strcmp(inFile->d_name, ".") &&
//                strcmp(inFile->d_name, "..")) {
//
//                CacheFileInfo cacheFileInfo;
//                int result = MDiskCacheUtil::getCacheFileInfoFromDir(cachePathString, inFile->d_name, cacheFileInfo);
//                if (result < 0) {
//                    continue;
//                }
//                std::string fileKey = cacheFileInfo.fileKey;
//                if (fileKey.empty()) {
//                    continue;
//                }
//
//                if (cacheFileInfo.isCacheComplete()) {
//                    fileList.push_back(fileKey);
//                }
//
//                int64_t cachedSize = (int64_t)cacheFileInfo.cacheSize;
//                if (cachedSize >= 0) {
//                    totalCachedSize += cachedSize;
//                    if (filesCount < 20) {
//                        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                                     "file key = %s, and video size = %lld", fileKey.c_str(),
//                                     cachedSize);
//                    }
//                }
//                filesCount++;
//            }
//        }
//        closedir(dir);
//    }
//    return filesCount;
//}
//
//#ifdef __ANDROID__
//static int removeDirAndroid(char *path) {
//    DIR *dir = opendir(path);
//    int status = 0;
//    int ret = 0;
//    char fileName[1024];
//    if (dir) {
//        struct dirent *dirent = NULL;
//        while ((dirent = readdir(dir)) != NULL) {
//            if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {
//                sprintf(fileName, "%s/%s", path, dirent->d_name);
//                ret = remove(fileName);
//                __MDLOGD_TAG("MDiskCache", "remove file = %s, ret = %d", fileName, ret);
//            }
//        }
//        ret = remove(path);
//        //        __MDLOGD_TAG("MDiskCache", "remove file = %s, ret = %d", fileName, ret);
//        closedir(dir);
//    } else {
//        status = -1;
//        __MDLOGD_TAG("MDiskCache", "opendir %s fail", path);
//    }
//    return status;
//}
//#endif
//
//static int removeCallback(const char *path, const struct stat *st, int typeFlag,
//                          struct FTW *ftwBuf) {
//    int result = remove(path);
//    if (result) {
//        __MDLOGD_TAG("MDiskCache", "remove path = %s, result = %d", path, result);
//    }
//    return result;
//}
//
//int MDiskCacheUtil::removeFilesAtPath(const std::string &cachePathString, char *path) {
//    char fullPath[1024];
//    char* cachePath = (char *)cachePathString.c_str();
//    sprintf(fullPath, "%s/%s", cachePath, path);
//
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "remove file = %s", fullPath);
//#ifdef __ANDROID__
//    int result = removeDirAndroid(fullPath);
//    return result;
//#else
//    int result = nftw(fullPath, removeCallback, 64, FTW_DEPTH | FTW_PHYS);
//    return result;
//#endif
//}
//
//int MDiskCacheUtil::removeFilesAtPathWithFileKey(const std::string &cachePathString, const std::string& fileKey)
//{
//    char md5Key[40];
//    MUtilGenerateMD5Value((uint8_t *)md5Key, (uint8_t *)fileKey.c_str(),
//                          (int)strlen(fileKey.c_str()));
//
//
//    return removeFilesAtPath(cachePathString, md5Key);
//}
//
//int MDiskCacheUtil::removeAllFileInAbsolutePath(const std::string &cachePathString)
//{
//    if (cachePathString.empty()) {
//        return -1;
//    }
//    char* cachePath = (char *)cachePathString.c_str();
//    DIR *dir = opendir(cachePath);
//    struct dirent *inFile;
//    if (dir) {
//        while ((inFile = readdir(dir))) {
//            if (inFile->d_type == DT_DIR &&
//                strcmp(inFile->d_name, ".") &&
//                strcmp(inFile->d_name, "..")) {
//                MDiskCacheUtil::removeFilesAtPath(cachePathString, inFile->d_name);
//            }
//        }
//        closedir(dir);
//    }
//    return 0;
//}
//
//int64_t MDiskCacheUtil::removeCacheWithKey(const std::string &cachePathString, const std::string &fileKey) {
//    char fileName[1024];
//    char md5Key[40];
//    memset(md5Key, 0, 40);
//    memset(fileName, 0, 1024);
//    MUtilGenerateMD5Value((uint8_t *)md5Key, (uint8_t *)fileKey.c_str(),
//                          (int)strlen(fileKey.c_str()));
//
//
//    char* cachePath = (char *)cachePathString.c_str();
//    sprintf(fileName, "%s/%s", cachePath, md5Key);
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "fileName = %s", fileName);
//    DIR *dir = opendir(fileName);
//    //    struct dirent* inFile;
//    if (dir) {
//        int64_t cachedSize = 0;
//        char configFileName[1024];
//        sprintf(configFileName, "%s/config.json", fileName);
//
//        cachedSize = MCacheConfigFile::getCachedSize(configFileName);
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "configFileName = %s, cachedSize = %lld", configFileName, cachedSize);
//        MDiskCacheUtil::removeFilesAtPath(cachePathString, md5Key);
//        //        while ((inFile = readdir(dir))) {
//        //            __MDLOGE_TAG(TAG, "d_name = %s, d_type = %d", inFile->d_name, inFile->d_type);
//        //            if (inFile->d_type == DT_DIR &&
//        //                strcmp(inFile->d_name, ".") &&
//        //                strcmp(inFile->d_name, "..")) {
//        //
//        //                fileSize = MCacheConfigFile::getCachedSize(configFileName);
//        //                __MDLOGE_TAG(TAG, "configFileName = %s, fileSize = %lld", configFileName,
//        //                fileSize); removeFilesAtPath(inFile->d_name);
//        //            }
//        //        }
//        closedir(dir);
//        return cachedSize;
//    } else {
//        return -1;
//    }
//}
//
//bool MDiskCacheUtil::isNeedClean(int64_t allCacheSize) {
//    int64_t maxLimitInBytes = MGlobalConfig::getInstance()->getCacheMaxLimitInMB() * 1024 * 1024;
//    if (allCacheSize >= maxLimitInBytes) {
//        return true;
//    } else {
//        return false;
//    }
//}

int DiskCacheUtil::is_file_path_exist(const std::string &cache_path_string, const char *file_key) {
    int path_exist = DiskCacheStatusOK;
    char folder_name[2048];

    char *cache_path = const_cast<char *>(cache_path_string.c_str());
    sprintf(folder_name, "%s/%s", cache_path, file_key);

    if (access(folder_name, F_OK) != 0) {
        if (mkdir(folder_name, 0777) != 0) {
            path_exist = DiskCacheStatusPathNotExist;
            return path_exist;
        }
    }
    return path_exist;
}

int DiskCacheUtil::is_config_file_exist(const std::string &cache_path_string, const char *md5_key) {
    int path_exist = DiskCacheStatusOK;
    char config_file[2048];

    char *cache_path = const_cast<char *>(cache_path_string.c_str());
    sprintf(config_file, "%s/%s", cache_path, md5_key);

    if (access(config_file, F_OK) != 0) {
        path_exist = DiskCacheStatusPathNotExist;
        return path_exist;
    }

    return path_exist;
}


int DiskCacheUtil::is_cache_path_exist(const std::string &cache_path_string) {
    char* cache_path = const_cast<char *>(cache_path_string.c_str());
    if (access(cache_path, F_OK) != 0) {
        if (mkdir(cache_path, 0777) != 0) {
            return DiskCacheStatusPathNotExist;
        }
    }
    return DiskCacheStatusOK;
}


//// 注意迭代器失效的问题
//void MDiskCacheUtil::removeExpireCache(MFileCacheInterfaceMap &filesMap,
//                                       StringList &fileRemoveList,
//                                       int64_t &currentCachedSize,
//                                       bool expiredFirstRemove)
//{
//    if (expiredFirstRemove) {
//        //首先把过期的缓存都删除
//        for (auto it = filesMap.begin(); it != filesMap.end();) {
//            std::shared_ptr<MFileCacheInterface> configFile = it->second;
//            if (configFile->parse() != 0) {
//                it++;
//                continue;
//            }
//            int64_t fileLastAccessTime = configFile->getAccessTime();
//            if (MDiskCacheUtil::isExpire(fileLastAccessTime, 1)) {
//
//                if (configFile->isCacheComplete()) {
//                    fileRemoveList.push_back(configFile->getFileKey());
//                }
//                currentCachedSize -= configFile->getCacheSize();
//                configFile->remove();
//                filesMap.erase(it++);
//            } else {
//                it++;
//            }
//
//            //            if (!isNeedClean(currentCachedSize)) {
//            //                __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//            //                             "Finish clean data, and exit， current size = %lld, limit size = %lld",
//            //                             currentCachedSize);
//            //                break;
//            //            }
//        }
//
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Clear all expired file, current size = %lld",
//                     currentCachedSize);
//
//    } else {
//
//        std::vector<PairCacheConfigFile> filesVector;
//        for (auto it = filesMap.begin(); it != filesMap.end(); it++) {
//            filesVector.push_back(std::make_pair(it->first, it->second));
//        }
//
//        std::sort(filesVector.begin(), filesVector.end(),
//                  [](const PairCacheConfigFile& a,
//                     const PairCacheConfigFile& b) {
//                      return (a.second->getAccessTime() < b.second->getAccessTime());
//                  });
//
//        std::for_each(filesVector.begin(), filesVector.end(), [](const PairCacheConfigFile& item){
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "Sorted file = %s, accessTime = %llu",
//                         item.first.c_str(), item.second->getAccessTime());
//        });
//
//        //按照access time删除文件
//        for (auto it = filesVector.begin(); it != filesVector.end(); it++) {
//            std::shared_ptr<MFileCacheInterface> configFile = it->second;
//            if (configFile->isCacheComplete()) {
//                fileRemoveList.push_back(configFile->getFileKey());
//            }
//            currentCachedSize -= configFile->getCacheSize();
//            configFile->remove();
//            filesMap.erase(it->first);
//
//            if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//                __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                             "Finish clean data, and exit， current size = %lld",
//                             currentCachedSize);
//                break;
//            }
//        }
//    }
//}
//
//void MDiskCacheUtil::removeExpireCache(const std::string &cachePathString, std::vector<CacheFileInfo> &totalFileList,
//                                       StringList &fileRemoveList,
//                                       int64_t &currentCachedSize,
//                                       bool expiredFirstRemove)
//{
//    if (expiredFirstRemove) {
//        //首先把过期的缓存都删除
//        for (auto it = totalFileList.begin(); it != totalFileList.end(); it++) {
//            int64_t fileLastAccessTime = it->getAccessTime();
//            if (MDiskCacheUtil::isExpire(fileLastAccessTime, 1)) {
//                if (it->isCacheComplete()) {
//                    fileRemoveList.push_back(it->getFileKey());
//                }
//                MDiskCacheUtil::removeFilesAtPathWithFileKey(cachePathString, it->getFileKey());
//                currentCachedSize -= it->getCacheSize();
//            }
//        }
//
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Clear all expired file, current size = %lld",
//                     currentCachedSize);
//    }
//    else {
//
//        std::sort(totalFileList.begin(), totalFileList.end(),
//                  [](CacheFileInfo &a,
//                     CacheFileInfo &b)
//                  {
//                      return (a.getAccessTime() < b.getAccessTime());
//                  });
//
//        std::for_each(totalFileList.begin(), totalFileList.end(), [](CacheFileInfo &item)
//        {
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "Sorted file = %s, accessTime = %llu",
//                         item.getFileKey().c_str(), item.getAccessTime());
//        });
//
//        //按照access time删除文件
//        for (auto it = totalFileList.begin(); it != totalFileList.end(); it++) {
//            if (it->isCacheComplete()) {
//                fileRemoveList.push_back(it->getFileKey());
//            }
//            MDiskCacheUtil::removeFilesAtPathWithFileKey(cachePathString, it->getFileKey());
//            currentCachedSize -= it->getCacheSize();
//            if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//                __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                             "Finish clean data, and exit， current size = %lld",
//                             currentCachedSize);
//                break;
//            }
//        }
//    }
//}
//
////优先级：
////1. 删除不完整的、小于600k、过期(例：ExpireDay=15day)文件
////2. 删除分享数为0且访问时间已经超过ExpireDay/3 day的文件
////3. 删除分享数少的文件
//void MDiskCacheUtil::removeExpireCacheV3(MFileCacheInterfaceMap &filesMap,
//                                         StringList &fileRemoveList,
//                                         int64_t &currentCachedSize)
//{
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "current size =%lld", currentCachedSize);
//
//    for (auto it = filesMap.begin(); it != filesMap.end();) {
//        std::shared_ptr<MFileCacheInterface> configFile = it->second;
//        if (configFile->parse() != 0) {
//            it++;
//            continue;
//        }
//
//        if (MDiskCacheUtil::isValidFile(configFile, 3, fileRemoveList, true)) {
//            it++;
//            continue;
//        }
//        else {
//            currentCachedSize -= configFile->getCacheSize();
//            configFile->remove();
//            filesMap.erase(it++);
//        }
//    }
//
//    if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Finish clean data, and pre exit， current size = %lld",
//                     currentCachedSize);
//        return;
//    }
//
//    std::vector<PairCacheConfigFile> filesVector;
//    for (auto it = filesMap.begin(); it != filesMap.end(); it++) {
//        filesVector.push_back(std::make_pair(it->first, it->second));
//    }
//
//    if (!filesVector.empty()) {
//        MDiskCacheUtil::sortByShareNum(filesVector, true);
//    }
//
//    for (auto item : filesVector) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Sorted file = %s, accessTime = %llu, shareNum:%lld",
//                     item.first.c_str(), item.second->getAccessTime(), item.second->getShareNum());
//    }
//
//    for (auto it = filesVector.begin(); it != filesVector.end(); it++) {
//        if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "Finish clean data, and exit， current size = %lld",
//                         currentCachedSize);
//            break;
//        }
//
//        std::shared_ptr<MFileCacheInterface> configFile = it->second;
//        if (configFile->isCacheComplete()) {
//            fileRemoveList.push_back(configFile->getFileKey());
//        }
//        currentCachedSize -= configFile->getCacheSize();
//        configFile->remove();
//        filesMap.erase(it->first);
//    }
//
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "Clear all expired file, current size = %lld",
//                 currentCachedSize);
//
//}
//
//void MDiskCacheUtil::removeExpireCacheV3(std::vector<CacheFileInfo> &totalFileList,
//                                         StringList &fileRemoveList,
//                                         int64_t &currentCachedSize)
//{
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk), "current size =%lld", currentCachedSize);
//
//    for (auto it = totalFileList.begin(); it != totalFileList.end();) {
//
//        CacheFileInfo cacheFileInfo = *it;
//        if (MDiskCacheUtil::isValidFile(cacheFileInfo, 3, fileRemoveList, true)) {
//            it++;
//            continue;
//        }
//        else {
//            currentCachedSize -= it->getCacheSize();
//        }
//    }
//
//    if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Finish clean data, and pre exit， current size = %lld",
//                     currentCachedSize);
//        return;
//    }
//
//
//    std::sort(totalFileList.begin(), totalFileList.end(),
//              [](CacheFileInfo &a, CacheFileInfo &b) {
//                  bool ret = false;
//                  if (a.getShareNum() < b.getShareNum()) {
//                      ret = true;
//                  }
//                  else if (a.getShareNum() == b.getShareNum()) {
//                      if (a.getAccessTime() < b.getAccessTime()) {
//                          ret = true;
//                      }
//                  }
//                  return ret;
//              });
//
//    for (auto item : totalFileList) {
//        __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                     "Sorted file = %s, accessTime = %llu, shareNum:%lld",
//                     item.getFileKey().c_str(), item.getAccessTime(), item.getShareNum());
//    }
//
//    for (auto it = totalFileList.begin(); it != totalFileList.end(); it++) {
//        if (!MDiskCacheUtil::isNeedClean(currentCachedSize)) {
//            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                         "Finish clean data, and exit， current size = %lld",
//                         currentCachedSize);
//            break;
//        }
//        if (it->isCacheComplete()) {
//            fileRemoveList.push_back(it->getFileKey());
//        }
//        currentCachedSize -= it->getCacheSize();
//    }
//
//    __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeDisk),
//                 "Clear all expired file, current size = %lld",
//                 currentCachedSize);
//}
