//
// Created by Hongmingwei on 2025/11/7.
//

#include "PreloadManager.h"


PreloadManager *PreloadManager::get_instance() {
    return Singleton<PreloadManager>::get_instance();
}


int PreloadManager::pause_all_preload_task() {
    return preload_task_list_.pause_all_preload_task();
}