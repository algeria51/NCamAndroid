package com.ncam.app

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.util.Log
import java.io.File

class NCamService : Service(), NCamJNI.Callback {

    companion object {
        const val TAG            = "NCamService"
        const val CHANNEL_ID     = "ncam_channel"
        const val NOTIF_ID       = 1001
        const val ACTION_START   = "com.ncam.app.START"
        const val ACTION_STOP    = "com.ncam.app.STOP"
        const val DEFAULT_PORT   = 8181

        fun readHttpPort(configDir: File): Int {
            return try {
                File(configDir, "ncam.conf")
                    .readLines()
                    .firstOrNull { it.trimStart().startsWith("httpport") }
                    ?.substringAfter("=")
                    ?.trim()
                    ?.toIntOrNull()
                    ?: DEFAULT_PORT
            } catch (e: Exception) {
                DEFAULT_PORT
            }
        }
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            ACTION_START -> startNcam()
            ACTION_STOP  -> stopNcam()
        }
        return START_STICKY
    }

    private fun startNcam() {
        if (NCamJNI.isRunning()) {
            Log.i(TAG, "Already running")
            return
        }
        val configDir = File(filesDir, "ncam").also { it.mkdirs() }
        copyDefaultConfigsIfNeeded(configDir)
        val port = readHttpPort(configDir)
        startForeground(NOTIF_ID, buildNotification("NCam running on port $port"))
        val rc = NCamJNI.startNCam(configDir.absolutePath, this)
        if (rc != 0) {
            Log.e(TAG, "startNCam returned $rc")
            stopSelf()
        }
    }

    private fun stopNcam() {
        NCamJNI.stopNCam()
        stopForeground(true)
        stopSelf()
    }

    override fun onNcamStopped(exitCode: Int) {
        Log.i(TAG, "NCam stopped, exit=$exitCode")
        stopForeground(true)
        stopSelf()
    }

    override fun onDestroy() {
        if (NCamJNI.isRunning()) NCamJNI.stopNCam()
        super.onDestroy()
    }

    private fun copyDefaultConfigsIfNeeded(dir: File) {
        val conf = File(dir, "ncam.conf")
        if (!conf.exists()) {
            assets.open("ncam.conf").use { input ->
                conf.outputStream().use { input.copyTo(it) }
            }
        }
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val chan = NotificationChannel(
                CHANNEL_ID, "NCam Service",
                NotificationManager.IMPORTANCE_LOW
            )
            getSystemService(NotificationManager::class.java)
                .createNotificationChannel(chan)
        }
    }

    private fun buildNotification(text: String): Notification =
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("NCam")
                .setContentText(text)
                .setSmallIcon(android.R.drawable.ic_media_play)
                .build()
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
                .setContentTitle("NCam")
                .setContentText(text)
                .setSmallIcon(android.R.drawable.ic_media_play)
                .build()
        }
}
