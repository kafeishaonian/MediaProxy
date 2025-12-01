package com.dns.cache

import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class DNSExampleActivity : AppCompatActivity() {

    private lateinit var etHostname: EditText
    private lateinit var tvResult: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_dns_example)

        initViews()
        setupListeners()
    }

    private fun initViews() {
        etHostname = findViewById(R.id.etHostname)
        tvResult = findViewById(R.id.tvResult)
    }

    private fun setupListeners() {
        findViewById<Button>(R.id.btnResolveSync).setOnClickListener {
            syncResolveExample()
        }

        findViewById<Button>(R.id.btnResolveAsync).setOnClickListener {
            asyncResolveExample()
        }

        findViewById<Button>(R.id.btnResolveCoroutine).setOnClickListener {
            coroutineResolveExample()
        }

        findViewById<Button>(R.id.btnGetAllIPs).setOnClickListener {
            getAllIPsExample()
        }

        findViewById<Button>(R.id.btnPreload).setOnClickListener {
            preloadDomainsExample()
        }

        findViewById<Button>(R.id.btnClearCache).setOnClickListener {
            clearCacheExample()
        }
    }

    private fun syncResolveExample() {
        val hostname = getHostname()
        if (hostname.isEmpty()) {
            showResult("请输入域名")
            return
        }

        showResult("同步解析中...")
        lifecycleScope.launch(Dispatchers.IO) {
            val ip = DNSManager.resolveHost(hostname)
            withContext(Dispatchers.Main) {
                if (ip.isNotEmpty()) {
                    showResult("同步解析成功:\n$hostname -> $ip")
                } else {
                    showResult("同步解析失败: $hostname")
                }
            }
        }
    }

    private fun asyncResolveExample() {
        val hostname = getHostname()
        if (hostname.isEmpty()) {
            showResult("请输入域名")
            return
        }

        showResult("异步解析中...")
        DNSManager.resolveHostAsync(hostname) { ip, success ->
            runOnUiThread {
                if (success) {
                    showResult("异步解析成功:\n$hostname -> $ip")
                } else {
                    showResult("异步解析失败: $hostname")
                }
            }
        }
    }

    private fun coroutineResolveExample() {
        val hostname = getHostname()
        if (hostname.isEmpty()) {
            showResult("请输入域名")
            return
        }

        showResult("协程解析中...")
        lifecycleScope.launch {
            try {
                val ip = DNSManager.resolveHostSuspend(hostname)
                if (ip.isNotEmpty()) {
                    showResult("协程解析成功:\n$hostname -> $ip")
                } else {
                    showResult("协程解析失败: $hostname")
                }
            } catch (e: Exception) {
                showResult("解析异常: ${e.message}")
            }
        }
    }

    private fun getAllIPsExample() {
        val hostname = getHostname()
        if (hostname.isEmpty()) {
            showResult("请输入域名")
            return
        }

        showResult("获取所有IP中...")
        lifecycleScope.launch(Dispatchers.IO) {
            val ips = DNSManager.getAllIPs(hostname)
            withContext(Dispatchers.Main) {
                if (ips.isNotEmpty()) {
                    showResult("所有IP:\n${ips.joinToString("\n")}")
                } else {
                    showResult("未找到IP: $hostname")
                }
            }
        }
    }

    private fun preloadDomainsExample() {
        val domains = listOf(
            "www.google.com",
            "www.github.com",
            "www.baidu.com"
        )

        showResult("预解析中...\n正在并发解析 ${domains.size} 个域名")

        lifecycleScope.launch(Dispatchers.IO) {
            val startTime = System.currentTimeMillis()
            val results = mutableMapOf<String, String>()

            val jobs = domains.map { domain ->
                async {
                    val ip = DNSManager.resolveHost(domain)
                    val success = ip.isNotEmpty()

                    withContext(Dispatchers.Main) {
                        results[domain] = ip
                        updatePreloadProgress(domain, ip, success, results.size, domains.size)
                    }

                    Pair(domain, ip)
                }
            }

            jobs.forEach { it.await() }

            val totalTime = System.currentTimeMillis() - startTime

            withContext(Dispatchers.Main) {
                showFinalPreloadResult(results, totalTime)
            }
        }
    }

    private fun updatePreloadProgress(domain: String, ip: String, success: Boolean, current: Int, total: Int) {
        val status = if (success) "✅" else "❌"
        val ipText = if (success) ip else "解析失败"
        tvResult.append("\n$status $domain\n   -> $ipText\n")
    }

    private fun showFinalPreloadResult(results: Map<String, String>, totalTime: Long) {
        val successCount = results.values.count { it.isNotEmpty() }
        val failCount = results.size - successCount

        val summary = StringBuilder()
        summary.append("\n")
        summary.append("━━━━━━━━━━━━━━━━━━━━\n")
        summary.append("预解析完成！\n")
        summary.append("总耗时: ${totalTime}ms\n")
        summary.append("成功: $successCount  失败: $failCount\n")
        summary.append("━━━━━━━━━━━━━━━━━━━━")

        tvResult.append(summary.toString())
    }

    private fun clearCacheExample() {
        DNSManager.clearCache()
        showResult("缓存已清空")
    }

    private fun getHostname(): String {
        return etHostname.text.toString().trim()
    }

    private fun showResult(text: String) {
        tvResult.text = text
    }

    /**
     * 示例: 批量解析域名
     */
    private fun batchResolveExample() {
        val domains = listOf(
            "www.google.com",
            "www.github.com",
            "www.baidu.com"
        )

        showResult("批量解析中...\n正在解析 ${domains.size} 个域名\n")

        lifecycleScope.launch(Dispatchers.IO) {
            val startTime = System.currentTimeMillis()
            val results = mutableListOf<Pair<String, String>>()

            domains.forEach { domain ->
                val ip = DNSManager.resolveHost(domain)
                results.add(domain to ip)

                withContext(Dispatchers.Main) {
                    val status = if (ip.isNotEmpty()) "✅" else "❌"
                    tvResult.append("$status $domain -> $ip\n")
                }
            }

            val totalTime = System.currentTimeMillis() - startTime
            withContext(Dispatchers.Main) {
                val successCount = results.count { it.second.isNotEmpty() }
                tvResult.append("\n批量解析完成！\n")
                tvResult.append("总耗时: ${totalTime}ms\n")
                tvResult.append("成功: $successCount/${domains.size}\n")
            }
        }
    }

    /**
     * 示例: 智能DNS切换（根据网络状态）
     */
    private fun smartDNSSwitchExample(networkState: NetworkState) {
        when (networkState) {
            NetworkState.WIFI -> {
                // WiFi环境使用HTTP DNS
                DNSManager.configure {
                    enableSystemDNS = true
                    enableHttpDNS = true
                    dohServer = "https://dns.alidns.com/dns-query"
                }
                showResult("已切换到WiFi模式：启用HTTP DNS")
            }
            NetworkState.MOBILE -> {
                // 移动网络使用系统DNS（节省流量）
                DNSManager.configure {
                    enableSystemDNS = true
                    enableHttpDNS = false
                }
                showResult("已切换到移动网络模式：仅使用系统DNS")
            }
            NetworkState.NONE -> {
                // 无网络时保存缓存
                DNSManager.configure {
                    enableSystemDNS = false
                    enableHttpDNS = false
                }
                showResult("无网络连接")
            }
            else -> {
                // 其他情况只使用系统DNS
                DNSManager.configure {
                    enableSystemDNS = true
                    enableHttpDNS = false
                }
                showResult("已切换到默认模式：仅使用系统DNS")
            }
        }
        DNSManager.setNetworkState(networkState)
    }

    /**
     * 示例: 解析URL中的域名
     */
    private fun resolveURLExample() {
        val url = "https://www.google.com/search?q=kotlin"
        showResult("解析URL中...")

        lifecycleScope.launch(Dispatchers.IO) {
            // 提取域名
            val hostname = url.substringAfter("://").substringBefore("/")
            val ip = DNSManager.resolveHost(hostname)

            withContext(Dispatchers.Main) {
                if (ip.isNotEmpty()) {
                    showResult("URL解析成功:\n$url\n域名: $hostname\nIP: $ip")
                } else {
                    showResult("URL解析失败: $url")
                }
            }
        }
    }

    /**
     * 示例: 刷新特定域名的缓存
     */
    private fun refreshCacheExample() {
        val domains = listOf("www.google.com", "www.github.com")
        showResult("刷新缓存中...")

        lifecycleScope.launch(Dispatchers.IO) {
            domains.forEach { domain ->
                // 先清除缓存
                DNSManager.clearCache()
                // 重新解析
                val ip = DNSManager.resolveHost(domain)

                withContext(Dispatchers.Main) {
                    tvResult.append("$domain -> $ip (已刷新)\n")
                }
            }

            withContext(Dispatchers.Main) {
                tvResult.append("\n缓存刷新完成！")
            }
        }
    }
}

