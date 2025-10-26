//
// Created by Hongmingwei on 2025/10/23.
//

#include "FileCacheInterface.h"

#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ftw.h>
#include <chrono>

#include "MappedFile.h"
#include "Util.h"
#include "ThreadUtil.h"
#include "GlobalConfig.h"
#include "GlobalConstant.h"


FileCacheInterface::FileCacheInterface() :
        preload_size_(-1),
        audio_duration_(-1),
        video_duration_(-1),
        share_num_(0),
        parsed_(false),
        file_size_(0),
        cached_size_(0) {

    mapped_config_file_segment_size_ = proxy::get_page_size();
    mapped_file_segment_size_ = 256 * mapped_config_file_segment_size_;

}


int FileCacheInterface::parse() {
    return -1;
}

int FileCacheInterface::try_parse() {
    return -1;
}

int FileCacheInterface::is_config_file_exist() {
    proxy::TimedLock lock(read_write_lock_);
    return is_config_file_exist_internal();
}

int FileCacheInterface::try_is_config_file_exist() {
    int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
    auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
    int result = -1;
    if (lock) {
        result = is_config_file_exist_internal();
    }
    return result;
}

void FileCacheInterface::add_share_num() {
    proxy::TimedLock lock(read_write_lock_);
    if (share_num_ >= 0) {
        share_num_++;
    } else {
        share_num_ = 0;
    }
}

int64_t FileCacheInterface::get_share_num() {
    proxy::TimedLock lock(read_write_lock_);
    return share_num_;
}

int FileCacheInterface::is_config_file_exist_internal() {
    if (parsed_) {
        return 1;
    }

    std::string config_file_name = get_config_file_full_name();
    if (config_file_name.empty()) {
        return -1;
    }
    if (access(config_file_name.c_str(), F_OK) != 0) {
        return -1;
    }
    return 1;
}

int FileCacheInterface::get_index_from_offset(int64_t offset) {
    int index = static_cast<int>(offset / mapped_file_segment_size_);
    return index;
}

void FileCacheInterface::set_cache_path(const std::string &path) {
    cache_path_ = path;
}

void FileCacheInterface::set_file_key(const std::string &file_key) {
    file_key_ = file_key;
    char md5_key[40];
    util_generate_md5_value((uint8_t *) md5_key, (uint8_t *) file_key_.c_str(),
                            (int) strlen(file_key_.c_str()));
    file_key_md5_ = std::string(md5_key);
    std::stringstream stream;
    stream << cache_path_ << "/" << file_key_md5_;
    video_file_path_ = stream.str();
}

void FileCacheInterface::set_video_file_path(const std::string &path) {
    video_file_path_ = path;
}

void FileCacheInterface::set_config_file_name(const std::string &config_file_name) {
    config_file_name_ = config_file_name;
}

void FileCacheInterface::set_segment_size(int segment_size) {
    mapped_file_segment_size_ = segment_size;
}

size_t FileCacheInterface::write_data(uint8_t *buffer, uint64_t offset, uint64_t size) {
    return 0;
}

size_t FileCacheInterface::read_data(uint8_t *buffer, uint64_t offset, uint64_t size) {
    return 0;
}

std::string FileCacheInterface::generate_seg_name() {
    std::stringstream stream;
    int random = std::rand() % 10000;
    stream << file_key_md5_ << "-" << util_get_current_time_in_micro_seconds() << "-" << random;
    return stream.str();
}

std::string FileCacheInterface::generate_index_name(int64_t offset) {
    int index = offset / mapped_file_segment_size_;
    return proxy::to_string(index);
}

void FileCacheInterface::print_segment() {

}

bool FileCacheInterface::check_info() {
    bool is_valid = false;
    if (cache_path_.empty()) {
        return false;
    }

    if (file_key_.empty()) {
        return false;
    }

    if (video_file_path_.empty()) {
        return false;
    }

    if (is_cache_path_exist() < 0) {
        return false;
    }

    if (is_video_file_path_exist() < 0) {
        return false;
    }
    return true;
}

std::string FileCacheInterface::get_config_file_full_name() {
    if (config_file_full_name_.empty()) {
        std::stringstream stream;
        stream << cache_path_ << "/" << file_key_md5_ << "/" << config_file_name_;
        config_file_full_name_ = stream.str();
    }
    return config_file_full_name_;
}


std::string FileCacheInterface::get_video_file_full_name(const std::string &seg_name) {
    std::string video_file_full_name;
    std::stringstream stream;
    stream << cache_path_ << "/" << file_key_md5_ << "/" << seg_name;
    video_file_full_name = stream.str();
    return video_file_full_name;
}

int FileCacheInterface::is_cache_path_exist() {
    char *cache_path = const_cast<char *>(cache_path_.c_str());
    if (access(cache_path, F_OK) != 0) {
        if (mkdir(cache_path, 0777) != 0) {
            return -1;
        }
    }
    return 0;
}


int FileCacheInterface::is_video_file_path_exist() {
    char *video_file_path = const_cast<char *>(video_file_path_.c_str());
    if (access(video_file_path, F_OK) != 0) {
        if (mkdir(video_file_path, 0777) != 0) {
            return -1;
        }
    }
    return 0;
}

void FileCacheInterface::set_file_size(uint64_t file_size) {
    file_size_ = file_size;
}

std::string &FileCacheInterface::get_file_key() {
    proxy::TimedLock lock(read_write_lock_);
    return file_key_;
}

uint64_t FileCacheInterface::get_file_size() {
    proxy::TimedLock lock(read_write_lock_);
    return file_size_;
}

uint64_t FileCacheInterface::get_access_time() {
    proxy::TimedLock lock(read_write_lock_);
    return access_time_;
}

