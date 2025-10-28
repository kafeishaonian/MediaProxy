//
// Created by Hongmingwei on 2025/10/28.
//

#ifndef MEDIAPROXY_NAMEDTHREAD_H
#define MEDIAPROXY_NAMEDTHREAD_H

#include <string>
#include <thread>

namespace proxy {
    class NamedThread {
    public:
        NamedThread();

        ~NamedThread();

        bool joinable();

        void join();

        void set_thread_name(const std::string &name);

        std::string &get_thread_name();

        void run(std::function<void()> function, bool detach = false);

    private:
        void set_thread_name_internal();

        void join_internal();

    private:
        std::thread thread_;
        std::string thread_name_;
        std::function<void()> function_;
        bool is_thread_started_;
        std::mutex mutex_;
        std::condition_variable condition_;
        bool is_detach_;

        bool is_thread_exit_;
        std::mutex thread_exit_mutex_;
        std::condition_variable thread_exit_condition_;
    };

    void named_thread_set_thread_name(const std::string& thread_name);
}

#endif //MEDIAPROXY_NAMEDTHREAD_H
