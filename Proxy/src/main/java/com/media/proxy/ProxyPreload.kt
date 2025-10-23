package com.media.proxy

import com.media.proxy.interfaces.OnReportListener
import com.media.proxy.interfaces.PreloadTaskCompleteListener
import com.media.proxy.interfaces.ProxyEventListener
import com.media.proxy.interfaces.ProxyServerResultListener
import com.media.proxy.interfaces.QuicListener
import com.media.proxy.task.HttpTaskInfo
import kotlinx.coroutines.sync.Mutex
import java.util.concurrent.CopyOnWriteArrayList


object ProxyPreload {

    private val taskCompleteListeners = CopyOnWriteArrayList<PreloadTaskCompleteListener>()
    private val serverResultListeners = CopyOnWriteArrayList<ProxyServerResultListener>()
    private val reportListeners = CopyOnWriteArrayList<OnReportListener>()
    private val proxyEventListeners = CopyOnWriteArrayList<ProxyEventListener>()

    private val mutex = Mutex()
    private val maxSize = 3

    private val quicList = LimitedQueue<QuicListener>(maxSize)

    private var init = false
    private var serverStart = false

    init {
        System.loadLibrary("media_proxy")
    }

    /**
     * 设置预加载任务的监听
     */
    fun addOnPreloadTaskCompleteListener(listener: PreloadTaskCompleteListener?) {
        taskCompleteListeners.add(listener)
    }

    /**
     * 设置代理的监听
     */
    fun addProxyServerResultListener(listener: ProxyServerResultListener?) {
        serverResultListeners.add(listener)
    }

    /**
     * 设置代理事件的监听
     */
    fun addProxyEventListener(listener: ProxyEventListener?) {
        proxyEventListeners.add(listener)
    }

    fun addOnReportListener(listener: OnReportListener?) {
        reportListeners.add(listener)
    }

    fun removeOnPreloadTaskCompleteListener(listener: PreloadTaskCompleteListener?) {
        taskCompleteListeners.remove(listener)
    }

    fun removeProxyServerResultListener(listener: ProxyServerResultListener?) {
        serverResultListeners.remove(listener)
    }

    fun removeOnReportListener(listener: OnReportListener?) {
        reportListeners.remove(listener)
    }


    /**
     * 清除代理事件的监听
     */
    fun removeProxyEventListener(listener: ProxyEventListener?) {
        proxyEventListeners.remove(listener)
    }


    fun getCurrentUserLocation(): DoubleArray {
        var location = DoubleArray(2)
        location[0] = 0.0
        location[1] = 0.0
        for (listener in taskCompleteListeners) {
            if (listener != null) {
                location = listener.onGetCurrentUserLocation()
                break
            }
        }
        return location
    }

    fun postEventFromNative(content: String?) {
        content ?: return
        runCatching {
            val taskInfo = HttpTaskInfo()
        }
    }


    /**
     * proxy初始化,在new之后第一个调用
     * cachePath:cache目录
     * ipaddr:ip地址
     * port:端口号
     * preloadThreadNum:预加载线程数
     * httpThreadNum:http server线程数
     */
    private external fun nativeProxyInit(
        cachePath: String, ipaddr: String, port: Int, preloadThreadNum: Int, httpThreadNum: Int
    )

    /**
     * proxy反初始化
     */
    private external fun nativeProxyUnInit()

    /**
     * 启动http server，必须在nativeProxyInit之后调用
     */
    private external fun nativeProxyHttpServerStart()

    /**
     *返回所有任务的信息，没有任务信息返回null
     */
    private external fun nativeProxyUpdateAllTaskInfo(): Array<Any?>

    /**
     *返回处于status状态的任务的信息，没有任务信息返回null
     */
    private external fun nativeProxyUpdateTaskInfoForStatus(status: Int): Array<Any?>

    /**
     * 添加下载任务
     * url:原始url
     * fileKey:url对应的key
     * rangeStart:下载的起始地址
     * rangeSize:下载数据大小
     * httpHeader：http header
     * priority：任务优先级，T0最高，T2最低，管理器先取优先级高的任务执行
     * limiRate: 本任务的限速值,单位：字节/秒
     * sessionID: session ID
     */
    private external fun nativeProxyAddPreloadTaskWithRangeSize(
        url: String,
        fileKey: String,
        rangeStart: Long,
        rangeSize: Long,
        httpHeader: String,
        priority: Int,
        limitRate: Long,
        sessionID: String
    )