uint64_t FileCacheInterface::get_cache_size() {
    proxy::TimedLock lock(read_write_lock_);
    return cached_size_;
}

void FileCacheInterface::set_preload_size(int64_t size) {
    proxy::TimedLock lock(read_write_lock_);
    preload_size_ = size;
}

int64_t FileCacheInterface::get_preload_size() {
    proxy::TimedLock lock(read_write_lock_);
    return preload_size_;
}

void FileCacheInterface::set_preload_audio_duration(int64_t duration) {
    proxy::TimedLock lock(read_write_lock_);
    audio_duration_ = duration;
}

int64_t FileCacheInterface::get_preload_audio_duration() {
    proxy::TimedLock lock(read_write_lock_);
    return audio_duration_;
}

void FileCacheInterface::set_preload_video_duration(int64_t duration) {
    proxy::TimedLock lock(read_write_lock_);
    video_duration_ = duration;
}

int64_t FileCacheInterface::get_preload_video_duration() {
    proxy::TimedLock lock(read_write_lock_);
    return video_duration_;
}

int64_t FileCacheInterface::get_instance_parameter(const std::string &parameter_key) {
    proxy::TimedLock lock(read_write_lock_);
    if (parameter_key.compare(CachedSizeKey) == 0) {
        return cached_size_;
    } else if (parameter_key.compare(FileSizeKey) == 0) {
        return file_size_;
    } else if (parameter_key.compare(PreloadAudioDurationKey) == 0) {
        return audio_duration_;
    } else if (parameter_key.compare(PreloadVideoDurationKey) == 0) {
        return video_duration_;
    } else if (parameter_key.compare(PreloadSizeKey) == 0) {
        return preload_size_;
    } else if (parameter_key.compare(ShareNumKey) == 0) {
        return share_num_;
    } else {
        return -1;
    }
}

int64_t FileCacheInterface::try_get_instance_parameter(const std::string &parameter_key) {
    int64_t result = -1;

    int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
    auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
    if (lock) {
        if (parameter_key.compare(CachedSizeKey) == 0) {
            result = cached_size_;
        } else if (parameter_key.compare(FileSizeKey) == 0) {
            result = file_size_;
        } else if (parameter_key.compare(PreloadAudioDurationKey) == 0) {
            result = audio_duration_;
        } else if (parameter_key.compare(PreloadVideoDurationKey) == 0) {
            result = video_duration_;
        } else if (parameter_key.compare(PreloadSizeKey) == 0) {
            result = preload_size_;
        } else if (parameter_key.compare(ShareNumKey) == 0) {
            result = share_num_;
        } else {
            result = -1;
        }
    }
    return result;
}

int FileCacheInterface::try_get_instance_parameter_with_map(Int64Map &map) {
    int result = -1;
    int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
    auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
    if (lock) {
        for (auto &item: map) {
            std::string key = item.first;
            int64_t data = -1;
            if (key.compare(CachedSizeKey) == 0) {
                data = cached_size_;
            } else if (key.compare(FileSizeKey) == 0) {
                data = file_size_;
            } else if (key.compare(PreloadAudioDurationKey) == 0) {
                data = audio_duration_;
            } else if (key.compare(PreloadVideoDurationKey) == 0) {
                data = video_duration_;
            } else if (key.compare(PreloadSizeKey) == 0) {
                data = preload_size_;
            } else if (key.compare(ShareNumKey) == 0) {
                data = share_num_;
            }
            map[key] = data;
            result = 0;
        }
    }
    return result;
}

void FileCacheInterface::set_instance_parameter(const std::string &parameter_key, int64_t value) {
    proxy::TimedLock lock(read_write_lock_);
    if (parameter_key.compare(CachedSizeKey) == 0) {
        cached_size_ = value;
    } else if (parameter_key.compare(FileSizeKey) == 0) {
        file_size_ = value;
    } else if (parameter_key.compare(PreloadAudioDurationKey) == 0) {
        audio_duration_ = value;
    } else if (parameter_key.compare(PreloadVideoDurationKey) == 0) {
        video_duration_ = value;
    } else if (parameter_key.compare(PreloadSizeKey) == 0) {
        preload_size_ = value;
    } else if (parameter_key.compare(ShareNumKey) == 0) {
        share_num_ = value;
    }
}

void FileCacheInterface::set_parameter(const StringMap &parameter_map) {
    proxy::TimedLock lock(read_write_lock_);
    for (auto& item : parameter_map) {
        std::string key = item.first;
        std::string value = item.second;
        if (key.compare(file_key_) == 0) {
            file_key_ = value;
        } else if (key.compare(video_file_path_) == 0) {
            video_file_path_ = value;
        } else if (key.compare(file_key_md5_) == 0) {
            file_key_md5_ = value;
        } else if (key.compare(config_file_name_) == 0) {
            config_file_name_ = value;
        }
    }
}


static int remove_dir(char *path) {
    DIR *dir = opendir(path);

    int status = 0;
    char file_name[1024];
    if (dir) {
        struct dirent *dirent = nullptr;
        while ((dirent = readdir(dir)) != nullptr) {
            if (strcmp(dirent->d_name, ".") != 0 &&
                    strcmp(dirent->d_name, "..") != 0) {
                sprintf(file_name, "%s/%s", path, dirent->d_name);
                remove(file_name);
            }
        }
        remove(path);
        closedir(dir);
    } else {
        status = -1;
    }
    return status;
}

static int remove_callback(const char *path) {
    int result = remove(path);
    return result;
}

int FileCacheInterface::remove_files_at_path(const std::string &path) {
    char* path_string = const_cast<char *>(path.c_str());
    int result = remove_dir(path_string);
    return result;
}


bool FileCacheInterface::is_cache_complete_internal() {
    return (cached_size_ >= file_size_ && file_size_ > 0);
}


