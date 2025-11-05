//
// Created by Hongmingwei on 2025/10/31.
//

#ifndef MEDIAPROXY_HTTPSERVERSESSIONREQUESTINFO_H
#define MEDIAPROXY_HTTPSERVERSESSIONREQUESTINFO_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

// 任务数据时间记录
struct TaskDataTime {
    int task_id_;        // 任务ID
    int read_data_used_time_;  // 首次读取到数据的用时 ms
    int task_end_used_time_; // 到收到任务完成通知用时 ms
};


enum class SessionEndCode : int {
    read_data_timeout_ = -10,
    reset_by_remote_peer_,
    write_error_,
    send_finish_ = 0
};

/**
 * HTTP 点播会话请求信息类
 * 用于记录 HTTP 点播请求的各种统计信息和状态
 */
class HttpServerSessionRequestInfo {

public:
    // 基本请求信息
    std::string session_id_;              // 会话ID，用于区分多个请求是否为同一次播放
    uint64_t req_start_time_;             // 收到请求的时间戳（毫秒）
    std::string req_url_;                 // 收到的请求URL
    int64_t req_range_start_;             // 请求中的 Range 起始位置
    int64_t req_range_end_;               // 请求中的 Range 结束位置
    std::string req_key_;                 // 请求URL中携带的key，用于唯一标识内容
    
    // 下载相关信息
    std::string download_url_;            // 组合后用于下载数据的URL
    bool is_https_;                       // 下载URL是否为HTTPS
    std::string download_url_header_;     // 下载请求时使用的 HTTP Header
    uint64_t req_content_size_;           // 请求内容的总大小
    
    // 任务管理
    std::vector<int> req_task_list_;      // 本次请求产生的下载任务ID列表
    std::vector<TaskDataTime> task_used_time_;  // 记录每个下载任务从添加到数据就绪的耗时
    
    // 响应信息
    int res_http_code_;                   // 响应的 HTTP 状态码
    uint64_t res_content_len_;            // 响应的内容长度
    std::string res_content_range_;       // 响应的 Content-Range 头
    
    // 重试统计
    int content_size_retry_cnt_;          // 获取 ContentSize 的重试次数
    int content_data_retry_cnt_;          // 读取内容数据失败时的最大重试次数
    int transfer_mode_;                   // 传输模式
    
    // 时间统计（单位：毫秒）
    uint32_t parse_req_used_time_;        // 解析请求头耗时（包括读取全局配置、解析URL、解析Range）
    uint32_t login_server_used_time_;     // 登录服务器耗时
    uint32_t content_size_used_time_;     // 获取ContentSize耗时
    uint32_t build_res_used_time_;        // 组装响应头耗时
    uint32_t first_pkt_used_time_;        // 回复首包耗时（从上一统计结束开始计算）
    uint32_t res_eof_used_time_;          // 全部发送完毕并关闭的耗时
    
    // 数据统计
    uint64_t send_data_bytes_;            // 发送的数据总量（不包含响应头）
    int send_media_header_bytes_;         // 发送的媒体文件头大小
    int last_read_result_;                // 最后一次读取操作的返回值
    std::string last_read_sequence_;      // 最后读取的序号名称
    int start_http_count_;                // 触发HTTP下载任务的次数
    SessionEndCode session_end_code_;     // 会话结束原因
    
    // 点播特定字段
    bool is_cache_complete_;              // 缓存是否完成
    int get_file_size_cost_;              // 获取文件大小的耗时
    
    // 播放器性能统计
    int64_t player_info_init_timestamp_;  // 播放器打开视频的时间戳
    int player_init_request_cost_;        // 播放器初始化到代理应答的耗时
    int64_t player_info_start_timestamp_; // 首次播放开始的时间戳
    int player_start_request_cost_;       // 秒开耗时（首帧显示耗时）
    
    // 其他统计
    int not_found_count_;                 // 404错误次数
    int task_count_;                      // 任务总数
    int session_count_;                   // 会话数
    int wait_count_;                      // 等待次数
    int add_task_id_;                     // 添加的任务ID

    /**
     * 构造函数
     * @param start_time 会话开始时间戳（毫秒）
     */
    HttpServerSessionRequestInfo(uint64_t start_time)
        : req_start_time_(start_time)
        , is_https_(false)
        , req_content_size_(0)
        , res_http_code_(0)
        , res_content_len_(0)
        , content_size_retry_cnt_(0)
        , content_data_retry_cnt_(0)
        , transfer_mode_(-1)
        , parse_req_used_time_(0)
        , login_server_used_time_(0)
        , content_size_used_time_(0)
        , build_res_used_time_(0)
        , first_pkt_used_time_(0)
        , res_eof_used_time_(0)
        , send_data_bytes_(0)
        , send_media_header_bytes_(0)
        , last_read_result_(0)
        , start_http_count_(0)
        , session_end_code_(SessionEndCode::send_finish_)
        , is_cache_complete_(false)
        , get_file_size_cost_(-1)
        , player_info_init_timestamp_(-1)
        , player_init_request_cost_(-1)
        , player_info_start_timestamp_(-1)
        , player_start_request_cost_(-1)
        , not_found_count_(0)
        , task_count_(0)
        , session_count_(0)
        , wait_count_(0)
        , add_task_id_(0)
        , req_range_start_(-1)
        , req_range_end_(-1)
    {
    }
};


#endif //MEDIAPROXY_HTTPSERVERSESSIONREQUESTINFO_H
