# DNS模块使用指南

## 快速开始

### 1. 基本初始化

```kotlin
class MyApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        
        // 基本初始化
        DNSManager.init(this)
        
        // 配置DoH服务器
        DNSManager.setDohServer("https://dns.alidns.com/dns-query")
        DNSManager.enableHttpDNS(true)
    }
}
```

### 2. 使用预设配置

```kotlin
// 使用默认配置
DNSManager.init(this)
DNSManager.configure {
    dohServer = DNSPresets.DEFAULT.dohServer
    enableSystemDNS = true
    enableHttpDNS = true
    enableLocalCache = true
    threadCount = 4
    logLevel = LogLevel.INFO
}

// 使用高性能配置
DNSManager.configure {
    dohServer = "https://dns.google/dns-query"
    enableSystemDNS = true
    enableHttpDNS = true
    enableLocalCache = true
    threadCount = 8
    logLevel = LogLevel.INFO
}

// 使用省电模式
DNSManager.configure {
    enableSystemDNS = true
    enableHttpDNS = false
    enableLocalCache = true
    threadCount = 1
    logLevel = LogLevel.WARN
}
```

### 3. DSL风格配置

```kotlin
DNSManager.quickInit(this) {
    dohServer = "https://dns.google/dns-query"
    enableSystemDNS = true
    enableHttpDNS = true
    enableLocalCache = true
    threadCount = 4
    logLevel = LogLevel.DEBUG
}
```

## 核心功能

### 1. 同步解析（阻塞调用）

```kotlin
// 在IO线程中调用
val ip = DNSManager.resolveHost("www.example.com")
println("IP: $ip")
```

### 2. 异步解析（回调方式）

```kotlin
DNSManager.resolveHostAsync("www.example.com") { ip, success ->
    if (success) {
        println("解析成功: $ip")
    } else {
        println("解析失败")
    }
}
```

### 3. 协程方式解析

```kotlin
lifecycleScope.launch {
    val ip = DNSManager.resolveHostSuspend("www.example.com")
    println("IP: $ip")
}
```

### 4. 获取所有IP地址

```kotlin
val ips = DNSManager.getAllIPs("www.example.com")
ips.forEach { ip ->
    println("IP: $ip")
}
```

## 高级功能

### 1. 批量解析

```kotlin
val hostnames = listOf("www.google.com", "www.baidu.com", "www.github.com")

// Flow方式
DNSManager.resolveHostsFlow(hostnames)
    .collect { (hostname, ip) ->
        println("$hostname -> $ip")
    }

// 批量异步
DNSManager.resolveHostsBatch(hostnames, lifecycleScope) { hostname, ip, success ->
    println("$hostname -> $ip (success: $success)")
}
```

### 2. 预加载常用域名

```kotlin
val commonDomains = listOf(
    "api.example.com",
    "cdn.example.com",
    "static.example.com"
)

DNSManager.preloadDomains(commonDomains, lifecycleScope)
```

### 3. 网络状态监听

```kotlin
class MyApplication : Application() {
    private val networkCallback = object : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            DNSManager.setNetworkState(NetworkState.WIFI)
        }
        
        override fun onLost(network: Network) {
            DNSManager.setNetworkState(NetworkState.UNKNOWN)
        }
    }
    
    override fun onCreate() {
        super.onCreate()
        
        val cm = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        cm.registerDefaultNetworkCallback(networkCallback)
    }
}
```

### 4. 性能监控

```kotlin
val monitor = object : PerformanceMonitor {
    override fun onResolutionComplete(hostname: String, duration: Long, success: Boolean) {
        println("解析 $hostname 耗时: ${duration}ms, 成功: $success")
    }
}

val ip = DNSManager.resolveWithMonitoring("www.example.com", monitor)
```

### 5. 缓存管理

```kotlin
// 清空缓存
DNSManager.clearCache()

// 刷新指定域名缓存
val domains = listOf("www.example.com", "api.example.com")
DNSManager.refreshCache(domains)
```

## 配置选项详解

### DoH服务器

