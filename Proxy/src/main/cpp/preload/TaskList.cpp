//
// Created by Hongmingwei on 2025/11/8.
//

#include "TaskList.h"

TaskList::TaskList() {
    pause_ = false;
}

int TaskList::pause_all_preload_task() {
    proxy::StdWriteLock lock(mutex_);

    pause_preload_request_count_++;
    if (pause_preload_request_count_ < 1) {
        return TaskListResultSuccess;
    }
    pause_ = true;

    for (auto it = running_list_.begin(); it != running_list_.end();) {
        TASK_PRIORITY priority = (*it)->get_priority();
        if (priority == T1 || priority == T2) {
            (*it)->set_status(STATUS_PAUSING);
            it = running_list_.erase(it);
        } else {
            it++;
        }
    }

    return TaskListResultSuccess;
}