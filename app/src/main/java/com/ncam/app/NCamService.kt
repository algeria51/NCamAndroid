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
        const val TAG          = "NCamService"
        const val CHANNEL_ID   = "ncam_channel"
        const val NOTIF_ID     = 1001
        const val ACTION_START = "com.ncam.app.START"
        const val ACTION_STOP  = "com.ncam.app.STOP"
        const val DEFAULT_PORT = 8181

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
            // null = service restarted by START_STICKY after being killed
            null -> {
                Log.i(TAG, "Restarted by system (START_STICKY)")
                if (!NCamJNI.isRunning()) startNcam()
                else startForeground(NOTIF_ID, buildNotification("NCam running"))
            }
        }
        return START_STICKY
    }

    private fun startNcam() {
        if (NCamJNI.isRunning()) {
            Log.i(TAG, "Already running")
            startForeground(NOTIF_ID, buildNotification("NCam running"))
            return
        }

        val configDir = File(filesDir, "ncam").also { it.mkdirs() }
        // tmp dir inside confdir — passed as -t arg in jni_bridge
        File(configDir, "tmp").mkdirs()

        copyDefaultConfigsIfNeeded(configDir)
        val port = readHttpPort(configDir)

        startForeground(NOTIF_ID, buildNotification("NCam starting on port $port"))
        Log.i(TAG, "Starting NCam in ${configDir.absolutePath}")

        val rc = NCamJNI.startNCam(configDir.absolutePath, this)
        if (rc != 0) {
            Log.e(TAG, "startNCam() returned $rc — stopping service")
            stopForegroundCompat()
            stopSelf()
        }
    }

    private fun stopNcam() {
        Log.i(TAG, "Stopping NCam")
        NCamJNI.stopNCam()
        stopForegroundCompat()
        stopSelf()
    }

    override fun onNcamStopped(exitCode: Int) {
        Log.i(TAG, "NCam stopped naturally, exit=$exitCode")
        stopForegroundCompat()
        stopSelf()
    }

    override fun onDestroy() {
        if (NCamJNI.isRunning()) {
            Log.i(TAG, "onDestroy: requesting NCam stop")
            NCamJNI.stopNCam()
        }
        super.onDestroy()
    }

    private fun stopForegroundCompat() {
        if (Build.VERSION.SDK_INT >= 24)
            stopForeground(STOP_FOREGROUND_REMOVE)
        else
            @Suppress("DEPRECATION") stopForeground(true)
    }

    private fun copyDefaultConfigsIfNeeded(dir: File) {
        val conf = File(dir, "ncam.conf")
        if (!conf.exists()) {
            try {
                assets.open("ncam.conf").use { input ->
                    conf.outputStream().use { input.copyTo(it) }
                }
                Log.i(TAG, "Copied default ncam.conf to ${conf.absolutePath}")
            } catch (e: Exception) {
                Log.e(TAG, "Failed to copy default ncam.conf", e)
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
