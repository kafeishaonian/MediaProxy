//
// Created by Hongmingwei on 2025/11/6.
//

#ifndef MEDIAPROXY_PRELOADRESULT_H
#define MEDIAPROXY_PRELOADRESULT_H


#include "ITaskInfo.h"
#include "HttpTaskInfo.h"


typedef struct IPreloadResult {

    IPreloadResult(TRANSFER_TYPE transfer_type = TRANSFER_TYPE_INVALID)
            : context_(this), transfer_type_(transfer_type) {
        business_type_ = BUSINESS_TYPE_VOD;
    }

    virtual ~IPreloadResult() {

    }

    void set_context(void *context) {
        context_ = context;
    }

    void *get_context() {
        return context_;
    }

    void set_transfer_type(TRANSFER_TYPE transfer_type) {
        transfer_type_ = transfer_type;
    }

    TRANSFER_TYPE get_transfer_type() {
        return transfer_type_;
    }

    void set_business_type(BUSINESS_TYPE business_type) {
        business_type_ = business_type;
    }

    BUSINESS_TYPE get_business_type() {
        return business_type_;
    }

private:

    void *context_;
    TRANSFER_TYPE transfer_type_;
    BUSINESS_TYPE business_type_;
} IPreloadResult;


class HttpPreloadResult : public IPreloadResult {

public:
    HttpPreloadResult() : IPreloadResult(TRANSFER_TYPE_HTTP) {

    }

    ~HttpPreloadResult() {

    }

    void set_info(const HttpTaskInfo &task_info) {
        task_info_ = task_info;
    }

    HttpTaskInfo &get_info() {
        return task_info_;
    }

private:
    HttpTaskInfo task_info_;

};

class PreloadTaskComplete {
public:
    virtual void on_task_complete(std::shared_ptr<IPreloadResult> result) = 0;

    virtual void on_task_receive_data(int task_id, void *buffer, size_t len) {};

    virtual void on_task_finished(int task_id, int code, std::string desc) {};

    virtual void on_task_connect_info(int task_id, std::string ip, double connect_time) {};
};

#endif //MEDIAPROXY_PRELOADRESULT_H
