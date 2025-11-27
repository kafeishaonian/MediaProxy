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


namespace dns {

    DNSDataCache::DNSDataCache(size_t cache_size) : cache_dir_("/data/local/tmp/dns_cache") {
        memory_cache_ = std::make_unique<LRUCache<std::string, std::string>>(cache_size);

        mkdir(cache_dir_.c_str(), 0755);

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

    void DNSDataCache::save_to_disk(const std::string &filename) {
        std::lock_guard<std::mutex> lock(file_mutex_);

        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"version\": \"1.0\",\n";

        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                now.time_since_epoch()
        ).count();

        oss << "  \"timestamp\": " << timestamp << ",\n";
        oss << "  \"cache\": {\n";

        bool first = true;
        memory_cache_->for_each([&](const std::string &key, const std::string &value) {
            if (!first) {
                oss << ",\n";
            }
            first = false;

            std::string escaped_key = escape_json_string(key);
            std::string escaped_value = escape_json_string(value);

            oss << "    \"" << escaped_key << "\": \"" << escaped_value << "\"";
        });
        oss << "\n  }\n";
        oss << "}\n";

        std::string full_path = cache_dir_ + "/" + filename;
        std::ofstream file(full_path, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "无法创建文件: " + full_path);
            return;
        }
        file << oss.str();
        file.close();

        Logger::log(LogLevel::INFO, "DNSDataCache",
                    "保存缓存到文件: " + filename);
    }

    void DNSDataCache::load_from_disk(const std::string &filename) {
        std::lock_guard<std::mutex> lock(file_mutex_);
        std::string full_path = cache_dir_ + "/" + filename;
        std::ifstream file(full_path, std::ios::binary);
        if (!file.is_open()) {
            Logger::log(LogLevel::WARN, "DNSDataCache",
                        "无法打开文件: " + full_path);
            return;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        file.close();
        std::string json = buffer.str();

        size_t cache_pos = json.find("\"cache\":");
        if (cache_pos == std::string::npos) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "JSON格式错误：找不到cache字段");
            return;
        }

        size_t obj_start = json.find('{', cache_pos);
        if (obj_start == std::string::npos) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "JSON格式错误：cache对象格式错误");
            return;
        }

        int brace_count = 0;
        size_t obj_end = obj_start;
        while (obj_end < json.length()) {
            if (json[obj_end] == '{') {
                brace_count++;
            } else if (json[obj_end] == '}') {
                brace_count--;
                if (brace_count == 0) {
                    break;
                }
            }
            obj_end++;
        }

        if (brace_count != 0) {
            Logger::log(LogLevel::ERROR, "DNSDataCache",
                        "JSON格式错误：括号不匹配");
            return;
        }

        int loaded_count = 0;
        size_t pos = obj_start + 1;

        while (pos < obj_end) {
            while (pos < obj_end && std::isspace(json[pos])) {
                pos++;
            }
            if (pos >= obj_end || json[pos] == '}') {
                break;
            }

            if (json[pos] != '\"') {
                pos++;
                continue;
            }

            size_t key_start = pos + 1;
            size_t key_end = json.find('\"', key_start);
            if (key_end == std::string::npos) {
                break;
            }

            std::string key = unescape_json_string(
                    json.substr(key_start, key_end - key_start));

            pos = json.find(':', key_end);
            if (pos == std::string::npos) {
                break;
            }
            pos++;

            while (pos < obj_end && std::isspace(json[pos])) {
                pos++;
            }

            if (json[pos] != '\"') {
                pos++;
                continue;
            }

            size_t value_start = pos + 1;
            size_t value_end = json.find('\"', value_start);
            if (value_end == std::string::npos) {
                break;
            }

            std::string value = unescape_json_string(
                    json.substr(value_start, value_end - value_start));

            memory_cache_->put(key, value);
            loaded_count++;

            pos = value_end + 1;
            while (pos < obj_end && (json[pos] == ',' || std::isspace(json[pos]))) {
                pos++;
            }
        }
        Logger::log(LogLevel::INFO, "DNSDataCache",
                    "从文件加载缓存: " + filename);
    }

    size_t DNSDataCache::get_cache_size() const {
        return memory_cache_->size();
    }


    void DNSDataCache::set_cache_dir(const std::string &dir) {
        cache_dir_ = dir;
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
                return false;
            }
            file.write(data.c_str(), data.size());
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