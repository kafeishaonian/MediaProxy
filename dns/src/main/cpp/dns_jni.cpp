#include <jni.h>
#include <string>
#include "DNSEntrance.h"

static const char *TAG = "nds_jni";

static void init(JNIEnv *env, jobject instance) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance("default");
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->init();
        }
    } catch (const std::exception& e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI", std::string("Init failed: ") + e.what());
    }
}
static jstring resolveHost(JNIEnv *env, jobject instance, jstring hostname) {


    return env->NewStringUTF("");
}

static void resolveHostAsync(JNIEnv *env, jobject instance, jstring hostname, jobject callback) {

}

static jobjectArray getAllIPs(JNIEnv *env, jobject instance, jstring hostname) {


    return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
}

static void setDohServer(JNIEnv *env, jobject instance, jstring server) {

}

static void setNetworkState(JNIEnv *env, jobject instance, jint state) {

}

static void enableSystemDNS(JNIEnv *env, jobject instance, jboolean enable) {

}

static void enableHttpDNS(JNIEnv *env, jobject instance, jboolean enable) {

}

static void setCacheDir(JNIEnv *env, jobject instance, jstring dir) {

}

static void clearCache(JNIEnv *env, jobject instance) {

}


static void setLogLevel(JNIEnv *env, jobject instance, jint level) {

}


static const JNINativeMethod gMethod[] = {
        {"nativeInit",             "()V",                                              (void *) init},
        {"nativeResolveHost",      "(Ljava/lang/String;)Ljava/lang/String;",           (void *) resolveHost},
        {"nativeResolveHostAsync", "(Ljava/lang/String;Lcom/dns/cache/DNSCallback;)V", (void *) resolveHostAsync},
        {"nativeGetAllIPs",        "(Ljava/lang/String;)[Ljava/lang/String;",          (void *) getAllIPs},
        {"nativeSetDohServer",     "(Ljava/lang/String;)V",                            (void *) setDohServer},
        {"nativeSetNetworkState",  "(I)V",                                             (void *) setNetworkState},
        {"nativeEnableSystemDNS",  "(Z)V",                                             (void *) enableSystemDNS},
        {"nativeEnableHttpDNS",    "(Z)V",                                             (void *) enableHttpDNS},
        {"nativeSetCacheDir",      "(Ljava/lang/String;)V",                            (void *) setCacheDir},
        {"nativeClearCache",       "()V",                                              (void *) clearCache},
        {"nativeSetLogLevel",      "(I)V",                                             (void *) setLogLevel},
};


extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("com/dns/cache/DNSManager");
    if (!clazz) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(clazz, gMethod, sizeof(gMethod) / sizeof(gMethod[0]))) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
