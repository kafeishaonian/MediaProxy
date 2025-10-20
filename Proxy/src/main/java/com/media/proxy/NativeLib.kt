package com.media.proxy

class NativeLib {

    /**
     * A native method that is implemented by the 'proxy' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'proxy' library on application startup.
        init {
            System.loadLibrary("proxy")
        }
    }
}