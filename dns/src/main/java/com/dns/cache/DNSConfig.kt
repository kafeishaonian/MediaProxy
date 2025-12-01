package com.dns.cache

/**
 * DNS配置类
 * 使用DSL模式配置DNS服务
 */
class DNSConfig {
    /**
     * DoH服务器URL
     */
    var dohServer: String = "https://user_id.alidns.com/dns-query"

    /**
     * 是否启用系统DNS
     */
    var enableSystemDNS: Boolean = true

    /**
     * 是否启用HTTP DNS
     */
    var enableHttpDNS: Boolean = true

    /**
     * 是否启用本地缓存
     */
    var enableLocalCache: Boolean = true

    /**
     * 缓存过期时间（秒）
     */
    var cacheExpireTime: Int = 3600

    /**
     * 线程数
     */
    var threadCount: Int = 4

    /**
     * 日志级别
     */
    var logLevel: LogLevel = LogLevel.INFO

    /**
     * 网络状态
     */
    var networkState: NetworkState = NetworkState.UNKNOWN
}

/**
 * DSL配置扩展函数
 */
fun DNSManager.configure(block: DNSConfig.() -> Unit) {
    val config = DNSConfig().apply(block)

    setDohServer(config.dohServer)
    enableSystemDNS(config.enableSystemDNS)
    enableHttpDNS(config.enableHttpDNS)
    enableLocalCache(config.enableLocalCache)
    setThreadCount(config.threadCount)
    setLogLevel(config.logLevel)
    setNetworkState(config.networkState)
}

/**
 * 快速配置预设
 */
object DNSPresets {
    /**
     * 默认配置
     */
    val DEFAULT = DNSConfig().apply {
        dohServer = "https://dns.alidns.com/dns-query"
        enableSystemDNS = true
        enableHttpDNS = true
        enableLocalCache = true
        threadCount = 4
        cacheExpireTime = 7200
        logLevel = LogLevel.INFO
    }

    /**
     * 高性能配置
     */
    val HIGH_PERFORMANCE = DNSConfig().apply {
        dohServer = "https://dns.google/dns-query"
        enableSystemDNS = true
        enableHttpDNS = true
        enableLocalCache = true
        threadCount = 8
        cacheExpireTime = 3600
        logLevel = LogLevel.INFO
    }

    /**
     * 调试配置
     */
    val DEBUG = DNSConfig().apply {
        dohServer = "https://dns.alidns.com/dns-query"
        enableSystemDNS = true
        enableHttpDNS = true
        enableLocalCache = true
        threadCount = 2
        logLevel = LogLevel.DEBUG
    }

    /**
     * 仅系统DNS
     */
    val SYSTEM_ONLY = DNSConfig().apply {
        enableSystemDNS = true
        enableHttpDNS = false
        enableLocalCache = true
        threadCount = 2
        logLevel = LogLevel.INFO
    }

    /**
     * 省电模式
     */
    val BATTERY_SAVER = DNSConfig().apply {
        enableSystemDNS = true
        enableHttpDNS = false
        enableLocalCache = true
        threadCount = 1
        cacheExpireTime = 14400
        logLevel = LogLevel.WARN
    }
}

/**
 * 应用预设配置
 */
fun DNSManager.applyPreset(preset: DNSConfig) {
    setDohServer(preset.dohServer)
    enableSystemDNS(preset.enableSystemDNS)
    enableHttpDNS(preset.enableHttpDNS)
    enableLocalCache(preset.enableLocalCache)
    setThreadCount(preset.threadCount)
    setLogLevel(preset.logLevel)
    setNetworkState(preset.networkState)
}
