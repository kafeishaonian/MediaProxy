package com.media.proxy.interfaces

import com.media.proxy.task.ITaskInfo

interface OnReportListener {
    /**
     *
     * @param info 获取任务信息、类型（直播或点播）
     */
    fun onReport(info: ITaskInfo?)
}