    /**
     * 添加下载任务
     * url:原始url
     * fileKey:url对应的key
     * preloadMilliSeconds:预加载的毫秒数
     * httpHeader：http header
     * priority：任务优先级，T0最高，T2最低，管理器先取优先级高的任务执行
     * limiRate: 本任务的限速值,单位：字节/秒
     * sessionID: session ID
     */

    private external fun nativeProxyAddPreloadTaskWithPreloadDuration(
        url: String,
        fileKey: String,
        rangeStart: Long,
        rangeSize: Long,
        preloadMilliSeconds: Long,
        httpHeader: String,
        priority: Int,
        limitRate: Long,
        sessionID: String
    )


    /**
     * 对任务限速，可以在任何时间对任务限速，如任务不在任务列表中，则设置无效。
     * taskId：任务ID
     * limitRate：指定的限速值，单位：字节/秒
     */
    private external fun nativeProxyLimitRate(taskId: Int, limitRate: Long)


    /**
     * 移除任务
     * taskId：移除的任务ID
     */
    private external fun nativeProxyRemovePreloadTaskWithId(taskId: Int)

    /**
     * 移除fileKey对应的所有任务
     * fileKey: url对应的key
     */
    private external fun nativeProxyRemovePreloadTaskWithFileKey(fileKey: String)

    /**
     * 暂停任务
     * taskId：暂停的任务ID
     */
    private external fun nativeProxyPausePreloadTaskWithId(taskId: Int)

    /**
     * 暂停fileKey对应的所有任务
     * fileKey: url对应的key
     */
    private external fun nativeProxyPausePreloadTaskWithFileKey(fileKey: String)

    /**
     * 恢复任务
     * taskId:要恢复的任务
     */
    private external fun nativeProxyResumePreloadTaskWithId(taskId: Int)

    /**
     * 恢复fileKey对应的所有任务
     * fileKey:url对应的key
     */
    private external fun nativeProxyResumePreloadTaskWithFileKey(fileKey: String)

    /**
     * 清除所有任务
     */
    private external fun nativeProxyClearAllPreloadTask()
    /**
     * 暂停所有任务
     */
    private external fun nativeProxyPauseAllPreloadTask()

    /**
     * 恢复所有任务
     */
    private external fun nativeProxyResumeAllPreloadTask()

    /**
     * 获取指定状态的任务数量
     */
    private external fun nativeProxyGetTaskCount(status: Int): Int

    /**
     * 获取proxy启动后，下载的总数据量
     */
    private external fun nativeProxyGetAllDownloadedBytes(): Long

    /**
     * 获取当前的下载速率，单位：字节/秒
     */
    private external fun nativeProxyGetCurAllDownloadRate(): Long


    /**
     * 将播放地址转换为代理地址
     * @param playURL 服务器下发的播放地址
     * @param fileKey
     * @param header
     * @return 代理播放地址
     */
    private external fun nativeProxySwitchPlayURL(playURL: String, fileKey: String, header: String): String

    /**
     * 设置代理播放时加载的视频大小
     * @param size 默认为1M
     */
    private external fun nativeProxyUpdatePlayerPreloadSize(size: Long)

    /**
     * 获取缓存是否存在
     * @param fileKey 文件的key
     * @return 缓存是否存在
     */
    private external fun nativeProxyCheckCacheExist(fileKey: String): Int


    /**
     * 清除缓存
     */
    private external fun nativeProxyClearCache()

    /**
     * 清除所有缓存
     */
    private external fun nativeProxyClearAllCache()

    /**
     * @return 返回产生的session
     */
    private external fun nativeProxyGenerateSession()

    /**
     * 删除指定cache
     * @param fileKey 文件的key
     */
    private external fun nativeClearCacheWithKey(fileKey: String)

    /**
     * 设置Global Config
     * @param config json格式配置字符串
     */
    private external fun nativeProxySetConfig(config: String)

    /**
     * 测试代码，用于测试crash
     */
    private external fun nativeCrash()

    /**
     * 获取对段IP
     * @return
     */
    private external fun nativeGetLivePeerIp()
}








