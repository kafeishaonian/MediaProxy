package com.media.proxy.task

abstract class ITaskInfo {

    /*任务状态*/
    object TaskStatus {
        var StrTaskStatus: Array<String?>? = arrayOf(
            TaskStatusHin.STR_INIT,
            TaskStatusHin.STR_STARTING,
            TaskStatusHin.STR_PAUSED,
            TaskStatusHin.STR_DELAY_REMOVE,
            TaskStatusHin.STR_DELAY_PAUSED,
            TaskStatusHin.STR_COMPLETE,
            TaskStatusHin.STR_REMOVE
        )

        object StatusCode {
            const val STATUS_INIT: Int = 0
            const val STATUS_STARTING: Int = 1
            const val STATUS_PAUSED: Int = 2
            const val STATUS_DELAY_REMOVE: Int = 3
            const val STATUS_DELAY_PAUSED: Int = 4
            const val STATUS_COMPLETE: Int = 5
            const val STATUS_REMOVE: Int = 6
        }

        object TaskStatusHin {
            const val STR_INIT: String = "未启动"
            const val STR_STARTING: String = "下载中"
            const val STR_PAUSED: String = "暂停"
            const val STR_DELAY_REMOVE: String = "删除中..."
            const val STR_DELAY_PAUSED: String = "暂停中..."
            const val STR_COMPLETE: String = "下载完成"
            const val STR_REMOVE: String = "已删除"
        }
    }

    /*任务优先级，T0优先级最高*/
    object TaskPriority {
        const val T0: Int = 0
        const val T1: Int = 1
        const val T2: Int = 2
    }

    /*任务结束的原因*/
    object TaskEndReason {
        var StrTaskEndReason: Array<String?>? = arrayOf(
            TaskEndReasonHin.STR_NO_END,
            TaskEndReasonHin.STR_NOT_CONNECT,
            TaskEndReasonHin.STR_READ_ERROR,
            TaskEndReasonHin.STR_COMPLETE,
            TaskEndReasonHin.STR_GET_FILESIZE_FAILED,
            TaskEndReasonHin.STR_REMOVE,
            TaskEndReasonHin.STR_PAUSE,
            TaskEndReasonHin.STR_WRITE_DISK_FAILED
        )

        fun getSubReasonString(subcode: Int): String {
            when (subcode) {
                TaskEndReasonSubCode.ErrorOK -> return TaskEndSubReasonHin.STR_OK

                TaskEndReasonSubCode.ErrorFileNotFound -> return TaskEndSubReasonHin.STR_FileNotFound

                TaskEndReasonSubCode.ErrorIO -> return TaskEndSubReasonHin.STR_ErrorIO

                TaskEndReasonSubCode.ErrorCDNDown -> return TaskEndSubReasonHin.STR_ErrorCDNDown

                TaskEndReasonSubCode.ErrorConnectionRefuse -> return TaskEndSubReasonHin.STR_ErrorConnectionRefuse

                TaskEndReasonSubCode.Error403 -> return TaskEndSubReasonHin.STR_Error403

                TaskEndReasonSubCode.Error5XX -> return TaskEndSubReasonHin.STR_Error5XX

                TaskEndReasonSubCode.Error400 -> return TaskEndSubReasonHin.STR_Error400

                TaskEndReasonSubCode.ErrorExit -> return TaskEndSubReasonHin.STR_ErrorExit

                TaskEndReasonSubCode.ErrorEOF -> return TaskEndSubReasonHin.STR_ErrorEOF

                TaskEndReasonSubCode.ErrorTimeOut -> return TaskEndSubReasonHin.STR_ErrorTimeOut

                TaskEndReasonSubCode.ErrorSeek -> return TaskEndSubReasonHin.STR_ErrorSeek

                TaskEndReasonSubCode.ErrorNetUnreach -> return TaskEndSubReasonHin.STR_ErrorNetUnreach

                TaskEndReasonSubCode.ErrorConnectReset -> return TaskEndSubReasonHin.STR_ErrorConnectReset

                TaskEndReasonSubCode.ErrorNotConnect -> return TaskEndSubReasonHin.STR_ErrorNotConnect

                TaskEndReasonSubCode.ErrorAudioCodec -> return TaskEndSubReasonHin.STR_ErrorAudioCodec

                TaskEndReasonSubCode.ErrorVideoCodec -> return TaskEndSubReasonHin.STR_ErrorVideoCodec

                TaskEndReasonSubCode.ErrorProtocol -> return TaskEndSubReasonHin.STR_ErrorProtocol

                TaskEndReasonSubCode.ErrorFileFormat -> return TaskEndSubReasonHin.STR_ErrorFileFormat

                TaskEndReasonSubCode.ErrorP2PConnectPeerNotFound -> return TaskEndSubReasonHin.STR_ErrorP2PConnectPeerNotFound

                TaskEndReasonSubCode.ErrorP2PConnectTimeout -> return TaskEndSubReasonHin.STR_ErrorP2PConnectTimeout

                TaskEndReasonSubCode.ErrorP2PConnectionIsNULL -> return TaskEndSubReasonHin.STR_ErrorP2PConnectionIsNULL

                TaskEndReasonSubCode.ErrorP2PConnectHoleFail -> return TaskEndSubReasonHin.STR_ErrorP2PConnectHoleFail

                TaskEndReasonSubCode.ErrorP2PReadFileNotFound -> return TaskEndSubReasonHin.STR_ErrorP2PReadFileNotFound

                else -> return TaskEndSubReasonHin.STR_ErrorUnKown
            }
        }

