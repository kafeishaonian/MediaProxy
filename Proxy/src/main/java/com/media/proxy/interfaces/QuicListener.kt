package com.media.proxy.interfaces

import com.media.proxy.task.ITaskInfo

interface QuicListener {
    fun onQuicReport(info: ITaskInfo?)
}
