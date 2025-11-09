//
// Created by Hongmingwei on 2025/11/8.
//

#ifndef MEDIAPROXY_HTTPSERVERTASKMANAGER_H
#define MEDIAPROXY_HTTPSERVERTASKMANAGER_H

#include <mutex>

class HTTPServerTaskManager {

public:
    HTTPServerTaskManager();

    bool is_task_need_added();
};


#endif //MEDIAPROXY_HTTPSERVERTASKMANAGER_H
