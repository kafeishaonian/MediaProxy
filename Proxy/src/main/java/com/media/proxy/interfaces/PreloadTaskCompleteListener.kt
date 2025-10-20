package com.media.proxy.interfaces

import com.media.proxy.task.ITaskInfo

interface PreloadTaskCompleteListener {

    /**
     * @param taskId 任务id
     * @param url 预加载的url
     * @param key 预加载的key
     * @param session 预加载的session
     * @param size 预加载的文件大小
     */
    fun onPreloadTaskComplete(taskId: Int, url: String, key: String, session: String, size: Long, taskInfo: ITaskInfo)

    /**
     * @param taskId 任务id
     * @param errorCode 错误码
     * @param url 预加载的url
     * @param key 预加载的key
     * @param session 预加载的session
     * @param size 预加载的文件大小
     */
    fun onPreloadTaskError(taskId: Int, errorCode: Int, url: String, key: String, session: String, size: Long, taskInfo: ITaskInfo)


    fun onGetCurrentUserLocation(): DoubleArray
}

