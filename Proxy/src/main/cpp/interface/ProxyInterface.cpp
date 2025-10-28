//
// Created by Hongmingwei on 2025/10/23.
//

#include "ProxyInterface.h"

ProxyInterface::ProxyInterface() {
    server_status_ = ServerStatus_Create;
}

ProxyInterface::~ProxyInterface() {

}

std::shared_ptr<ProxyInterface> &ProxyInterface::get_instance() {
    return SingletonShared<ProxyInterface>::get_instance();
}

void ProxyInterface::init() {
    server_status_ = ServerStatus_Init;
}

void ProxyInterface::un_init() {
    server_status_ = ServerStatus_UnInit;


}


int ProxyInterface::setup_cache(const char *path) {
    DiskCache::get_instance()->set_cache_path(path);

    MCacheManager::getInstance()->startSerializeTask();
    return 0;
}