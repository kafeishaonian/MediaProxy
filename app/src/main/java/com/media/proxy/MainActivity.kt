package com.media.proxy

import android.content.Intent
import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.dns.cache.DNSExampleActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.main_activity)

        findViewById<TextView>(R.id.dns_test).setOnClickListener {
            startActivity(Intent(this, DNSExampleActivity::class.java))
        }
    }

}
