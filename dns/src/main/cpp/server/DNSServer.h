//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSERVER_H
#define MEDIAPROXY_DNSSERVER_H

#include <thread>
#include <atomic>

#include "DNSCommon.h"
#include "DNSHostManager.h"
#include "DNSSpeedChecker.h"
#include "DNSBlockingQueue.h"
#include "DNSServerTask.h"
#include "DNSServerHandle.h"

namespace dns {

    class DNSServer {
    public:
        struct ServerStats {
            int total_requests;
            int success_requests;
            int failed_requests;
            int cached_requests;
            int queue_size;
            int thread_count;
        };

        DNSServer();

        ~DNSServer();

        void start();

        void stop();

        bool is_running() const;

        std::shared_ptr<DNSHostModel> resolve_sync(const std::string &hostname);

        void resolve_async(const std::string &hostname, TaskCallback callback);


        void add_server_handle(DNSServerType type, std::shared_ptr<DNSServerHandle> handle);

        void remove_server_handle(DNSServerType type);

        void set_thread_count(int count);

        void set_queue_size(size_t size);

        void set_speed_checker(std::shared_ptr<DNSSpeedChecker> checker);

        void set_host_manager(std::shared_ptr<DNSHostManager> manager);

        std::shared_ptr<DNSHostManager> get_host_manager() const;

        ServerStats get_stats() const;

    private:
        void worker_loop();

        void process_task(std::shared_ptr<DNSServerTask> task);

        std::shared_ptr<DNSHostModel> perform_resolve(const std::string &hostname);

        DNSServer(const DNSServer &) = delete;

        DNSServer &operator=(const DNSServer &) = delete;

    private:

        std::map<DNSServerType, std::shared_ptr<DNSServerHandle>> server_handlers_;

        std::shared_ptr<DNSBlockingQueue<std::shared_ptr<DNSServerTask>>> task_queue_;

        std::vector<std::shared_ptr<std::thread>> worker_threads_;

        std::shared_ptr<DNSHostManager> host_manager_;

        std::shared_ptr<DNSSpeedChecker> speed_checker_;

        std::atomic<bool> running_;

        int thread_count_;

        mutable std::mutex stats_mutex_;

        std::atomic<int> total_requests_;

        std::atomic<int> success_requests_;

        std::atomic<int> failed_requests_;

        std::atomic<int> cached_requests_;
    };

}


#endif //MEDIAPROXY_DNSSERVER_H