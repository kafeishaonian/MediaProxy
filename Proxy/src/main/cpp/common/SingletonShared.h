//
// Created by Hongmingwei on 2025/10/23.
//

#ifndef MEDIAPROXY_SINGLETONSHARED_H
#define MEDIAPROXY_SINGLETONSHARED_H

#include <cstdio>
#include <pthread.h>
#include <boost/noncopyable.hpp>

#include "SpinMutex.h"

template<typename T>
class SingletonShared : public boost::noncopyable {
public:
    static std::shared_ptr<T>& get_instance() {
        if (!instance_) {
            std::lock_guard<SpinMutex> auto_lock(mutex_);
            pthread_once(&once_, init);
        }
        return instance_;
    }

private:
    SingletonShared() = default;
    ~SingletonShared() = default;

    static void init() {
        instance_ = std::make_shared<T>();
    };


    static pthread_once_t once_;
    static std::shared_ptr<T> instance_;
    static SpinMutex mutex_;

};

template<typename T>
pthread_once_t SingletonShared<T>::once_ = PTHREAD_ONCE_INIT;

template<typename T>
std::shared_ptr<T> SingletonShared<T>::instance_ = NULL;

template<typename T>
SpinMutex SingletonShared<T>::mutex_;

#endif //MEDIAPROXY_SINGLETONSHARED_H
