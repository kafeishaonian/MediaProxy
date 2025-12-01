//
// Created by Hongmingwei on 2025/11/12.
//

#include "DNSDataCache.h"

#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <ctype.h>
#include <cerrno>


namespace dns {

    static bool create_directory_recursive(const std::string& path) {
        if (path.empty()) {
            return false;
        }

        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            return S_ISDIR(st.st_mode);
        }

        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos && pos > 0) {
            std::string parent = path.substr(0, pos);
            if (!create_directory_recursive(parent)) {
                return false;
            }
        }

        return mkdir(path.c_str(), 0755) == 0;
    }

    DNSDataCache::DNSDataCache(size_t cache_size) : cache_dir_("/data/local/tmp/dns_cache") {
        memory_cache_ = std::make_unique<LRUCache<std::string, std::string>>(cache_size);

        if (!create_directory_recursive(cache_dir_)) {
            Logger::log(LogLevel::WARN, "DNSDataCache",
                        "创建缓存目录失败: " + cache_dir_);
        }

        Logger::log(LogLevel::INFO, "DNSDataCache",
                    "数据缓存已创建，大小: " + std::to_string(cache_size));
    }

    DNSDataCache::~DNSDataCache() {
        Logger::log(LogLevel::INFO, "DNSDataCache", "数据缓存已销毁");
    }

    void DNSDataCache::save(const std::string &key, const std::string &data) {
        memory_cache_->put(key, data);
        std::string file_path = get_cache_file_path(key);
        if (!write_to_file(file_path, data)) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "写入文件失败: " + file_path);
        }
    }

    std::string DNSDataCache::load(const std::string &key) {
        auto cached = memory_cache_->get(key);
        if (cached.has_value()) {
            Logger::log(LogLevel::DEBUG, "DNSDataCache",
                        "内存缓存命中: " + key);
            return cached.value();
        }

        std::string file_path = get_cache_file_path(key);
        std::string data = read_from_file(file_path);

        if (!data.empty()) {
            memory_cache_->put(key, data);
            Logger::log(LogLevel::DEBUG, "DNSDataCache",
                        "文件缓存命中: " + key);
        }
        return data;
    }

    void DNSDataCache::remove(const std::string &key) {
        memory_cache_->remove(key);

        std::string file_path = get_cache_file_path(key);
        if (unlink(file_path.c_str()) == 0) {
            Logger::log(LogLevel::DEBUG, "DNSDataCache",
                        "删除缓存文件: " + file_path);
        }
    }

    void DNSDataCache::clear() {
        memory_cache_->clear();

        DIR *dir = opendir(cache_dir_.c_str());
        if (dir == nullptr) {
            Logger::log(LogLevel::WARN, "DNSDataCache",
                        "无法打开缓存目录: " + cache_dir_);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            std::string file_path = cache_dir_ + "/" + entry->d_name;
            if (unlink(file_path.c_str()) == 0) {
                Logger::log(LogLevel::DEBUG, "DNSDataCache",
                            "删除缓存文件: " + file_path);
            }
        }

        closedir(dir);
        Logger::log(LogLevel::INFO, "DNSDataCache", "清空缓存");
    }

    size_t DNSDataCache::get_cache_size() const {
        return memory_cache_->size();
    }


    void DNSDataCache::set_cache_dir(const std::string &dir) {
        cache_dir_ = dir;
        if (!create_directory_recursive(cache_dir_)) {
            Logger::log(LogLevel::WARN, "DNSDataCache",
                        "创建缓存目录失败: " + cache_dir_);
        } else {
            Logger::log(LogLevel::INFO, "DNSDataCache",
                        "缓存目录已设置: " + cache_dir_);
        }
    }

    std::string DNSDataCache::get_cache_file_path(const std::string &key) const {
        std::hash<std::string> hasher;
        size_t hash = hasher(key);

        std::ostringstream oss;
        oss << cache_dir_ << "/" << hash << ".cache";
        return oss.str();
    }

    std::string DNSDataCache::get_cache_dir() const {
        return cache_dir_;
    }

    bool DNSDataCache::write_to_file(const std::string &file_path, const std::string &data) {
        std::lock_guard<std::mutex> lock(file_mutex_);

        try {
            std::ofstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                Logger::log(LogLevel::ERROR, "DNSDataCache",
                            "无法打开文件: " + file_path + " (errno: " + std::to_string(errno) + ")");
                return false;
            }
            file.write(data.c_str(), data.size());
            if (!file.good()) {
                Logger::log(LogLevel::ERROR, "DNSDataCache",
                            "写入数据失败: " + file_path);
                return false;
            }
            file.close();

            return true;
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "写入文件异常: " + std::string(e.what()));
            return false;
        }
    }

    std::string DNSDataCache::read_from_file(const std::string &file_path) {
        std::lock_guard<std::mutex> lock(file_mutex_);

        try {
            std::ifstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                return "";
            }

            std::ostringstream oss;
            oss << file.rdbuf();
            file.close();

            return oss.str();
        } catch (const std::exception &e) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "读取文件异常: " + std::string(e.what()));
            return "";
        }
    }

    std::string DNSDataCache::escape_json_string(const std::string &str) {
        std::ostringstream escaped;
        for (char c: str) {
            switch (c) {
                case '\"':
                    escaped << "\\\"";
                    break;
                case '\\':
                    escaped << "\\\\";
                    break;
                case '\n':
                    escaped << "\\n";
                    break;
                case '\r':
                    escaped << "\\r";
                    break;
                case '\t':
                    escaped << "\\t";
                    break;
                default:
                    if (c < 32) {
                        escaped << "\\u" << std::hex << std::setw(4)
                                << std::setfill('0') << (int) c;
                    } else {
                        escaped << c;
                    }
            }
        }
        return escaped.str();
    }

    std::string DNSDataCache::unescape_json_string(const std::string &str) const {
        std::ostringstream unescaped;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '\"':
                        unescaped << '\"';
                        i++;
                        break;
                    case '\\':
                        unescaped << '\\';
                        i++;
                        break;
                    case 'n':
                        unescaped << '\n';
                        i++;
                        break;
                    case 'r':
                        unescaped << '\r';
                        i++;
                        break;
                    case 't':
                        unescaped << '\t';
                        i++;
                        break;
                    default:
                        unescaped << str[i];
                }
            } else {
                unescaped << str[i];
            }
        }
        return unescaped.str();
    }

}