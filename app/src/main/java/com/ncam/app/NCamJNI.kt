package com.ncam.app

object NCamJNI {

    init {
        System.loadLibrary("ncam")
    }

    interface Callback {
        fun onNcamStopped(exitCode: Int)
    }

    external fun startNCam(configDir: String, callback: Callback?): Int
    external fun stopNCam()
    external fun isRunning(): Boolean
    external fun getVersion(): String
}
