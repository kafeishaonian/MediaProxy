//
// Created by Hongmingwei on 2025/10/25.
//

#include "MappedFile.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>


namespace proxy {
    int get_page_size() {
        int page_size = 4096;
        page_size = getpagesize();
        return page_size;
    }
}

const int DEFAULT_MMAP_SIZE = proxy::get_page_size();


MappedFile::MappedFile(const std::string &file_name, int64_t segment_size) :
        file_name_(file_name),
        file_handler_(-1),
        segment_ptr_(nullptr),
        segment_size_(segment_size),
        file_offset_(-1) {

    file_handler_ = open(file_name.c_str(), O_RDWR | O_CREAT, S_IRWXU);
    if (file_handler_ < 0) {
        close(file_handler_);
        file_handler_ = -1;
    } else {
        struct stat file_stat = {};
        size_t file_size = 0;
        if (fstat(file_handler_, &file_stat) != -1) {
            file_size = static_cast<size_t>(file_stat.st_size);
        }

        if (file_size < segment_size_) {
            if (ftruncate(file_handler_, segment_size_) != 0) {
                return;
            }
            if (zero_file(file_handler_, file_size, segment_size_ - file_size) < 0) {
                return;
            }
        } else {

        }
    }

}

//            mSegmentSize = fileSize;
//            if (mSegmentSize < DEFAULT_MMAP_SIZE || (mSegmentSize % DEFAULT_MMAP_SIZE != 0)) {
//                mSegmentSize = (fileSize / DEFAULT_MMAP_SIZE + 1) * DEFAULT_MMAP_SIZE;
////            __MDLOGD_TAG("MappedFile", "fileSize:%lld, segmentSize:%lld", fileSize, mSegmentSize);
//                if (ftruncate(mFileHandle, mSegmentSize) != 0) {
//                    return;
//                }
//
//                if (zeroFile(mFileHandle, fileSize, mSegmentSize - fileSize) < 0) {
//                    return;
//                }
//            }
//        }
//        __MDLOGD_TAG("MappedFile", "fileSize:%zu, segmentSize:%lld", fileSize, mSegmentSize);
//
//        mSegmentPtr = mmap(nullptr, mSegmentSize, PROT_READ | PROT_WRITE, MAP_SHARED, mFileHandle, 0);
//        if (mSegmentPtr == MAP_FAILED) {
//            close(mFileHandle);
//            mFileHandle = -1;
//            mSegmentPtr = nullptr;
//        }
//    }
//}
//
//MappedFile::~MappedFile() {
//
//    if (mFileHandle >= 0) {
//        close(mFileHandle);
//        mFileHandle = -1;
//    }
//
//    if (mSegmentPtr != MAP_FAILED && mSegmentPtr != nullptr) {
//        munmap(mSegmentPtr, mSegmentSize);
//        mSegmentPtr = nullptr;
//    }
//}
//
//void MappedFile::setFileOffset(int64_t offset)
//{
//    mFileOffset = offset;
//}
//
//int64_t MappedFile::writeData(int64_t offset, uint8_t *buffer, int64_t size)
//{
//    if (mSegmentPtr == MAP_FAILED || !mSegmentPtr) {
//        return -1;
//    }
//
//    if (mFileOffset < 0 || size <= 0 || !buffer) {
//        return -1;
//    }
//
//    int64_t writeSize = 0;
//    int64_t fileEndOffset = mFileOffset + mSegmentSize;
//    if (offset >= mFileOffset && offset < fileEndOffset) {
////        int64_t endOffset = offset + size;
////        if (endOffset < fileEndOffset) {
////            uint8_t * data = (uint8_t *)mSegmentPtr + (int)(offset - mFileOffset);
////            memcpy(data, buffer, size);
////            writeSize = size;
////        } else {
////            uint8_t * data = (uint8_t *)mSegmentPtr + (int)(offset - mFileOffset);
////            writeSize = fileEndOffset - offset;
////            memcpy(data, buffer, writeSize);
////        }
//        uint8_t * data = (uint8_t *)mSegmentPtr + (int)(offset - mFileOffset);
//        writeSize = std::min(size, fileEndOffset - offset);
//        memcpy(data, buffer, writeSize);
//    } else if (offset >= fileEndOffset) {
//        writeSize = 0;
//    } else {
//        int64_t endOffset = offset + size;
//        if (endOffset <= mFileOffset) {
//            writeSize = 0;
//        } else {
//            writeSize = std::min(endOffset - mFileOffset, mSegmentSize);
//            memcpy(mSegmentPtr, buffer + (mFileOffset - offset), writeSize);
//        }
//    }
//    return writeSize;
//}
//
//
//int64_t MappedFile::readData(int64_t offset, uint8_t *buffer, int64_t size)
//{
//    if (mSegmentPtr == MAP_FAILED || !mSegmentPtr) {
//        return -1;
//    }
//    if (mFileOffset < 0) {
//        return -1;
//    }
//
//    int64_t readSize = 0;
//    int64_t fileEndOffset = mFileOffset + mSegmentSize;
//    if (offset < mFileOffset) {
//        readSize = 0;
//    } else if (offset >= fileEndOffset) {
//        readSize = 0;
//    } else {
//        int64_t endOffset = offset + size;
//        readSize = std::min(endOffset - offset, fileEndOffset - offset);
//        uint8_t *data = (uint8_t *)mSegmentPtr + (offset - mFileOffset);
//        memcpy(buffer, data, readSize);
//    }
//    return readSize;
//}
//
//int64_t MappedFile::writeStringToFile(const std::string& content)
//{
//    int64_t size = (int64_t)content.length();
//    if (mSegmentPtr == MAP_FAILED || !mSegmentPtr) {
//        return -1;
//    }
//
//    if (size > mSegmentSize) {
//        int64_t oldSize = mSegmentSize;
//        do {
//            mSegmentSize = mSegmentSize * 2;
//        } while (size > mSegmentSize);
//
//        __MDLOGV_TAG("MappedFile", "Resize mmap, oldSize:%lld, segmentSize:%lld", oldSize, mSegmentSize);
//        if (ftruncate(mFileHandle, mSegmentSize) != 0) {
//        }
//
//        if (zeroFile(mFileHandle, oldSize, mSegmentSize - oldSize) < 0) {
//            return -1;
//        }
//
//        if (munmap(mSegmentPtr, oldSize) != 0) {
//            return -1;
//        }
//
//        mSegmentPtr = mmap(mSegmentPtr, mSegmentSize, PROT_READ | PROT_WRITE, MAP_SHARED, mFileHandle, 0);
//        if (mSegmentPtr == MAP_FAILED) {
//            return -1;
//        }
//    }
//
//    memset(mSegmentPtr, 0, mSegmentSize);
//    memcpy(mSegmentPtr, content.c_str(), size);
//
//    return size;
//}
//
//std::string MappedFile::readStringFromFile()
//{
//    std::string content;
//    if (mSegmentPtr == MAP_FAILED || !mSegmentPtr) {
//        return content;
//    }
//    content.assign((const char*)mSegmentPtr, mSegmentSize);
//    return content;
//}
//
//int MappedFile::zeroFile(int fd, int64_t startPos, int64_t size)
//{
//    if (lseek(fd, startPos, SEEK_SET) < 0) {
//        return -1;
//    }
//
//    const char zeros[4096] = {0};
//    while (size >= sizeof(zeros)) {
//        if (write(fd, zeros, sizeof(zeros)) < 0) {
//            return -1;
//        }
//        size -= sizeof(zeros);
//    }
//
//    if (size > 0) {
//        if (write(fd, zeros, size) < 0) {
//            return -1;
//        }
//    }
//    return 0;
//}
