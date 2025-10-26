//
// Created by Hongmingwei on 2025/10/24.
//

#include "MemoryMediaCache.h"

#include <memory>

#include "ThreadUtil.h"
#include <DiskCache.h>
#include <Util.h>
#include <GlobalConfig.h>
#include <GlobalConstant.h>

template<typename T>
struct ArrayDeleter {
    void operator ()(T const * p) {
        delete[] p;
    }
};

MemoryMediaCache::MemoryMediaCache(const std::string &file_key) {
    file_key_ = file_key;
    file_size_ = 0;
    access_time_ = util_get_current_time_in_milli_seconds();

    audio_duration_ = 0;
    video_duration_ = 0;
    cached_size_ = 0;
    preload_size_ = 0;
    memory_usage_ = 0;
    is_dump_ = false;
}

int64_t
MemoryMediaCache::write_data(uint8_t *buffer, int64_t offset, uint64_t size, int64_t file_size) {
    proxy::WriteLock lock(read_write_lock_);
    if (file_size > 0) {
        file_size_ = file_size;
    }
    int64_t write_size = 0;
    int64_t write_start = offset;
    int64_t write_end = offset + size;
    int64_t write_seg_size;
    int64_t remain_size = size;

    auto it = media_file_slices_.begin();
    for (; it != media_file_slices_.end();) {
        int64_t seg_start_position = it->offset_;
        int64_t seg_file_size = it->size_;
        int64_t seg_end_position= seg_start_position + seg_file_size;

        if (write_start < seg_start_position) {
            if (write_end <= seg_start_position) {
                write_seg_size = remain_size;
            } else {
                write_seg_size = seg_start_position - write_start;
            }

            if (write_seg_size > 0) {
                MemoryMediaSlice slice;
                slice.offset_ = write_start;
                slice.size_ = write_seg_size;
                slice.timestamp_ = util_get_current_time_in_milli_seconds();
                slice.buffer_.insert(slice.buffer_.begin(), buffer + (write_start - offset),
                                     buffer + (write_start - offset) + slice.size_);

                write_size += slice.size_;
                write_start += slice.size_;
                write_seg_size -= slice.size_;
                remain_size -= slice.size_;
                media_file_slices_.insert(it, slice);
            } else {
                it++;
            }
        } else if (write_start >= seg_start_position && write_start < seg_end_position) {
            if (write_end <= seg_end_position) {
                remain_size = 0;
                break;
            } else {
                write_seg_size = seg_end_position - write_start;
                remain_size -= write_seg_size;
                write_start = seg_end_position;
                write_end = write_start + remain_size;
                it++;
            }
        } else {
            it++;
        }
        if (remain_size == 0) {
            break;
        }
    }
    while (remain_size > 0) {
        MemoryMediaSlice slice;
        slice.offset_ = write_start;
        slice.size_ = remain_size;
        slice.timestamp_ = util_get_current_time_in_milli_seconds();
        slice.buffer_.insert(slice.buffer_.begin(), buffer + (write_start - offset),
                             buffer + (write_start - offset) + slice.size_);

        write_size += slice.size_;
        write_start += slice.size_;
        remain_size -= slice.size_;
        media_file_slices_.push_back(slice);
    }

    if (write_size > 0) {
        memory_usage_ += write_size;
    }
    return write_size;
}





