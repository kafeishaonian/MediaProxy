//
// Created by Hongmingwei on 2025/10/29.
//

#include "HttpServerAdvancedBeast.h"


HttpServerAdvancedBeast::HttpServerAdvancedBeast(
        const std::string &address_string,
        const std::string &doc_root, uint16_t port,
        int threads) : address_string_(address_string),
                       doc_root_(doc_root), port_(port),
                       threads_(threads), listener_(nullptr), io_service_(static_cast<size_t>(threads_)) {

    is_run_done_ = true;
}


HttpServerAdvancedBeast::~HttpServerAdvancedBeast() {
    stop();
}

void HttpServerAdvancedBeast::start() {

}


//mIsRunDone = false;
//auto const address = boost::asio::ip::address::from_string(mAddressString);
//// Create and launch a listening port
//mPort = MGlobalConfig::getInstance()->getServerPort();
//mListener =
//std::make_shared<MHttpServerAdvancedListener>(mIoService, tcp::endpoint{address, mPort}, mDocRoot);
//
//mPort = mListener->getPort();
//bool status = mListener->getStatus();
//
//if (status) {
//
//MGlobalConfig::getInstance()->setServerPort(mPort);
//mListener->start();
//
//// Run the I/O service on the requested number of threads
//for (auto i = mThreads; i > 0; --i) {
//std::shared_ptr<MomoBase::MomoNamedThread> thread = std::make_shared<MomoBase::MomoNamedThread>();
//
//std::stringstream stream;
//stream << "httpServerAdvancedBeast-" << i;
//std::string threadName;
//threadName = stream.str();
//
//thread->setThreadName(threadName);
//
//thread->run([&, i](){
//__MDLOGI_TAG(TAG, "thread:%d start thread_id:%d", i, std::this_thread::get_id());
//mIoService.run();
//__MDLOGI_TAG(TAG, "thread:%d stop...", i);
//});
//
//mThreadList.push_back(thread);
//}
//} else {
//__MDLOGE_TAG(TAG, "Listener start error");
//}
//mIsRunDone = true;
