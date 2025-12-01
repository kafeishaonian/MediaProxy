//
// Created by Hongmingwei on 2025/11/28.
// RAII utilities for resource management
//

#ifndef MEDIAPROXY_DNSRAIIUTILS_H
#define MEDIAPROXY_DNSRAIIUTILS_H

#include <dirent.h>
#include <memory>
#include <fstream>
#include <sstream>

namespace dns {

    // RAII wrapper for DIR*
    class DirectoryHandle {
    public:
        explicit DirectoryHandle(const std::string& path)
            : dir_(opendir(path.c_str())) {
        }

        ~DirectoryHandle() {
            close();
        }

        // Delete copy operations
        DirectoryHandle(const DirectoryHandle&) = delete;
        DirectoryHandle& operator=(const DirectoryHandle&) = delete;

        // Allow move operations
        DirectoryHandle(DirectoryHandle&& other) noexcept
            : dir_(other.dir_) {
            other.dir_ = nullptr;
        }

        DirectoryHandle& operator=(DirectoryHandle&& other) noexcept {
            if (this != &other) {
                close();
                dir_ = other.dir_;
                other.dir_ = nullptr;
            }
            return *this;
        }

        bool is_open() const {
            return dir_ != nullptr;
        }

        DIR* get() const {
            return dir_;
        }

        struct dirent* read() {
            return dir_ ? readdir(dir_) : nullptr;
        }

        void close() {
            if (dir_) {
                closedir(dir_);
                dir_ = nullptr;
            }
        }

    private:
        DIR* dir_;
    };

    // RAII wrapper for file streams with size checking
    class SafeFileReader {
    public:
        SafeFileReader(const std::string& path, std::streamsize max_size = 10 * 1024 * 1024)
            : max_size_(max_size), file_size_(0), valid_(false) {

            file_.open(path, std::ios::binary | std::ios::ate);
            if (!file_.is_open()) {
                return;
            }

            file_size_ = file_.tellg();
            if (file_size_ < 0 || file_size_ > max_size_) {
                file_.close();
                return;
            }

            file_.seekg(0, std::ios::beg);
            valid_ = true;
        }

        ~SafeFileReader() = default;

        // Delete copy operations
        SafeFileReader(const SafeFileReader&) = delete;
        SafeFileReader& operator=(const SafeFileReader&) = delete;

        bool is_valid() const {
            return valid_;
        }

        std::streamsize file_size() const {
            return file_size_;
        }

        std::string read_all() {
            if (!valid_) {
                return "";
            }

            std::ostringstream buffer;
            buffer << file_.rdbuf();
            return buffer.str();
        }

        std::ifstream& get_stream() {
            return file_;
        }

    private:
        std::ifstream file_;
        std::streamsize max_size_;
        std::streamsize file_size_;
        bool valid_;
    };

} // namespace dns

#endif //MEDIAPROXY_DNSRAIIUTILS_H
