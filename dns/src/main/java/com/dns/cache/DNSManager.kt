package com.dns.cache

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

object DNSManager{

    init {
        runCatching {
            System.loadLibrary("dns_cache")
        }
    }

    private var initialized = false

    /**
     * 初始化DNS服务
     */
    fun init(context: Context) {
        if (initialized) return

        val cacheDir = context.cacheDir.absolutePath + "/dns"
        nativeSetCacheDir(cacheDir)

        nativeInit()
        initialized = true
    }

    /**
     * 同步解析主机名（阻塞调用）
     */
    fun resolveHost(hostname: String): String {
        checkInitialized()
        return nativeResolveHost(hostname)
    }

    /**
     * 协程方式解析主机名
     */
    suspend fun resolveHostSuspend(hostname: String): String = withContext(Dispatchers.IO) {
        resolveHost(hostname)
    }

    /**
     * 异步解析主机名
     */
    fun resolveHostAsync(hostname: String, onResult: (ip: String, success: Boolean) -> Unit) {
        checkInitialized()
        nativeResolveHostAsync(hostname, object : DNSCallback {
            override fun onResult(ip: String, success: Boolean) {
                onResult(ip, success)
            }
        })
    }

    /**
     * 获取所有IP地址
     */
    fun getAllIPs(hostname: String): List<String> {
        checkInitialized()
        return nativeGetAllIPs(hostname).toList()
    }

    /**
     * 设置DoH服务器
     */
    fun setDohServer(server: String = "https://dns.google/dns-query") {
        nativeSetDohServer(server)
    }

    /**
     * 设置网络状态
     */
    fun setNetworkState(state: NetworkState) {
        nativeSetNetworkState(state.value)
    }

    /**
     * 启用/禁用系统DNS
     */
    fun enableSystemDNS(enable: Boolean) {
        nativeEnableSystemDNS(enable)
    }

    /**
     * 启用/禁用HTTP DNS
     */
    fun enableHttpDNS(enable: Boolean) {
        nativeEnableHttpDNS(enable)
    }

    /**
     * 启用/禁用本地缓存
     */
    fun enableLocalCache(enable: Boolean) {
        nativeEnableLocalCache(enable)
    }

    /**
     * 设置工作线程数
     */
    fun setThreadCount(count: Int) {
        nativeSetThreadCount(count)
    }

    /**
     * 清空所有缓存
     */
    fun clearCache() {
        nativeClearCache()
    }

    /**
     * 设置日志级别
     */
    fun setLogLevel(level: LogLevel) {
        nativeSetLogLevel(level.value)
    }

    private fun checkInitialized() {
        if (!initialized) {
            throw IllegalStateException("DNSManager not initialized. Call init(context) first.")
        }
    }

    /**
     * 初始化DNS服务
     */
    private external fun nativeInit()

    /**
     * 同步解析主机名
     */
    private external fun nativeResolveHost(hostname: String): String

    /**
     * 异步解析主机名
     */
    private external fun nativeResolveHostAsync(hostname: String, callback: DNSCallback)

    /**
     * 获取所有IP地址
     */
    private external fun nativeGetAllIPs(hostname: String): Array<String>

    /**
     * 设置DoH服务器
     */
    private external fun nativeSetDohServer(server: String)

    /**
     * 设置网络状态
     */
    private external fun nativeSetNetworkState(state: Int)

    /**
     * 启用/禁用系统DNS
     */
    private external fun nativeEnableSystemDNS(enable: Boolean)

    /**
     * 启用/禁用HTTP DNS
     */
    private external fun nativeEnableHttpDNS(enable: Boolean)

    /**
     * 启用/禁用本地缓存
     */
    private external fun nativeEnableLocalCache(enable: Boolean)

    /**
     * 设置工作线程数
     */
    private external fun nativeSetThreadCount(count: Int)

    /**
     * 设置缓存目录
     */
    private external fun nativeSetCacheDir(dir: String)

    /**
     * 清空缓存
     */
    private external fun nativeClearCache()

    /**
     * 设置日志级别
     */
    private external fun nativeSetLogLevel(level: Int)
}


/**
 * DNS回调接口
 */
interface DNSCallback {
    /**
     * 解析结果回调
     */
    fun onResult(ip: String, success: Boolean)
}

/**
 * 网络状态枚举
 */
enum class NetworkState(val value: Int) {
    UNKNOWN(0),
    WIFI(1),
    MOBILE(2),
    NONE(3)
}

/**
 * 日志级别枚举
 */
enum class LogLevel(val value: Int) {
    DEBUG(0),
    INFO(1),
    WARN(2),
    ERROR(3)
}