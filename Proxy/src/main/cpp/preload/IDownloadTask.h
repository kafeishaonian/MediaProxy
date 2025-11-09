//
// Created by Hongmingwei on 2025/11/8.
//

#ifndef MEDIAPROXY_IDOWNLOADTASK_H
#define MEDIAPROXY_IDOWNLOADTASK_H

#include "PreloadCommon.h"
#include "ITaskInfo.h"

class IDownloadTask {

public:

    virtual void cancel_connect() = 0;

    IDownloadTask();

    TASK_PRIORITY get_priority();

    void set_priority(TASK_PRIORITY priority);

    void set_status(TASK_STATUS status);

    bool is_task_ready() const;

    bool is_task_running() const;

    bool is_task_pausing() const;

    TASK_STATUS get_status() const;
private:
    std::shared_ptr<ITaskInfo> task_info_;

};


#endif //MEDIAPROXY_IDOWNLOADTASK_H
