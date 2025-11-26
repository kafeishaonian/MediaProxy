//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSERVERTASK_H
#define MEDIAPROXY_DNSSERVERTASK_H

#include "DNSCommon.h"
#include "DNSHostModel.h"

namespace dns {

    using TaskCallback = std::function<void(std::shared_ptr<DNSHostModel>, bool,
                                            std::shared_ptr<DNSHostModel>)>;

    class DNSServerTask {
    public:
        DNSServerTask(
                DNSServerTaskType task_type,
                const std::string &hostname,
                TaskCallback callback = nullptr
        );

        virtual ~DNSServerTask() = default;

        DNSServerTaskType get_task_type() const;

        std::string get_hostname() const;

        TaskCallback get_callback() const;

        long get_create_time() const;

        int get_priority() const;

        void set_priority(int priority);

        void set_callback(TaskCallback callback);

        virtual void execute() = 0;

        bool operator<(const DNSServerTask &other) const;

    protected:
        DNSServerTaskType task_type_;
        std::string hostname_;
        TaskCallback callback_;
        long create_time_;
        int priority_;
    };

    class ResolveHostTask : public DNSServerTask {
    public:
        ResolveHostTask(const std::string &hostname, TaskCallback callback = nullptr);

        void execute() override;
    };

    class SpeedCheckTask : public DNSServerTask {
    public:
        SpeedCheckTask(
                const std::string &hostname,
                std::shared_ptr<DNSHostModel> host_model,
                TaskCallback callback = nullptr
        );

        void execute() override;

    private:
        std::shared_ptr<DNSHostModel> host_model_;
    };

    class CacheUpdateTask : public DNSServerTask {
    public:
        CacheUpdateTask(
                const std::string &hostname,
                std::shared_ptr<DNSHostModel> host_mode
        );

        void execute() override;

    private:
        std::shared_ptr<DNSHostModel> host_model_;
    };
}


#endif //MEDIAPROXY_DNSSERVERTASK_H
