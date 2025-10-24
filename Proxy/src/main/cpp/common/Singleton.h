//
// Created by Hongmingwei on 2025/10/23.
//

#ifndef MEDIAPROXY_SINGLETON_H
#define MEDIAPROXY_SINGLETON_H

#include <cstdio>
#include <pthread.h>
#include <boost/noncopyable.hpp>

template<typename T>
class Singleton : public boost::noncopyable {

public:
    static T *get_instance() {
        pthread_once(&once_, init);
        return instance_;
    }


private:
    Singleton() = default;

    ~Singleton() = default;

    static void init() {
        instance_ = new T();
    }

    static pthread_once_t once_;
    static T *instance_;
};

template<typename T>
pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;

template<typename T>
T *Singleton<T>::instance_ = NULL;


#endif //MEDIAPROXY_SINGLETON_H
