//
// Created by Hongmingwei on 2025/11/7.
//

#include "PreloadManager.h"


PreloadManager *PreloadManager::get_instance() {
    return Singleton<PreloadManager>::get_instance();
}