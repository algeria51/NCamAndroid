package com.ncam.app

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var tvStatus : TextView
    private lateinit var btnStart : Button
    private lateinit var btnStop  : Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus = findViewById(R.id.tvStatus)
        btnStart = findViewById(R.id.btnStart)
        btnStop  = findViewById(R.id.btnStop)

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

        updateUI(NCamJNI.isRunning())
    }

    override fun onResume() {
        super.onResume()
        updateUI(NCamJNI.isRunning())
    }

    private fun updateUI(running: Boolean) {
        btnStart.isEnabled = !running
        btnStop.isEnabled  = running
        tvStatus.text      = if (running) "Status: Running" else "Status: Stopped"
    }
}
