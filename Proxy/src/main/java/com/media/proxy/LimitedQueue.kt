package com.media.proxy

import java.util.LinkedList

class LimitedQueue<E>(val limit: Int) : LinkedList<E>() {

    override fun add(e: E): Boolean {
        if (!contains(e)) {
            super.add(e)
            while (size > limit) {
                super.remove()
            }
            return true
        }

        return false
    }
}