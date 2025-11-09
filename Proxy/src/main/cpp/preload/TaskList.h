//
// Created by Hongmingwei on 2025/11/8.
//

#ifndef MEDIAPROXY_TASKLIST_H
#define MEDIAPROXY_TASKLIST_H

#include <cstdio>
#include <list>
#include <memory>

#include "ThreadUtil.h"
#include "IDownloadTask.h"
#include "PreloadCommon.h"

typedef enum : int {
    TaskListResultNotFound = -1,
    TaskListResultFound = 0,
    TaskListResultSuccess = 1
} TaskListResult;

class TaskList {

public:

    TaskList();

    int pause_all_preload_task();


private:
    std::mutex mutex_;

    int pause_preload_request_count_;

    bool pause_;

    std::list<std::shared_ptr<IDownloadTask>> running_list_;

};


#endif //MEDIAPROXY_TASKLIST_H
