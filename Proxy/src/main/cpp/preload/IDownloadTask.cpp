//
// Created by Hongmingwei on 2025/11/8.
//

#include "IDownloadTask.h"


IDownloadTask::IDownloadTask() {

}



TASK_PRIORITY IDownloadTask::get_priority() {
    return task_info_->priority_;
}

void IDownloadTask::set_priority(TASK_PRIORITY priority) {
    task_info_->priority_ = priority;
}

void IDownloadTask::set_status(TASK_STATUS status) {
    if (is_task_ready() && status == STATUS_RUNNING) {
        task_info_->status_ = status;
    } else if (is_task_running() && status == STATUS_PAUSING) {
        task_info_->status_ = status;
    } else if (is_task_running() && status == STATUS_REMOVING) {
        cancel_connect();
        task_info_->status_ = status;
    } else if (is_task_pausing() && status == STATUS_PAUSED) {
        task_info_->status_ = status;
    } else if (get_status() == STATUS_PAUSED && status == STATUS_INITED) {
        task_info_->status_ = status;
    }
}

bool IDownloadTask::is_task_ready() const {
    return get_status() == STATUS_INITED;
}

bool IDownloadTask::is_task_running() const {
    return get_status() == STATUS_RUNNING;
}

bool IDownloadTask::is_task_pausing() const {
    return get_status() == STATUS_PAUSING;
}

TASK_STATUS IDownloadTask::get_status() const {
    return task_info_->status_;
}