//int64_t MMemoryMediaCache::readData(uint8_t *buffer, uint64_t offset, uint64_t size)
//{
//    MWriteLock lock(mReadWriteLock);
//    int isFound = 0;
//    int64_t readSize = 0;
//    MMemoryMediaSlice *slice = NULL;
//
//    uint64_t readStart = offset;
//    uint64_t readEnd = offset + size;
//
//    uint64_t segStart = 0;
//    uint64_t segEnd = 0;
//
//    uint64_t readSegSize = 0;
//    uint64_t readOffset = 0;
//
//    for (auto it = mMediaFileSlices.begin(); it != mMediaFileSlices.end(); it++) {
//
//        uint64_t segStartPosition = it->mOffset;
//        uint64_t segFileSize = it->mSize;
//        uint64_t segEndPosition = segStartPosition + segFileSize;
//
//        if (readStart >= segStartPosition && readEnd <= segEndPosition) {
//            // 数据都在当前段落
//            readSegSize = readEnd - readStart;
//        } else if (readStart >= segStartPosition &&
//                   readStart < segEndPosition &&
//                   readEnd > segEndPosition) {
//            // 数据有分段
//            readSegSize = segEndPosition - readStart;
//        } else {
//            continue;
//        }
//        readOffset = readStart - segStartPosition;
//
////        __MDLOGD_TAG(TAG, "file offset = %u, size = %u", readStart, readSegSize);
////        memcpy(buffer + readSize, it->mBuffer + readOffset, readSegSize);
//        for (int i = 0; i < readSegSize; i++) {
//            buffer[readSize + i] = it->mBuffer[i + readOffset];
//        }
//
//        readSize += readSegSize;
//        readStart += readSegSize;
//        size -= readSegSize;
//
//        if (size == 0) {
//            break;
//        }
//    }
//
//    mAccessTime = MUtilGetCurrentTimeInMilliSeconds();
//
//    return readSize;
//
////    do {
////        isFound = findSlice(readStart, readSize, &slice);
////        if (isFound > 0) {
////            segStart = slice->mOffset;
////            segEnd = segStart + slice->mSize;
////
////            if (readStart >= segStart && readStart < segEnd) {
////
////                if (readEnd >= segEnd) {
////                    readSegSize = segEnd - readStart;
////                    readOffset = readStart - segStart;
////                } else {
////                    readSegSize = readEnd - readStart;
////                    readOffset = readStart - segStart;
////                }
////
////            }
////            memcpy(buffer + readSize, slice->mBuffer + readOffset, readSegSize);
////            readSize += readSegSize;
////            readStart += readSegSize;
////        }
////        if (readSize == size) {
////            break;
////        }
////    } while (isFound > 0);
////    return readSize;
//
//    //    uint64_t readStart = offset;
//    //    uint64_t readEnd = offset + size;
//    //
//    //    uint64_t segStart = mOffset;
//    //    uint64_t segEnd = mOffset + mSize;
//    //
//    //    uint64_t readSize = 0;
//    //    uint64_t readOffset = 0;
//    //
//    //    if (readStart >= segStart && readStart < segEnd) {
//    //
//    //        if (readEnd >= segEnd) {
//    //            readSize = segEnd - readStart;
//    //            readOffset = readStart - segStart;
//    //        } else {
//    //            readSize = readEnd - readStart;
//    //            readOffset = readStart - segStart;
//    //        }
//    //
//    //    }
//    //
//    //    if (readSize > 0) {
//    //        memcpy(buffer, mData.get() + readOffset, readSize);
//    //    }
//    //
//    //    return readSize;
//}
//
//void MMemoryMediaCache::setFileSize(uint64_t fileSize)
//{
//    MWriteLock lock(mReadWriteLock);
//    mFileSize = fileSize;
//}
//
//void MMemoryMediaCache::setFileKey(std::string fileKey)
//{
//    MWriteLock lock(mReadWriteLock);
//    mFileKey = fileKey;
//}
//
//void MMemoryMediaCache::serialize()
//{
//    MWriteLock lock(mReadWriteLock);
//
//    while (!mMediaFileSlices.empty()) {
//        auto it = mMediaFileSlices.begin();
//        if (it->mSize > 0) {
//            MDiskCache::getInstance()->writeData(mFileKey.c_str(), &it->mBuffer[0], it->mOffset, it->mSize);
//            __MDLOGD_TAG(TAG, "save file = %s, offset = %lld, size = %lld", mFileKey.c_str(), it->mOffset, it->mSize);
//        }
//
//        mMediaFileSlices.erase(it);
//    }
//}
//
//bool MMemoryMediaCache::isCacheExpired()
//{
//    MWriteLock lock(mReadWriteLock);
//    uint64_t currentTime = MUtilGetCurrentTimeInMilliSeconds();
//    uint64_t delta = currentTime - mAccessTime;
//    int64_t expireTime = MGlobalConfig::getInstance()->getMemoryExpiredTimeInSecond();
//    if (delta > expireTime * 1000) {
//        __MDLOGD_TAG(TAG, "memory expired = %lld ms, key:%s", delta, mFileKey.c_str());
//        return true;
//    } else {
//        return false;
//    }
//}
//
//int64_t MMemoryMediaCache::serializeExpiredCache()
//{
//    MWriteLock lock(mReadWriteLock);
//    int64_t writeSize = 0;
//    while (!mMediaFileSlices.empty()) {
//        auto it = mMediaFileSlices.begin();
//        if (it->mSize > 0) {
//
//            MDiskCache::getInstance()->setInstanceParameter(mFileKey, kPreloadSizeKey, mPreloadSize);
//            MDiskCache::getInstance()->setInstanceParameter(mFileKey, kPreloadAudioDurationKey, mAudioDuration);
//            MDiskCache::getInstance()->setInstanceParameter(mFileKey, kPreloadVideoDurationKey, mVideoDuration);
//
//            MDiskCache::getInstance()->writeData(mFileKey.c_str(), &it->mBuffer[0], it->mOffset, it->mSize, mFileSize);
//
////            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeMemory),
////                         "PreloadSizeKey = %lld, AudioDuration = %lld, VideoDuration = %lld",
////                         mPreloadSize, mAudioDuration, mVideoDuration);
////            __MDLOGD_TAG(MLogTAG::getInstance()->getTag(MLogTAGTypeMemory),
////                         "save file = %s, offset = %lld, size = %lld",
////                         mFileKey.c_str(), it->mOffset, it->mSize);
//            writeSize += it->mSize;
//        }
//        mMediaFileSlices.erase(it);
//    }
//    if( writeSize > 0 ){
//        MDiskCache::getInstance()->flushConfigFile(mFileKey);
//    }
//    return writeSize;
//}
//
//int64_t MMemoryMediaCache::getFileSize()
//{
//    MWriteLock lock(mReadWriteLock);
//    return mFileSize;
//}
//
//void MMemoryMediaCache::dumpData()
//{
//    MWriteLock lock(mReadWriteLock);
//
//    if (mIsDump) {
//        return;
//    }
//    mIsDump = true;
//
//    for (auto it = mMediaFileSlices.begin(); it != mMediaFileSlices.end(); it++) {
//        __MDLOGD_TAG("Media", "offset = %llu, size = %llu, time = %llu", it->mOffset, it->mSize, it->mTimeStamp);
////        std::cout << "offset: " << it->mOffset;
////        std::cout << ", size: " << it->mSize << std::endl;
//    }
//
////    std::cout << std::endl;
//}
//
//int MMemoryMediaCache::findSlice(uint64_t offset, uint64_t size, MMemoryMediaSlice **slice)
//{
//    int isFound = 0;
//    auto it = mMediaFileSlices.begin();
//    for (; it != mMediaFileSlices.end(); it++) {
//        if (offset >= it->mOffset && offset + size <= it->mOffset + it->mSize) {
//            isFound = 1;
//            *slice = &(*it);
//            break;
//        }
//    }
//    if (it == mMediaFileSlices.end()) {
//        isFound = 0;
//    }
//    return isFound;
//}
//
//std::pair<uint64_t, uint64_t> MMemoryMediaCache::queryRemainDataByOffset(uint64_t offset)
//{
//    MWriteLock lock(mReadWriteLock);
//    auto it = mMediaFileSlices.begin();
//    auto ite = mMediaFileSlices.end();
//    for (; it != ite; it++) {
//        uint64_t segEndPos = it->mOffset + it->mSize;
//        if (offset < it->mOffset) {
//            // 不在分段内
//            auto curRemainBytes = 0llu;
//            auto needBytes = 0llu;
//            needBytes = it->mOffset - offset;
//            return std::make_pair(curRemainBytes, needBytes);
//        } else if (offset >= it->mOffset && offset < segEndPos) { // 在分段内
//            // 计算当前分段剩余
//            auto curRemainBytes = segEndPos - offset;
//            auto lastEndPos = segEndPos;
//            auto needBytes = 0llu;
//            // 查找下一分段 是否连续
//            it++;
//            for (; it != ite;) {
//                uint64_t isegEndPos = it->mOffset + it->mSize;
//                if (lastEndPos == it->mOffset) { // 连续
//                    curRemainBytes += it->mSize;
//                    lastEndPos = isegEndPos;
//                    it++;
//                } else if(lastEndPos < it->mOffset){
//                    //不连续,新段offset大于上一个分段结束
//                    needBytes = it->mOffset - lastEndPos;
//                    break;
//                } else {
//                    //新段与上一个分段有重合
//                    if(isegEndPos > lastEndPos){
//                        curRemainBytes += isegEndPos - lastEndPos;
//                        lastEndPos = isegEndPos;
//                    }
//                    it++;
//                }
//            }
//            return std::make_pair(curRemainBytes, needBytes);
//        }
//    }
//    return std::make_pair(0, 0);
//}
//
//int MMemoryMediaCache::queryEmptySegment(
//        std::vector<std::pair<uint64_t, uint64_t>> &emptySegmentVector)
//{
//    MWriteLock lock(mReadWriteLock);
//
//    if(mFileSize <= 0 || mMediaFileSlices.empty()){
//        return -1;
//    }
//    auto it = mMediaFileSlices.begin();// 前提 mSegments 必须是排序的
//    auto itEnd = mMediaFileSlices.end();
//
//    auto segmentStartPosition = it->mOffset;
//    uint64_t lastSegmentEndPosition = it->mOffset+it->mSize;
//    //第一个段是否从0开始
//    if(segmentStartPosition != 0){
//        std::pair<uint64_t, uint64_t> emptySegment = std::make_pair(0, segmentStartPosition);
//        emptySegmentVector.push_back(emptySegment);
//    }
//
//    it++;
//    for (; it != itEnd; it++){
//        auto segmentStartPosition = it->mOffset;
//        auto segmentEndPosition = it->mOffset+it->mSize;
//        if(lastSegmentEndPosition == segmentStartPosition){
//            lastSegmentEndPosition = segmentEndPosition;
//        }else if(lastSegmentEndPosition < segmentStartPosition){
//            auto emptySize = segmentStartPosition - lastSegmentEndPosition;
//            std::pair<uint64_t, uint64_t> emptySegment =
//                    std::make_pair(lastSegmentEndPosition, emptySize);
//            emptySegmentVector.push_back(emptySegment);
//            lastSegmentEndPosition = segmentEndPosition;
//        }else {
//            if(lastSegmentEndPosition < segmentEndPosition){
//                lastSegmentEndPosition = segmentEndPosition;
//            }
//        }
//    }
//
//    //最后的段，是否到文件尾
//    if(lastSegmentEndPosition < mFileSize){
//        std::pair<uint64_t, uint64_t> emptySegment =
//                std::make_pair(lastSegmentEndPosition, mFileSize - lastSegmentEndPosition);
//        emptySegmentVector.push_back(emptySegment);
//    }
//
//    return 0;
//}
//
//int64_t MMemoryMediaCache::getInstanceParameter(const std::string& parameterKey)
//{
//    MWriteLock lock(mReadWriteLock);
//    if (parameterKey.compare(kCachedSizeKey) == 0) {
//        return mCachedSize;
//    } else if (parameterKey.compare(kFileSizeKey) == 0) {
//        return mFileSize;
//    } else if (parameterKey.compare(kPreloadAudioDurationKey) == 0) {
//        return mAudioDuration;
//    } else if (parameterKey.compare(kPreloadVideoDurationKey) == 0) {
//        return mVideoDuration;
//    } else if (parameterKey.compare(kPreloadSizeKey) == 0) {
//        return mPreloadSize;
//    } else {
//        return -1;
//    }
//}
//
//void MMemoryMediaCache::getInstanceParameterWithMap(Int64Map& map)
//{
//    MWriteLock lock(mReadWriteLock);
//    for (auto& item : map) {
//        std::string key = item.first;
//        int64_t data = -1;
//        if (key.compare(kCachedSizeKey) == 0) {
//            data = mCachedSize;
//        } else if (key.compare(kFileSizeKey) == 0) {
//            data = mFileSize;
//        } else if (key.compare(kPreloadAudioDurationKey) == 0) {
//            data = mAudioDuration;
//        } else if (key.compare(kPreloadVideoDurationKey) == 0) {
//            data = mVideoDuration;
//        } else if (key.compare(kPreloadSizeKey) == 0) {
//            data = mPreloadSize;
//        }
//        map[key] = data;
//    }
//}
//
//void MMemoryMediaCache::setInstanceParameter(const std::string& parameterKey, int64_t value)
//{
//    MWriteLock lock(mReadWriteLock);
//    if (parameterKey.compare(kCachedSizeKey) == 0) {
//        mCachedSize = value;
//    } else if (parameterKey.compare(kFileSizeKey) == 0) {
//        mFileSize = value;
//    } else if (parameterKey.compare(kPreloadAudioDurationKey) == 0) {
//        mAudioDuration = value;
//    } else if (parameterKey.compare(kPreloadVideoDurationKey) == 0) {
//        mVideoDuration = value;
//    } else if (parameterKey.compare(kPreloadSizeKey) == 0) {
//        mPreloadSize = value;
//    }
//}
//
//int64_t MMemoryMediaCache::calculateMemoryUsage()
//{
//    MWriteLock lock(mReadWriteLock);
//    int64_t memoryUsage = 0;
//    for (auto it = mMediaFileSlices.begin(); it != mMediaFileSlices.end(); it++) {
//        memoryUsage += it->mSize;
//    }
//    return memoryUsage;
//}
//
//bool MMemoryMediaCache::isCacheComplete()
//{
//    MWriteLock lock(mReadWriteLock);
////    __MDLOGD_TAG(TAG, "mMemoryUsage = %lld, mFileSize = %lld", mMemoryUsage, mFileSize);
//    return (mMemoryUsage >= mFileSize && mFileSize > 0);
//}
//
//bool MMemoryMediaCache::queryDataRangeExist(int64_t start, int64_t size)
//{
//    MWriteLock lock(mReadWriteLock);
//    bool exist = false;
//
//    int64_t rangeStart = start;
//    int64_t rangeEnd = start + size - 1;
//
//    auto itStart = std::find_if(mMediaFileSlices.begin(), mMediaFileSlices.end(), [&](const MMemoryMediaSlice& slice) {
//        int64_t segStart = slice.mOffset;
//        int64_t segEnd = slice.mSize + segStart - 1;
//        if (rangeStart >= segStart && rangeStart <= segEnd) {
//            return true;
//        } else {
//            return false;
//        }
//    });
//
//    auto itEnd = std::find_if(mMediaFileSlices.begin(), mMediaFileSlices.end(), [&](const MMemoryMediaSlice& slice) {
//        int64_t segStart = slice.mOffset;
//        int64_t segEnd = slice.mSize + segStart - 1;
//        if (rangeEnd >= segStart && rangeEnd <= segEnd) {
//            return true;
//        } else {
//            return false;
//        }
//    });
//
//    if (itStart == mMediaFileSlices.end() ||
//        itEnd == mMediaFileSlices.end()) {
//        return false;
//    }
//
//    if (itStart == itEnd) {
//        int64_t segStart = itStart->mOffset;
//        int64_t segEnd = itStart->mSize + segStart - 1;
//        if (rangeStart >= segStart && rangeEnd <= segEnd) {
//            return true;
//        } else {
//            return false;
//        }
//    } else {
//        auto itNext = std::next(itStart);
//        while (itNext != itEnd && itNext != mMediaFileSlices.end()) {
//            int64_t firstEnd = itStart->mOffset + itStart->mSize;
//            int64_t secondStart = itNext->mOffset;
//            if (firstEnd == secondStart) {
//                exist = true;
//            } else {
//                exist = false;
//                break;
//            }
//            itStart = itNext;
//            itNext = std::next(itStart);
//        }
//    }
//
//
//
//    return exist;
//}
