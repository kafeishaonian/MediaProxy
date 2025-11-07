//
// Created by Hongmingwei on 2025/11/7.
//

#ifndef MEDIAPROXY_PRELOADMANAGER_H
#define MEDIAPROXY_PRELOADMANAGER_H

#include "Singleton.h"

#define DEFAULT_THREAD_NUM 6

class PreloadManager {

public:
    PreloadManager(int thread_num = DEFAULT_THREAD_NUM);

    static PreloadManager *get_instance();

};


#endif //MEDIAPROXY_PRELOADMANAGER_H
