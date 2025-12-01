//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSERVERTASK_H
#define MEDIAPROXY_DNSSERVERTASK_H

#include "DNSCommon.h"
#include "DNSHostModel.h"

namespace dns {

    class DNSHostManager;
    class DNSSpeedChecker;
    class DNSServer;

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
        ResolveHostTask(
                const std::string &hostname,
                TaskCallback callback,
                std::function<std::shared_ptr<DNSHostModel>(const std::string&)> resolve_func
        );

        void execute() override;

    private:
        std::function<std::shared_ptr<DNSHostModel>(const std::string&)> resolve_func_;
    };

    class SpeedCheckTask : public DNSServerTask {
    public:
        SpeedCheckTask(
                const std::string &hostname,
                std::shared_ptr<DNSHostModel> host_model,
                std::shared_ptr<DNSSpeedChecker> speed_checker,
                std::shared_ptr<DNSHostManager> host_manager,
                TaskCallback callback = nullptr
        );

        void execute() override;

    private:
        std::shared_ptr<DNSHostModel> host_model_;
        std::shared_ptr<DNSSpeedChecker> speed_checker_;
        std::shared_ptr<DNSHostManager> host_manager_;
    };

    class CacheUpdateTask : public DNSServerTask {
    public:
        CacheUpdateTask(
                const std::string &hostname,
                std::shared_ptr<DNSHostModel> host_model,
                std::shared_ptr<DNSHostManager> host_manager
        );

        void execute() override;

    private:
        std::shared_ptr<DNSHostModel> host_model_;
        std::shared_ptr<DNSHostManager> host_manager_;
    };
}


#endif //MEDIAPROXY_DNSSERVERTASK_H
