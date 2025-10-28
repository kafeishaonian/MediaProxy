//
// Created by Hongmingwei on 2025/10/28.
//

#include "SerializeTask.h"
#include "Singleton.h"
#include "MemoryCache.h"

static std::once_flag once_flag_;

SerializeTask *SerializeTask::get_instance() {
    return Singleton<SerializeTask>::get_instance();
}

SerializeTask::SerializeTask() {
    running_ = false;
}

SerializeTask::~SerializeTask() {

}

void SerializeTask::start() {
    std::call_once(once_flag_, std::bind(&SerializeTask::create_task, this));
}

void SerializeTask::create_task() {
    running_ = true;
    thread_.set_thread_name("proxy-serialize-task");
    thread_.run(std::bind(&SerializeTask::do_task, this));
}

void SerializeTask::do_task() {
    while (running_) {
        MemoryCache::get_instance()->serialize_expired_cache();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}