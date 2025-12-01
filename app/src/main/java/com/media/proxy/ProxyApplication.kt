package com.media.proxy

import android.app.Application
import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.util.Log
import com.dns.cache.DNSManager
import com.dns.cache.LogLevel
import com.dns.cache.NetworkState
import com.dns.cache.configure
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch

/**
 * 应用Application
 * 负责全局初始化工作
 */
class ProxyApplication : Application() {

    companion object {
        private const val TAG = "ProxyApplication"
    }

    private val networkCallback = object : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            Log.d(TAG, "网络可用")
            updateNetworkState()
        }

        override fun onLost(network: Network) {
            Log.d(TAG, "网络断开")
            DNSManager.setNetworkState(NetworkState.NONE)
        }

        override fun onCapabilitiesChanged(
            network: Network,
            networkCapabilities: NetworkCapabilities
        ) {
            Log.d(TAG, "网络状态变化")
            updateNetworkState()
        }
    }

    override fun onCreate() {
        super.onCreate()
        Log.d(TAG, "应用启动")

        // 初始化DNS服务
        initDNS()

        // 注册网络监听
        registerNetworkCallback()

        // 预加载常用域名
        preloadCommonDomains()
    }

    /**
     * 初始化DNS服务
     * 使用DSL配置方式
     */
    private fun initDNS() {
        try {
            DNSManager.init(this)
            DNSManager.configure {
                dohServer = "https://dns.alidns.com/dns-query"
                enableSystemDNS = true
                enableHttpDNS = true
                enableLocalCache = true
                threadCount = 4
                logLevel = LogLevel.DEBUG
                networkState = getCurrentNetworkState()
            }
            Log.d(TAG, "DNS服务初始化成功")
        } catch (e: Exception) {
            Log.e(TAG, "DNS服务初始化失败", e)
        }
    }

    /**
     * 注册网络状态监听
     */
    private fun registerNetworkCallback() {
        try {
            val connectivityManager = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
            val networkRequest = NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .build()
            connectivityManager.registerNetworkCallback(networkRequest, networkCallback)
            Log.d(TAG, "网络监听注册成功")
        } catch (e: Exception) {
            Log.e(TAG, "网络监听注册失败", e)
        }
    }

    /**
     * 更新网络状态
     */
    private fun updateNetworkState() {
        val networkState = getCurrentNetworkState()
        DNSManager.setNetworkState(networkState)
        
        // 根据网络状态智能切换DNS策略
        when (networkState) {
            NetworkState.WIFI -> {
                // WiFi环境：启用HTTP DNS，获得更好的解析效果
                DNSManager.configure {
                    enableSystemDNS = true
                    enableHttpDNS = true
                    dohServer = "https://dns.alidns.com/dns-query"
                }
                Log.d(TAG, "切换到WiFi模式：启用HTTP DNS")
            }
            NetworkState.MOBILE -> {
                // 移动网络：仅使用系统DNS，节省流量
                DNSManager.configure {
                    enableSystemDNS = true
                    enableHttpDNS = false
                }
                Log.d(TAG, "切换到移动网络模式：仅使用系统DNS")
            }
            NetworkState.NONE -> {
                Log.d(TAG, "无网络连接")
            }
            else -> {
                Log.d(TAG, "网络状态未知")
            }
        }
    }

    /**
     * 获取当前网络状态
     */
    private fun getCurrentNetworkState(): NetworkState {
        return try {
            val connectivityManager = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
            val network = connectivityManager.activeNetwork ?: return NetworkState.NONE
            val capabilities = connectivityManager.getNetworkCapabilities(network) ?: return NetworkState.NONE

            when {
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_WIFI) -> NetworkState.WIFI
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_CELLULAR) -> NetworkState.MOBILE
                capabilities.hasTransport(NetworkCapabilities.TRANSPORT_ETHERNET) -> NetworkState.WIFI
                else -> NetworkState.UNKNOWN
            }
        } catch (e: Exception) {
            Log.e(TAG, "获取网络状态失败", e)
            NetworkState.UNKNOWN
        }
    }

    /**
     * 预加载常用域名
     * 在应用启动时预解析，提升后续访问速度
     */
    private fun preloadCommonDomains() {
        // 这里可以配置你的常用域名
        val commonDomains = listOf(
            "www.google.com",
            "www.github.com",
            "api.example.com"
        )

        // 使用协程在后台预加载
        GlobalScope.launch(Dispatchers.IO) {
            try {
                commonDomains.forEach { domain ->
                    DNSManager.resolveHost(domain)
                    Log.d(TAG, "预加载域名: $domain")
                }
                Log.d(TAG, "域名预加载完成")
            } catch (e: Exception) {
                Log.e(TAG, "域名预加载失败", e)
            }
        }
    }

    override fun onTerminate() {
        super.onTerminate()
        Log.d(TAG, "应用终止")
    }

    override fun onLowMemory() {
        super.onLowMemory()
        Log.w(TAG, "内存不足，清理DNS缓存")
        // 内存不足时清理缓存
        try {
            DNSManager.clearCache()
        } catch (e: Exception) {
            Log.e(TAG, "清理缓存失败", e)
        }
    }
}