```kotlin
// 阿里云DoH
DNSManager.setDohServer("https://dns.alidns.com/dns-query")

// Google DoH
DNSManager.setDohServer("https://dns.google/dns-query")

// Cloudflare DoH
DNSManager.setDohServer("https://cloudflare-dns.com/dns-query")
```

### DNS源控制

```kotlin
// 启用系统DNS
DNSManager.enableSystemDNS(true)

// 启用HTTP DNS (DoH)
DNSManager.enableHttpDNS(true)

// 启用本地缓存
DNSManager.enableLocalCache(true)
```

### 性能调优

```kotlin
// 设置工作线程数（建议2-8）
DNSManager.setThreadCount(4)

// 设置日志级别
DNSManager.setLogLevel(LogLevel.INFO)
```

### 网络状态

```kotlin
DNSManager.setNetworkState(NetworkState.WIFI)    // WiFi
DNSManager.setNetworkState(NetworkState.MOBILE)  // 移动网络
DNSManager.setNetworkState(NetworkState.NONE)    // 无网络
DNSManager.setNetworkState(NetworkState.UNKNOWN) // 未知
```

## 最佳实践

### 1. 应用启动时初始化

```kotlin
class MyApp : Application() {
    override fun onCreate() {
        super.onCreate()
        
        DNSManager.init(this)
        DNSManager.configure {
            dohServer = "https://dns.alidns.com/dns-query"
            enableSystemDNS = true
            enableHttpDNS = true
            enableLocalCache = true
            threadCount = 4
            logLevel = if (BuildConfig.DEBUG) LogLevel.DEBUG else LogLevel.INFO
        }
        
        // 预加载常用域名
        val commonDomains = listOf("api.myapp.com", "cdn.myapp.com")
        DNSManager.preloadDomains(commonDomains, GlobalScope)
    }
}
```

### 2. 网络请求前解析

```kotlin
suspend fun fetchData(url: String): String {
    val uri = Uri.parse(url)
    val hostname = uri.host ?: return ""
    
    // 先解析DNS
    val ip = DNSManager.resolveHostSuspend(hostname)
    if (ip.isEmpty()) {
        throw IOException("DNS解析失败: $hostname")
    }
    
    // 使用解析后的IP进行请求
    // ...
}
```

### 3. 错误处理

```kotlin
try {
    val ip = DNSManager.resolveHost("www.example.com")
    if (ip.isEmpty()) {
        // 解析失败，使用降级方案
        fallbackRequest()
    } else {
        // 使用解析的IP
        makeRequest(ip)
    }
} catch (e: Exception) {
    Log.e("DNS", "解析异常", e)
    fallbackRequest()
}
```

### 4. 生命周期管理

```kotlin
class MainActivity : AppCompatActivity() {
    override fun onPause() {
        super.onPause()
        // 应用进入后台时保存缓存
        DNSManager.setNetworkState(NetworkState.UNKNOWN)
    }
    
    override fun onResume() {
        super.onResume()
        // 应用恢复时更新网络状态
        updateNetworkState()
    }
}
```

## 性能优化建议

1. **合理设置线程数**: 根据设备性能调整，一般4-8个线程即可
2. **启用本地缓存**: 减少重复解析，提升响应速度
3. **预加载常用域名**: 应用启动时预解析常用域名
4. **监控网络状态**: 网络变化时及时更新DNS配置
5. **使用协程**: 避免阻塞主线程

## 故障排查

### 1. 解析失败

```kotlin
// 检查初始化
if (!DNSManager.initialized) {
    DNSManager.init(context)
}

// 检查网络权限
<uses-permission android:name="android.permission.INTERNET" />
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />

// 启用调试日志
DNSManager.setLogLevel(LogLevel.DEBUG)
```

### 2. 性能问题

```kotlin
// 减少线程数
DNSManager.setThreadCount(2)

// 禁用HTTP DNS
DNSManager.enableHttpDNS(false)

// 清空缓存
DNSManager.clearCache()
```

## 注意事项

1. 必须在Application中初始化
2. 同步解析不要在主线程调用
3. 网络权限必须声明
4. DoH服务器需要支持HTTPS
5. 缓存目录需要有写权限