        object TaskEndReasonCode {
            const val END_NO_END: Int = 0 //初始
            const val END_NOT_CONNECT: Int = 1 //connect出错
            const val END_READ_ERROR: Int = 2 //read出错
            const val END_COMPLETE: Int = 3 //任务完成
            const val END_GET_FILESIZE_FAILED: Int = 4 //获取文件大小失败
            const val END_REMOVE: Int = 5
            const val END_PAUSE: Int = 6
            const val END_WRITE_FAILED: Int = 7 //写磁盘失败
        }

        object TaskEndReasonHin {
            const val STR_NO_END: String = "初始化,无错误"
            const val STR_NOT_CONNECT: String = "connect失败"
            const val STR_READ_ERROR: String = "读错误"
            const val STR_COMPLETE: String = "下载完成"
            const val STR_GET_FILESIZE_FAILED: String = "获取文件大小失败"
            const val STR_REMOVE: String = "被删除"
            const val STR_PAUSE: String = "被暂停"
            const val STR_WRITE_DISK_FAILED: String = "写磁盘失败"
        }

        object TaskEndReasonSubCode {
            const val ErrorOK: Int = 0
            val ErrorFileNotFound: Int = -1001
            val ErrorIO: Int = -1002
            val ErrorCDNDown: Int = -1003
            val ErrorConnectionRefuse: Int = -1004
            val Error403: Int = -1005
            val Error5XX: Int = -1006
            val Error400: Int = -1007
            val ErrorExit: Int = -1008
            val ErrorEOF: Int = -1009
            val ErrorTimeOut: Int = -1010
            val ErrorSeek: Int = -1011
            val ErrorNetUnreach: Int = -1012
            val ErrorConnectReset: Int = -1013
            val ErrorNotConnect: Int = -1999
            val ErrorAudioCodec: Int = -2001
            val ErrorVideoCodec: Int = -2002
            val ErrorProtocol: Int = -2003
            val ErrorFileFormat: Int = -2004

            val ErrorP2PConnectPeerNotFound: Int = -100
            val ErrorP2PConnectTimeout: Int = -101
            val ErrorP2PConnectionIsNULL: Int = -102
            val ErrorP2PConnectHoleFail: Int = -103
            val ErrorP2PReadFileNotFound: Int = -104
        }

        object TaskEndSubReasonHin {
            const val STR_OK: String = "OK"
            const val STR_FileNotFound: String = "文件未找到"
            const val STR_ErrorIO: String = "I/O错误"
            const val STR_ErrorCDNDown: String = "CDN Down"
            const val STR_ErrorConnectionRefuse: String = "连接被拒绝"
            const val STR_Error403: String = "403"
            const val STR_Error5XX: String = "5XX"
            const val STR_Error400: String = "400"
            const val STR_ErrorExit: String = "Immediate exit was requested"
            const val STR_ErrorEOF: String = "EOF,到达文件末尾"
            const val STR_ErrorTimeOut: String = "TimeOut,超时"
            const val STR_ErrorSeek: String = "Seek失败"
            const val STR_ErrorNetUnreach: String = "Network is unreachable,网络不可达"
            const val STR_ErrorConnectReset: String = "Connect Reset by Peer"
            const val STR_ErrorNotConnect: String = "ErrorNotConnect"
            const val STR_ErrorAudioCodec: String = "AudioCodec Error "
            const val STR_ErrorVideoCodec: String = "VideoCodec Error"
            const val STR_ErrorProtocol: String = "Protocol Error"
            const val STR_ErrorFileFormat: String = "FileFormat Error"
            const val STR_ErrorP2PConnectPeerNotFound: String = "Peer Not Found"
            const val STR_ErrorP2PConnectTimeout: String = "Connect Timeout"
            const val STR_ErrorP2PConnectionIsNULL: String = "Connection is NULL"
            const val STR_ErrorP2PConnectHoleFail: String = "Hole Fail"
            const val STR_ErrorP2PReadFileNotFound: String = "Peer Not Found"
            const val STR_ErrorUnKown: String = "错误未定义"
        }
    }

    object TransferType {
        val TRANSFER_TYPE_INVALID: Int = -1

        const val TRANSFER_TYPE_P2P: Int = 0

        const val TRANSFER_TYPE_HTTP: Int = 1
    }



}