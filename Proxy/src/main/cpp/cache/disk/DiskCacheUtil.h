//
// Created by Hongmingwei on 2025/10/29.
//

#ifndef MEDIAPROXY_DISKCACHEUTIL_H
#define MEDIAPROXY_DISKCACHEUTIL_H

#include <vector>

#include "FileCacheInterface.h"
#include "DiskCacheCommon.h"

typedef enum : int {
    DiskCacheConfigFileEmpty = -100,
    DiskCacheConfigV1NotExist,
    DiskCacheConfigV2NotExist,
    DiskCacheConfigFileOpenFail,
    DiskCacheConfigJsonFail,
} DiskCacheErrorCode;

class DiskCacheUtil {

public:
    DiskCacheUtil();

    ~DiskCacheUtil();

    static void sort_by_share_num(std::vector<PairCacheConfigFile> &files_vector, bool is_small);

    static void sort_by_share_num(std::vector<CacheFileInfo> &files_vector, bool is_small);

    static bool is_expire(int64_t last_access_time, int factor);

    static bool
    is_valid_file(std::shared_ptr<FileCacheInterface> config_file, int share_num_expire_factor,
                  StringList &file_removed_list, bool is_check_expire);

    static bool is_valid_file(CacheFileInfo &config_file, int share_num_expire_factor,
                              StringList &file_removed_list, bool is_check_expire);

    static std::string get_config_file_full_path_with_version(const std::string &cache_path,
                                                              const std::string &md5_key,
                                                              int version = 1);

    static int get_cache_file_info_form_dir(const std::string &cache_path, const char *dir,
                                            CacheFileInfo &cache_file_info);

    static int is_config_file_with_full_name_exist(const std::string &file_name);

    static int get_all_cached_file_key_and_total_size(const std::string &cache_path_string,
                                                      int64_t &total_cached_size,
                                                      StringList &file_list);

    static int remove_files_at_path(const std::string &cache_path_string, char *path);

    static int remove_files_at_path_with_file_key(const std::string &cache_path_string,
                                                  const std::string &file_key);

    static int remove_all_file_in_absolute_path(const std::string &cache_path_string);

    static int64_t
    remove_cache_with_key(const std::string &cache_path_string, const std::string &file_key);

    static bool is_need_clean(int64_t all_cache_size);

    static int is_cache_path_exist(const std::string &cache_path_string);

    static int is_file_path_exist(const std::string &cache_path_string, const char *file_key);


    static int is_config_file_exist(const std::string &cache_path_string, const char *md5_key);

    static void remove_expire_cache(FileCacheInterfaceMap &files_map,
                                    StringList &file_remove_list,
                                    int64_t &current_cached_size,
                                    bool expired_first_remove);

    static void remove_expire_cache(const std::string &cache_path_string,
                                    std::vector<CacheFileInfo> &total_file_list,
                                    StringList &file_remove_list,
                                    int64_t &current_cached_size,
                                    bool expired_first_remove);

    static void remove_expire_cache_v3(FileCacheInterfaceMap &files_map,
                                       StringList &file_remove_list,
                                       int64_t &current_cached_size);

    static void remove_expire_cache_v3(std::vector<CacheFileInfo> &total_file_list,
                                       StringList &file_remove_list,
                                       int64_t &current_cached_size);

};


#endif //MEDIAPROXY_DISKCACHEUTIL_H
