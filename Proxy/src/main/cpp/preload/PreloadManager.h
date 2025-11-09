//
// Created by Hongmingwei on 2025/11/7.
//

#ifndef MEDIAPROXY_PRELOADMANAGER_H
#define MEDIAPROXY_PRELOADMANAGER_H

#include "Singleton.h"
#include "TaskList.h"

#define DEFAULT_THREAD_NUM 6

class PreloadManager {

public:
    PreloadManager(int thread_num = DEFAULT_THREAD_NUM);

    static PreloadManager *get_instance();

    int pause_all_preload_task();


private:
    TaskList preload_task_list_;
};


#endif //MEDIAPROXY_PRELOADMANAGER_H
