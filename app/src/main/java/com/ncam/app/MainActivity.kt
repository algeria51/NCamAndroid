package com.ncam.app

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import java.io.File
import java.net.NetworkInterface

class MainActivity : AppCompatActivity() {

    private lateinit var tvStatus: TextView
    private lateinit var tvUrl: TextView
    private lateinit var btnStart: Button
    private lateinit var btnStop: Button
    private lateinit var btnOpenBrowser: Button
    private lateinit var btnCopyUrl: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus       = findViewById(R.id.tvStatus)
        tvUrl          = findViewById(R.id.tvUrl)
        btnStart       = findViewById(R.id.btnStart)
        btnStop        = findViewById(R.id.btnStop)
        btnOpenBrowser = findViewById(R.id.btnOpenBrowser)
        btnCopyUrl     = findViewById(R.id.btnCopyUrl)

        tvStatus.text = "NCam v${NCamJNI.getVersion()}"

        btnStart.setOnClickListener {
            startService(Intent(this, NCamService::class.java).apply {
                action = NCamService.ACTION_START
            })
            updateUI(true)
        }

        btnStop.setOnClickListener {
            startService(Intent(this, NCamService::class.java).apply {
                action = NCamService.ACTION_STOP
            })
            updateUI(false)
        }

        btnOpenBrowser.setOnClickListener {
            val url = getWebifUrl()
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
        }

        btnCopyUrl.setOnClickListener {
            val url = getWebifUrl()
            val cm = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            cm.setPrimaryClip(ClipData.newPlainText("NCam URL", url))
            Toast.makeText(this, "Copied: $url", Toast.LENGTH_SHORT).show()
        }

        updateUI(NCamJNI.isRunning())
    }

    override fun onResume() {
        super.onResume()
        updateUI(NCamJNI.isRunning())
    }

    private fun getConfigDir(): File = File(filesDir, "ncam")

    private fun getHttpPort(): Int = NCamService.readHttpPort(getConfigDir())

    private fun getDeviceIp(): String {
        try {
            val interfaces = NetworkInterface.getNetworkInterfaces()
            while (interfaces.hasMoreElements()) {
                val iface = interfaces.nextElement()
                if (iface.isLoopback || !iface.isUp) continue
                val addrs = iface.inetAddresses
                while (addrs.hasMoreElements()) {
                    val addr = addrs.nextElement()
                    if (addr.isLoopbackAddress) continue
                    val host = addr.hostAddress ?: continue
                    if (!host.contains(':')) return host
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        return "127.0.0.1"
    }

    private fun getWebifUrl(): String = "http://${getDeviceIp()}:${getHttpPort()}"

    private fun updateUI(running: Boolean) {
        btnStart.isEnabled       = !running
        btnStop.isEnabled        = running
        btnOpenBrowser.isEnabled = running
        btnCopyUrl.isEnabled     = running

        if (running) {
            val url = getWebifUrl()
            tvStatus.text = "Status: Running"
            tvUrl.text    = "WebIF: $url"
        } else {
            tvStatus.text = "Status: Stopped"
            tvUrl.text    = ""
        }
    }
}
