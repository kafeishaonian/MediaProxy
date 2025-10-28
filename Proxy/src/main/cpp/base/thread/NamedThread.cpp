//
// Created by Hongmingwei on 2025/10/28.
//

#include "NamedThread.h"
#include "STLCommon.h"
#include "GlobalConfig.h"

#include <sys/prctl.h>


namespace proxy{

    NamedThread::NamedThread(): is_thread_started_(false),
    is_detach_(false), is_thread_exit_(false){

    }

    NamedThread::~NamedThread() {

    }

    bool NamedThread::joinable() {
        if (is_detach_) {
            return true;
        } else {
            return thread_.joinable();
        }
    }


    void NamedThread::join() {
        join_internal();
    }

    void NamedThread::set_thread_name(const std::string &name) {
        thread_name_ = name;
    }

    void NamedThread::run(std::function<void()> function, bool detach) {
        function_ = function;
        is_detach_ = detach;

        if (is_detach_) {
            thread_ = std::thread([&](){
                {
                    UniqueLock lock(mutex_);
                    is_thread_started_ = true;
                    condition_.notify_all();
                }
                set_thread_name_internal();
                function_();

                {
                    UniqueLock lock(thread_exit_mutex_);
                    is_thread_exit_ = true;
                    thread_exit_condition_.notify_all();
                }
            });
            thread_.detach();
        } else {
            try {
                thread_ = std::thread([&]{
                    {
                        UniqueLock lock(mutex_);
                        is_thread_started_ = true;
                        condition_.notify_all();
                    }
                    set_thread_name_internal();
                    function_();
                });
            } catch (std::system_error &error) {
                UniqueLock lock(mutex_);
                is_thread_started_ = true;
                condition_.notify_all();
            }
        }

        {
            UniqueLock lock(mutex_);
            condition_.wait(lock, [this](){
                return is_thread_started_;
            });
        }
    }

    void NamedThread::set_thread_name_internal() {
        if (!thread_name_.empty()) {
            prctl(PR_SET_NAME, (unsigned long )thread_name_.c_str());
        }
    }

    std::string &NamedThread::get_thread_name() {
        return thread_name_;
    }


    void NamedThread::join_internal() {
        if (is_detach_) {
            UniqueLock lock(thread_exit_mutex_);
            thread_exit_condition_.wait(lock, [this]() {
                return is_thread_exit_;
            });
        } else {
            try {
                if (thread_.joinable()) {
                    thread_.join();
                }
            } catch (std::system_error &error) {

            }
        }
    }

    void named_thread_set_thread_name(const std::string& thread_name) {
        if (!thread_name.empty()) {
            prctl(PR_SET_NAME, (unsigned long )thread_name.c_str());
        }
    }

}
