//
// Created by Hongmingwei on 2025/10/24.
//

#ifndef MEDIAPROXY_PRELOADCOMMON_H
#define MEDIAPROXY_PRELOADCOMMON_H


#include <string>


typedef enum task_priority {
    T0 = 0,
    T1,
    T2,
} TASK_PRIORITY;

typedef enum task_status {
    STATUS_INITED = 0,
    STATUS_RUNNING,
    STATUS_PAUSING,
    STATUS_PAUSED,
    STATUS_REMOVING,
    STATUS_REMOVED,
    STATUS_COMPLETE,

    STATUS_STARTING,
    STATUS_DELAY_REMOVE,
    STATUS_DELAY_PAUSED,

    STATUS_REMOVE
} TASK_STATUS;


typedef enum task_end_reason {
    END_NO_END = 0,      //初始
    END_NOT_CONNECT,     //connect出错
    END_READ_ERROR,      //read出错
    END_COMPLETE,        //下载完成
    END_GET_FILESIZE_FAILED,//获取文件大小失败
    END_REMOVE,          //被删除
    END_PAUSE,           //被暂停
    END_WRITE_FAILED,    //写磁盘失败
    END_SEEK_ERROR,      //seek错误
    END_MEMORY_ERROR,    //内存错误
} TASK_END_REASON;



typedef enum : int {
    TRANSFER_TYPE_INVALID = -1,

    TRANSFER_TYPE_HTTP
} TRANSFER_TYPE;

#endif //MEDIAPROXY_PRELOADCOMMON_H
