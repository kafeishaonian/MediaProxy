//
// Created by Hongmingwei on 2025/10/28.
//

#ifndef MEDIAPROXY_SERIALIZETASK_H
#define MEDIAPROXY_SERIALIZETASK_H

#include <iostream>
#include <thread>
#include "NamedThread.h"

class SerializeTask {

public:
    static SerializeTask *get_instance();

    SerializeTask();

    ~SerializeTask();

    void start();

    void stop();


private:

    void create_task();

    void do_task();

    bool running_;

    proxy::NamedThread thread_;
};


#endif //MEDIAPROXY_SERIALIZETASK_H
