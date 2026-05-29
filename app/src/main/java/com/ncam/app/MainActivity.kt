package com.ncam.app

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.ScrollView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.io.File
import java.net.NetworkInterface

class MainActivity : AppCompatActivity() {

    private lateinit var tabControl: Button
    private lateinit var tabConsole: Button
    private lateinit var pageControl: View
    private lateinit var pageConsole: View

    private lateinit var tvStatus: TextView
    private lateinit var tvUrl: TextView
    private lateinit var btnStart: Button
    private lateinit var btnStop: Button
    private lateinit var btnOpenBrowser: Button

    private lateinit var tvLog: TextView
    private lateinit var scrollLog: ScrollView
    private lateinit var btnClearLog: Button

    private val handler = Handler(Looper.getMainLooper())
    private val logBuffer = StringBuilder()
    private var logcatProcess: Process? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tabControl     = findViewById(R.id.tabControl)
        tabConsole     = findViewById(R.id.tabConsole)
        pageControl    = findViewById(R.id.pageControl)
        pageConsole    = findViewById(R.id.pageConsole)
        tvStatus       = findViewById(R.id.tvStatus)
        tvUrl          = findViewById(R.id.tvUrl)
        btnStart       = findViewById(R.id.btnStart)
        btnStop        = findViewById(R.id.btnStop)
        btnOpenBrowser = findViewById(R.id.btnOpenBrowser)
        tvLog          = findViewById(R.id.tvLog)
        scrollLog      = findViewById(R.id.scrollLog)
        btnClearLog    = findViewById(R.id.btnClearLog)

        tvStatus.text = "NCam v${NCamJNI.getVersion()}"

        tabControl.setOnClickListener { showTab(0) }
        tabConsole.setOnClickListener { showTab(1) }

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
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(getWebifUrl())))
        }

        btnClearLog.setOnClickListener {
            logBuffer.clear()
            tvLog.text = ""
        }

        showTab(0)
        updateUI(NCamJNI.isRunning())
        startLogcatCapture()
    }

    override fun onResume() {
        super.onResume()
        updateUI(NCamJNI.isRunning())
    }

    override fun onDestroy() {
        super.onDestroy()
        logcatProcess?.destroy()
        handler.removeCallbacksAndMessages(null)
    }

    private fun showTab(index: Int) {
        pageControl.visibility = if (index == 0) View.VISIBLE else View.GONE
        pageConsole.visibility = if (index == 1) View.VISIBLE else View.GONE
        tabControl.setTextColor(if (index == 0) 0xFFFFFFFF.toInt() else 0xFFBBDEFB.toInt())
        tabConsole.setTextColor(if (index == 1) 0xFFFFFFFF.toInt() else 0xFFBBDEFB.toInt())
    }

    private fun startLogcatCapture() {
        Thread {
            try {
                logcatProcess = Runtime.getRuntime().exec(
                    arrayOf("logcat", "-v", "time", "-s", "NCam:*", "NCam-JNI:*", "NCamService:*")
                )
                logcatProcess!!.inputStream.bufferedReader().forEachLine { line ->
                    appendLog(line)
                }
            } catch (e: Exception) {
                Log.e("MainActivity", "logcat error", e)
            }
        }.also { it.isDaemon = true }.start()
    }

    private fun appendLog(line: String) {
        handler.post {
            if (logBuffer.length > 80_000) {
                logBuffer.delete(0, 20_000)
            }
            logBuffer.append(line).append('\n')
            tvLog.text = logBuffer
            scrollLog.post { scrollLog.fullScroll(View.FOCUS_DOWN) }
        }
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
            Log.e("MainActivity", "getDeviceIp", e)
        }
        return "127.0.0.1"
    }

    private fun getWebifUrl(): String = "http://${getDeviceIp()}:${getHttpPort()}"

    private fun updateUI(running: Boolean) {
        btnStart.isEnabled       = !running
        btnStop.isEnabled        = running
        btnOpenBrowser.isEnabled = running
        tvStatus.text = if (running) "Status: Running" else "Status: Stopped"
        tvUrl.text    = if (running) "WebIF: ${getWebifUrl()}" else ""
    }
}
