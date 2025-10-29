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
    void operator()(T const *p) {
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
        int64_t seg_end_position = seg_start_position + seg_file_size;

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

int64_t MemoryMediaCache::read_data(uint8_t *buffer, uint64_t offset, uint64_t size) {
    proxy::WriteLock lock(read_write_lock_);
    int is_found = 0;
    int64_t read_size = 0;
    MemoryMediaSlice *slice = nullptr;

    uint64_t read_start = offset;
    uint64_t read_end = offset + size;

    uint64_t seg_start = 0;
    uint64_t seg_end = 0;

    uint64_t read_seg_size = 0;
    uint64_t read_offset = 0;

    for (auto it = media_file_slices_.begin(); it != media_file_slices_.end(); it++) {
        uint64_t seg_start_position = it->offset_;
        uint64_t seg_file_size = it->size_;
        uint64_t seg_end_position = seg_start_position + seg_file_size;

        if (read_start >= seg_start_position && read_end <= seg_end_position) {
            read_seg_size = read_end - read_start;
        } else if (read_start >= seg_start_position
                   && read_start < seg_end_position
                   && read_end > seg_end_position) {
            read_seg_size = seg_end_position - read_start;
        } else {
            continue;
        }
        read_offset = read_start - seg_start_position;

        for (int i = 0; i < read_seg_size; i++) {
            buffer[read_size + i] = it->buffer_[i + read_offset];
        }

        read_size += read_seg_size;
        read_start += read_seg_size;
        size -= read_seg_size;
        if (size == 0) {
            break;
        }
    }

    access_time_ = util_get_current_time_in_milli_seconds();

    return read_size;
}

void MemoryMediaCache::set_file_size(uint64_t file_size) {
    proxy::WriteLock lock(read_write_lock_);
    file_size_ = file_size;
}

void MemoryMediaCache::set_file_key(std::string file_key) {
    proxy::WriteLock lock(read_write_lock_);
    file_key_ = file_key;
}

void MemoryMediaCache::serialize() {
    proxy::WriteLock lock(read_write_lock_);

    while (!media_file_slices_.empty()) {
        auto it = media_file_slices_.begin();

        if (it->size_ > 0) {
            DiskCache::get_instance()->write_data(file_key_.c_str(), &it->buffer_[0], it->offset_, it->size_);
        }

        media_file_slices_.erase(it);
    }
}


bool MemoryMediaCache::is_cache_expired() {
    proxy::WriteLock lock(read_write_lock_);
    int64_t current_time = util_get_current_time_in_milli_seconds();
    int64_t delta = current_time - access_time_;
    int64_t expire_time = GlobalConfig::get_instance()->get_memory_expired_time_in_second();
    if (delta > expire_time * 1000) {
        return true;
    } else {
        return false;
    }
}

int64_t MemoryMediaCache::serialize_expired_cache() {
    proxy::WriteLock lock(read_write_lock_);
    int64_t write_size = 0;
    while (!media_file_slices_.empty()) {
        auto it = media_file_slices_.begin();
        if (it->size_ > 0) {
            DiskCache::get_instance()->set_instance_parameter(file_key_, PreloadSizeKey, preload_size_);
            DiskCache::get_instance()->set_instance_parameter(file_key_, PreloadAudioDurationKey, audio_duration_);
            DiskCache::get_instance()->set_instance_parameter(file_key_, PreloadVideoDurationKey, video_duration_);

            DiskCache::get_instance()->write_data(file_key_.c_str(), &it->buffer_[0], it->offset_, it->size_, file_size_);

            write_size += it->size_;
        }
        media_file_slices_.erase(it);
    }
    if (write_size > 0) {
        DiskCache::get_instance()->flush_config_file(file_key_);
    }
    return write_size;
}








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
