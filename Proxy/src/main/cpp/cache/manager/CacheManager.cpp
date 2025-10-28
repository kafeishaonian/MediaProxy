//
// Created by Hongmingwei on 2025/10/28.
//

#include "CacheManager.h"

#include "Singleton.h"
#include "SerializeTask.h"


CacheManager *CacheManager::get_instance() {
    return Singleton<CacheManager>::get_instance();
}

CacheManager::CacheManager() {

}

CacheManager::~CacheManager() {

}

void CacheManager::start_serialize_task() {
    SerializeTask::get_instance()->start();
}