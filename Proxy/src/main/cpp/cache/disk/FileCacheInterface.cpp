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
#include <cassert>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "MappedFile.h"
#include "Util.h"
#include "ThreadUtil.h"
#include "GlobalConfig.h"
#include "GlobalConstant.h"
#include "JsonParser.h"
#include "BaseUtil.h"
#include "MD5Util.h"


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
    proxy::TimedLock lock(read_write_lock_);
    int result = parse_internal();
    return result;
}

int FileCacheInterface::try_parse() {
    int result = -1;
    int timeout = GlobalConfig::get_instance()->get_cache_lock_timeout_in_ms();
    auto lock = proxy::make_unique_lock(read_write_lock_, std::chrono::milliseconds(timeout));
    if (lock) {
        result = parse_internal();
    }
    return result;
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
    proxy::TimedLock lock(read_write_lock_);

    int64_t current_offset = offset;
    int64_t write_size = 0;
    int64_t remain_size = size;
    while (remain_size > 0) {
        int index = get_index_from_offset(current_offset);
        std::string seg_name = proxy::to_string(index);
        std::string video_file = get_video_file_full_name(seg_name);

        std::shared_ptr<MappedFile> mapped_file = nullptr;
        if (mapped_file_list_.find(seg_name) != mapped_file_list_.end()) {
            mapped_file = mapped_file_list_[seg_name];
        } else {
            mapped_file = std::make_shared<MappedFile>(video_file, mapped_file_segment_size_);
            mapped_file->set_file_offset(index * mapped_file_segment_size_);
            mapped_file_list_[seg_name] = mapped_file;
        }

        int64_t current_map_end_offset = (index + 1) * mapped_file_segment_size_;
        int64_t max_size = current_map_end_offset - current_offset;
        write_size = std::min(max_size, remain_size);

        assert(write_size <= mapped_file_segment_size_);

        write_size = mapped_file->write_data(current_offset, buffer + current_offset - offset,
                                             write_size);
        if (write_size <= 0) {
            break;
        }

        update_segment(current_offset, write_size);
        current_offset = current_offset + write_size;
        remain_size = remain_size - write_size;
    }

    flush_config_file_internal();
    parsed_ = true;
    return write_size;
}

size_t FileCacheInterface::read_data(uint8_t *buffer, uint64_t offset, uint64_t size) {
    proxy::TimedLock lock(read_write_lock_);
    return read_data_internal(buffer, offset, size);
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
    proxy::TimedLock lock(read_write_lock_);
    get_config_json();
}


bool FileCacheInterface::is_cache_complete() {
    proxy::TimedLock lock(read_write_lock_);
    return (cached_size_ >= file_size_ && file_size_ > 0);
}

std::pair<uint64_t, uint64_t> FileCacheInterface::query_remain_data_by_offset(uint64_t offset) {
    proxy::TimedLock lock(read_write_lock_);

    uint64_t current_remain_bytes = 0;
    uint64_t current_needed_bytes = file_size_ - offset;
    uint64_t current_offset = offset;

    auto segment_vector = get_segment_vector();
    for (const auto &segment: segment_vector) {
        uint64_t seg_end_position = segment.seg_start_position_ + segment.seg_file_size_;
        if (offset < segment.seg_start_position_) {
            current_needed_bytes = segment.seg_start_position_ - offset;
            break;
        } else if (offset >= segment.seg_start_position_ && offset < seg_end_position) {
            current_remain_bytes = current_remain_bytes + seg_end_position - current_offset;
            current_offset = seg_end_position;
            current_needed_bytes = current_needed_bytes - current_remain_bytes;
        }
    }
    return std::make_pair(current_remain_bytes, current_needed_bytes);
}

