package com.media.proxy.task

class HttpTaskInfo: ITaskInfo() {

    var httpHeader: String? = null      //    url的http header
    var tcpConnectUsedTime: Long = 0    //    Tcp连接所用的时间(毫秒)
    var dnsUsedTime: Long = 0           //    dns消耗的时长(毫秒)
    var httpHeaderTime: Long = 0        //    http header所用的时间(毫秒)
    var httpBodyTime: Long = 0          //    http body所用的时间(毫秒)
    var httpCode: Int = 0               //    收到http 消息头中的状态码  例： 200 404 302
    var cdnIp: String? = null           //    任务的cdnIP
    var dnsServers: String? = null      //    dns servers


}