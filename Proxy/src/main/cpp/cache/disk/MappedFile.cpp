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
            segment_size_ = file_size;
            if (segment_size_ < DEFAULT_MMAP_SIZE || segment_size_ % DEFAULT_MMAP_SIZE != 0) {
                segment_size_ = (file_size / DEFAULT_MMAP_SIZE + 1) * DEFAULT_MMAP_SIZE;

                if (ftruncate(file_handler_, segment_size_) != 0) {
                    return;
                }

                if (zero_file(file_handler_, file_size, segment_size_ - file_size) < 0) {
                    return;
                }
            }
        }

        segment_ptr_ = mmap(nullptr, segment_size_, PROT_READ | PROT_WRITE, MAP_SHARED,
                            file_handler_, 0);
        if (segment_ptr_ == MAP_FAILED) {
            close(file_handler_);
            file_handler_ = -1;
            segment_ptr_ = nullptr;
        }
    }
}

MappedFile::~MappedFile() {
    if (file_handler_ >= 0) {
        close(file_handler_);
        file_handler_ = -1;
    }

    if (segment_ptr_ != MAP_FAILED && segment_ptr_ != nullptr) {
        munmap(segment_ptr_, segment_size_);
        segment_ptr_ = nullptr;
    }

}

void MappedFile::set_file_offset(int64_t offset) {
    file_offset_ = offset;
}


int64_t MappedFile::write_data(int64_t offset, uint8_t *buffer, int64_t size) {
    if (segment_ptr_ == MAP_FAILED || !segment_ptr_) {
        return -1;
    }

    if (file_offset_ < 0 || size <= 0 || !buffer) {
        return -1;
    }

    int64_t write_size = 0;
    int64_t file_end_offset = file_offset_ + segment_size_;
    if (offset >= file_offset_ && offset < file_end_offset) {
        uint8_t *data = (uint8_t *)segment_ptr_ + (offset - file_offset_);
        write_size = std::min(size, file_end_offset - offset);
        memcpy(data, buffer, write_size);
    } else if (offset >= file_end_offset) {
        write_size = 0;
    } else {
        int64_t end_offset = offset + size;
        if (end_offset <= file_offset_) {
            write_size = 0;
        } else {
            write_size = std::min(end_offset - file_offset_, segment_size_);
            memcpy(segment_ptr_, buffer + (file_offset_ - offset), write_size);
        }
    }
    return write_size;
}

int64_t MappedFile::read_data(int64_t offset, uint8_t *buffer, int64_t size) {
    if (segment_ptr_ == MAP_FAILED || !segment_ptr_) {
        return -1;
    }
    if (file_offset_ < 0) {
        return -1;
    }

    int64_t read_size = 0;
    int64_t file_end_offset = file_offset_ + segment_size_;
    if (offset < file_offset_ || offset >= file_end_offset) {
        read_size = 0;
    } else {
        int64_t end_offset = offset + size;
        read_size = std::min(end_offset - offset, file_end_offset - offset);
        uint8_t *data = (uint8_t *)segment_ptr_ + (offset - file_offset_);
        memcpy(buffer, data, read_size);
    }
    return read_size;
}

int64_t MappedFile::write_string_to_file(const std::string &content) {
    int64_t size = content.length();
    if (segment_ptr_ == MAP_FAILED || !segment_ptr_) {
        return -1;
    }

    if (size > segment_size_) {
        int64_t old_size = segment_size_;
        do {
            segment_size_ = segment_size_ * 2;
        } while (size > segment_size_);

        if (zero_file(file_handler_, old_size, segment_size_ - old_size) < 0) {
            return -1;
        }

        if (munmap(segment_ptr_, old_size) != 0) {
            return -1;
        }
        segment_ptr_ = mmap(segment_ptr_, segment_size_, PROT_READ | PROT_WRITE, MAP_SHARED, file_handler_, 0);
        if (segment_ptr_ == MAP_FAILED) {
            return -1;
        }
    }
    memset(segment_ptr_, 0, segment_size_);
    memcpy(segment_ptr_, content.c_str(), size);
    return size;
}

std::string MappedFile::read_string_from_file() {
    std::string content;
    if (segment_ptr_ == MAP_FAILED || !segment_ptr_) {
        return content;
    }
    content.assign((const char*)segment_ptr_, segment_size_);
    return content;
}

int MappedFile::zero_file(int fd, int64_t start_post, int64_t size) {
    if (lseek(fd, start_post, SEEK_SET) < 0) {
        return -1;
    }

    const char zeros[4096] = {0};
    while (size >= sizeof(zeros)) {
        if (write(fd, zeros, sizeof(zeros)) < 0) {
            return -1;
        }
        size -= sizeof(zeros);
    }
    if (size > 0) {
        if (write(fd, zeros, size) < 0) {
            return -1;
        }
    }
    return 0;
}