int FileCacheInterface::query_empty_segment(
        std::vector<std::pair<uint64_t, uint64_t>> &empty_segment_vector) {

    proxy::TimedLock lock(read_write_lock_);

    uint64_t offset = 0;
    uint64_t size = file_size_;

    auto segment_vector = get_segment_vector();
    for (auto &segment: segment_vector) {
        if (segment.seg_start_position_ > offset) {
            size = segment.seg_start_position_ - offset;
            empty_segment_vector.push_back(std::make_pair(offset, size));
            offset = segment.seg_start_position_ + segment.seg_file_size_;
        }
    }

    if (offset < file_size_ && file_size_ > 0) {
        size = file_size_ - offset;
        empty_segment_vector.push_back(std::make_pair(offset, size));
    }

    return 0;
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
    for (auto &item: parameter_map) {
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
    char *path_string = const_cast<char *>(path.c_str());
    int result = remove_dir(path_string);
    return result;
}


bool FileCacheInterface::is_cache_complete_internal() {
    return (cached_size_ >= file_size_ && file_size_ > 0);
}


int FileCacheInterface::remove() {

    proxy::TimedLock lock(read_write_lock_);
    int result = 0;

    mapped_file_list_.clear();
    mapped_config_file_.reset();

    if (video_file_path_.empty()) {
        return -1;
    }

    result = remove_files_at_path(video_file_path_);
    return result;
}

std::string &FileCacheInterface::calc_file_sign() {
    proxy::TimedLock lock(read_write_lock_);
    if (file_sign_.empty()) {
        calc_file_sign_iternal();
    }
    return file_sign_;
}


int64_t FileCacheInterface::flush_config_file() {
    proxy::TimedLock lock(read_write_lock_);
    return flush_config_file_internal();
}

int FileCacheInterface::parse_json_with_rapid_json(const std::string &config_file_name,
                                                   const std::string &file_key) {

    if (!mapped_config_file_) {
        mapped_config_file_ = std::make_shared<MappedFile>(config_file_name,
                                                           mapped_config_file_segment_size_);
    }

    std::string json_string = mapped_config_file_->read_string_from_file();
    if (json_string.empty()) {
        return -1;
    }

    proxy::JsonParser json_parser(json_string);
    if (json_parser.parse() != 0) {
        return -1;
    }

    std::string key = json_parser.opt_get_string(FileKey, "");
    if (!file_key_.empty() && key.compare(file_key) != 0) {
        return -1;
    }
    file_key_ = key;

    file_size_ = json_parser.opt_get_uint64(FileSizeKey, 0);
    preload_size_ = json_parser.opt_get_int64(PreloadSizeKey, 0);
    share_num_ = json_parser.opt_get_int64(ShareNumKey, 0);

    audio_duration_ = json_parser.opt_get_int64(PreloadAudioDurationKey, 0);
    video_duration_ = json_parser.opt_get_int64(PreloadVideoDurationKey, 0);
    file_sign_ = json_parser.opt_get_string(FileSign, "");

    seg_num_ = json_parser.opt_get_int(SegNumKey, 0);
    int64_t cached_size = 0;


    for (int i = 0; i < seg_num_; i++) {
        std::string seg_name;
        uint64_t seg_offset;
        uint64_t seg_length;

        {
            std::stringstream stream;
            stream << "/seg_list/" << i << "/seg_name";
            seg_name = json_parser.ptr_get_string(stream.str(), "");
        }
        {
            std::stringstream stream;
            stream << "/seg_list/" << i << "/seg_offset";
            seg_offset = json_parser.ptr_get_uint64(stream.str(), 0);
        }
        {
            std::stringstream stream;
            stream << "/seg_list/" << i << "/seg_length";
            seg_length = json_parser.ptr_get_uint64(stream.str(), 0);
        }

        if (!proxy::is_number(seg_name)) {
            return -1;
        }

        int segIndex = proxy::string_to_int(seg_name);
        segments_map_[segIndex].push_back(CacheSegment(seg_name, seg_offset, seg_length));
        cached_size += seg_length;
    }

    cached_size_ = json_parser.opt_get_int64(CachedSizeKey, 0);
    if (cached_size > 0 && cached_size > cached_size_) {
        cached_size_ = cached_size;
    }
    access_time_ = json_parser.opt_get_uint64(AccessTimeKey, 0);

    return 0;
}

int64_t FileCacheInterface::read_from_local_file(const std::string &seg_name, uint8_t *buffer,
                                                 uint64_t offset, uint64_t size) {
    size_t read_size = 0;
    std::string video_file = get_video_file_full_name(seg_name);
    if (video_file.empty()) {
        return -1;
    }

    std::shared_ptr<MappedFile> mapped_file = nullptr;
    if (mapped_file_list_.find(seg_name) != mapped_file_list_.end()) {
        mapped_file = mapped_file_list_[seg_name];
    } else {
        mapped_file = std::make_shared<MappedFile>(video_file, mapped_file_segment_size_);
        mapped_file_list_[seg_name] = mapped_file;
        int64_t seg_offset = proxy::string_to_int64(seg_name) * mapped_file_segment_size_;
        mapped_file->set_file_offset(seg_offset);
    }
    read_size = mapped_file->read_data(offset, buffer, size);
    return read_size;
}

void FileCacheInterface::update_segment(int64_t current_offset, int64_t write_size) {
    int64_t offset = current_offset;
    int64_t remain_size = write_size;
    do {

        int index = get_index_from_offset(offset);
        int64_t current_map_end_offset = (index + 1) * mapped_file_segment_size_;

        int64_t write_size = std::min(current_map_end_offset - offset, remain_size);

        std::string seg_name = proxy::to_string(index);
        auto segment_vector = segments_map_[index];

        segment_vector.push_back(CacheSegment(seg_name, offset, write_size));

        // 按 offset排序
        std::sort(segment_vector.begin(), segment_vector.end(),
                  [](const CacheSegment &first, const CacheSegment &second) {
                      return (first.seg_start_position_ < second.seg_start_position_);
                  });


        auto output = merge_interval(segment_vector);
        segments_map_[index] = output;

        offset = offset + write_size;
        remain_size = remain_size - write_size;
    } while (remain_size > 0);
}


std::vector<CacheSegment>
FileCacheInterface::merge_interval(const std::vector<CacheSegment> &input) {
    std::vector<CacheSegment> output;

    if (input.empty()) {
        return output;
    }

    int64_t left = input[0].seg_start_position_;
    int64_t right = left + input[0].seg_file_size_;
    std::string seg_name = input[0].seg_name_;

    int size = input.size();
    for (int i = 1; i < size; ++i) {
        int64_t start = input[i].seg_start_position_;
        int64_t end = start + input[i].seg_file_size_;

        if (start > right) {
            output.push_back(CacheSegment(seg_name, left, right - left));
            left = start;
            right = end;
        } else if (end > right) {
            right = end;
        }
    }
    int64_t seg_file_szie = right - left;
    output.push_back(CacheSegment(seg_name, left, seg_file_szie));
    return output;
}


std::string FileCacheInterface::get_config_json() {
    std::string json;
    boost::property_tree::ptree root;

    if (segments_map_.size() == 0) {
        return json;
    }

    root.put(FileKey, file_key_);
    root.put(FileSizeKey, file_size_);
    root.put(PreloadSizeKey, preload_size_);
    root.put(ShareNumKey, share_num_);
    root.put(PreloadAudioDurationKey, audio_duration_);
    root.put(PreloadVideoDurationKey, video_duration_);
    root.put(FileSign, file_sign_);

    int seg_num = 0;
    int64_t cached_size = 0;
    boost::property_tree::ptree elements;
    for (auto &segment_item: segments_map_) {
        for (auto &segment: segment_item.second) {
            seg_num++;
            cached_size += segment.seg_file_size_;
            boost::property_tree::ptree element;
            element.add(SegNameKey, segment.seg_name_);
            element.add(SegOffsetKey, segment.seg_start_position_);
            element.add(SegLengthKey, segment.seg_file_size_);
            elements.push_back(std::make_pair("", element));
        }
    }
    root.put(CachedSizeKey, cached_size);
    if (cached_size > cached_size_) {// update mCachedSize
        cached_size_ = cached_size;
    }
    root.put(SegNumKey, seg_num);
    root.put_child(SegListKey, elements);

    uint64_t current_time = util_get_current_time_in_milli_seconds();
    root.put(AccessTimeKey, current_time);

    std::stringstream stream;
    try {
        boost::property_tree::write_json(stream, root);
    } catch (std::exception &exception) {
        return json;
    }

    json = stream.str();
    return json;
}


std::vector<CacheSegment> FileCacheInterface::get_segment_vector() {
    std::vector<CacheSegment> vector;
    for (auto &item: segments_map_) {
        vector.insert(vector.end(), item.second.begin(), item.second.end());
    }
    return vector;
}


std::string &FileCacheInterface::calc_file_sign_iternal() {
    if (!file_sign_.empty()) {
        return file_sign_;
    }

    if (is_cache_complete_internal()) {
        proxy::MD5Util util;
        uint8_t buffer[8096];
        int64_t current_offset = 0;
        size_t read_size;

        do {
            read_size = read_data_internal(buffer, current_offset, 8096);
            if (read_size > 0) {
                util.update(buffer, (int) read_size);
                current_offset += read_size;
            }
            if ((current_offset >= file_size_ && file_size_ > 0) ||
                read_size == 0) {
                break;
            }
        } while (true);

        file_sign_ = util.get_result();
    }
    return file_sign_;
}

size_t FileCacheInterface::read_data_internal(uint8_t *buffer, uint64_t offset, uint64_t size) {
    int64_t read_size = 0;
    int64_t read_file_size = 0;

    uint64_t read_seg_size = 0;
    int64_t current_offset = offset;
    int64_t current_end = offset + size;
    int64_t remain_size = size;

    do {
        int index = get_index_from_offset(current_offset);

        std::string seg_name = proxy::to_string(index);
        if (segments_map_.find(index) == segments_map_.end()) {
            break;
        }

        read_file_size = 0;

        for (auto &segment: segments_map_[index]) {
            uint64_t seg_start_position = segment.seg_start_position_;
            uint64_t seg_file_size = segment.seg_file_size_;

            if (seg_file_size > mapped_file_segment_size_) {
                seg_file_size = mapped_file_segment_size_;
            }

            uint64_t seg_end_position = seg_start_position + seg_file_size;
            std::string segment_name = segment.seg_name_;
            if (current_offset >= seg_start_position && current_end <= seg_end_position) {
                read_seg_size = current_end - current_offset;
            } else if (current_offset >= seg_start_position && current_offset < seg_end_position &&
                       current_end > seg_end_position) {
                read_seg_size = seg_end_position - current_offset;
            } else {
                continue;
            }

            read_file_size = read_from_local_file(segment_name, buffer + read_size, current_offset,
                                                  read_seg_size);

            if (read_file_size <= 0) {
                break;
            }

            read_size = read_size + read_file_size;
            remain_size = remain_size - read_file_size;
            current_offset = current_offset + read_file_size;

            if (remain_size <= 0) {
                break;
            }
        }
        if (remain_size <= 0) {
            break;
        }

        if (read_file_size <= 0) {
            break;
        }
    } while (true);

    return read_size;
}

int64_t FileCacheInterface::flush_config_file_internal() {
    if (get_update_cached_size() == 0) {
        return -1;
    }

    if (!mapped_config_file_) {
        mapped_config_file_ = std::make_shared<MappedFile>(get_config_file_full_name(),
                                                           mapped_config_file_segment_size_);
    }

    if (is_cache_complete_internal()) {
        calc_file_sign_iternal();
    }

    std::string json = get_config_json();
    if (json.empty()) {
        return -1;
    }

    return mapped_config_file_->write_string_to_file(json);
}

int64_t FileCacheInterface::get_update_cached_size() {
    int64_t cached_size = 0;
    for (auto &segment_tem: segments_map_) {
        for (auto &segment: segment_tem.second) {
            cached_size += segment.seg_file_size_;
        }
    }
    if (cached_size > 0 && cached_size > cached_size_) {
        cached_size_ = cached_size;
    }
    return cached_size_;
}

int FileCacheInterface::parse_internal() {
    if (parsed_) {
        return 0;
    }

    std::string config_file_full_name = get_config_file_full_name();
    if (config_file_full_name.empty()) {
        return -1;
    }
    if (access(config_file_full_name.c_str(), F_OK) != 0) {
        return -1;
    }
    if (parse_json_with_rapid_json(config_file_full_name, file_key_) < 0) {
        return -1;
    }
    parsed_ = true;
    return 0;
}


std::shared_ptr<FileCacheInterface>
create_file_disk_cache(const std::string &cache_path, const std::string &file_key) {
    auto file_disk_cache = std::make_shared<FileCacheInterface>();
    file_disk_cache->set_cache_path(cache_path);
    file_disk_cache->set_file_key(file_key);
    file_disk_cache->set_config_file_name("config.json");
    file_disk_cache->parse();
    return file_disk_cache;
}

