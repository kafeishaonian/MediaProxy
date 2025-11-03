//
// Created by Hongmingwei on 2025/10/29.
//

#include "HttpServerAdvancedBeast.h"


HttpServerAdvancedBeast::HttpServerAdvancedBeast(
        const std::string &address_string,
        const std::string &doc_root, uint16_t port,
        int threads) : address_string_(address_string),
                       doc_root_(doc_root), port_(port),
                       threads_(threads), listener_(nullptr), io_context_(static_cast<size_t>(threads_)) {

    is_run_done_ = true;
}


HttpServerAdvancedBeast::~HttpServerAdvancedBeast() {
    stop();
}

void HttpServerAdvancedBeast::start() {
    is_run_done_ = false;

    auto const address = boost::asio::ip::address::from_string(address_string_);
    port_ = GlobalConfig::get_instance()->get_server_port();

    listener_ = std::make_shared<HttpServerAdvancedListener>(io_context_, boost::asio::ip::tcp::endpoint{address, port_}, doc_root_);

    port_ = listener_->get_port();
    bool status = listener_->get_status();
    if (status) {
        GlobalConfig::get_instance()->set_server_port(port_);
        listener_->start();

        for (auto i = threads_; i > 0; i--) {
            std::shared_ptr<proxy::NamedThread> thread = std::make_shared<proxy::NamedThread>();
            std::stringstream stream;
            stream << "httpServerAdvancedBeast-" << i;
            std::string thread_name = stream.str();
            thread->set_thread_name(thread_name);
            thread->run([&, i]() {
                io_context_.run();
            });
            thread_list_.push_back(thread);
        }
    }
    is_run_done_ = true;
}