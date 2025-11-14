//
// Created by Hongmingwei on 2025/11/12.
//

#ifndef MEDIAPROXY_DNSSERVER_H
#define MEDIAPROXY_DNSSERVER_H

#include "DNSHostModel.h"

#include "DNSCommon.h"
#include "DNSHostManager.h"
#include "DNSDataCache.h"
#include "DNSSpeedChecker.h"

namespace dns {

    using TaskCallback = std::function<void(std::shared_ptr<DNSHostModel>, bool,
                                            std::shared_ptr<DNSHostModel>)>;


}

class DNSServer {

};


#endif //MEDIAPROXY_DNSSERVER_H
