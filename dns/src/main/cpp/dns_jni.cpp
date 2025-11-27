#include <jni.h>
#include <string>
#include "DNSEntrance.h"

static const char *TAG = "nds_jni";

static std::string jstring_to_string(JNIEnv *env, jstring jstr) {
    if (!jstr) {
        return "";
    }

    const char *chars = env->GetStringUTFChars(jstr, nullptr);
    std::string str(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return str;
}

static jstring string_to_jstring(JNIEnv *env, const std::string &str) {
    return env->NewStringUTF(str.c_str());
}


static void init(JNIEnv *env, jobject instance) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->init();
        }
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI", std::string("Init failed: ") + e.what());
    }
}

static jstring resolveHost(JNIEnv *env, jobject instance, jstring jhostname) {
    std::string hostname = jstring_to_string(env, jhostname);

    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        std::string ip = dns->resolve_host(hostname);
        return string_to_jstring(env, ip);
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI", std::string("Resolve failed: ") + e.what());
        return string_to_jstring(env, "");
    }
}

static void resolveHostAsync(JNIEnv *env, jobject instance, jstring jhostname, jobject jcallback) {
    std::string hostname = jstring_to_string(env, jhostname);

    jobject global_callback = env->NewGlobalRef(jcallback);
    JavaVM *jvm;
    env->GetJavaVM(&jvm);

    try {
        auto dns = dns::DNSEntranceImpl::get_instance();

        dns->resolve_host_async(hostname,
                                [jvm, global_callback, hostname](auto host, bool success,
                                                                 auto old_host) {
                                    JNIEnv *cb_env;
                                    bool attached = false;

                                    if (jvm->GetEnv((void **) &cb_env, JNI_VERSION_1_6) ==
                                        JNI_EDETACHED) {
                                        if (jvm->AttachCurrentThread(&cb_env, nullptr) == 0) {
                                            attached = true;
                                        }
                                    }

                                    if (cb_env) {
                                        jclass callback_class = cb_env->GetObjectClass(
                                                global_callback);
                                        jmethodID on_result = cb_env->GetMethodID(callback_class,
                                                                                  "onResult",
                                                                                  "(Ljava/lang/String;Z)V");

                                        if (on_result) {
                                            jstring jip = cb_env->NewStringUTF(
                                                    host ? host->get_bast_ip_string().c_str() : ""
                                            );
                                            cb_env->CallVoidMethod(global_callback, on_result, jip,
                                                                   success);
                                            cb_env->DeleteLocalRef(jip);
                                        }

                                        cb_env->DeleteLocalRef(callback_class);
                                        cb_env->DeleteGlobalRef(global_callback);

                                        if (attached) {
                                            jvm->DetachCurrentThread();
                                        }
                                    }
                                });
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("Async resolve failed: ") + e.what());
        env->DeleteGlobalRef(global_callback);
    }
}

static jobjectArray getAllIPs(JNIEnv *env, jobject instance, jstring jhostname) {
    std::string hostname = jstring_to_string(env, jhostname);

    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto ips = dns->get_all_ips(hostname);

        jclass string_class = env->FindClass("java/lang/String");
        jobjectArray result = env->NewObjectArray(ips.size(), string_class, nullptr);

        for (size_t i = 0; i < ips.size(); ++i) {
            jstring jip = string_to_jstring(env, ips[i]);
            env->SetObjectArrayElement(result, i, jip);
            env->DeleteLocalRef(jip);
        }

        env->DeleteLocalRef(string_class);
        return result;
    } catch (const std::exception& e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                    std::string("获取全部IP列表失败: ") + e.what());
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    }
}

static void setDohServer(JNIEnv *env, jobject instance, jstring jserver) {
    std::string server = jstring_to_string(env, jserver);
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->set_doh_server(server);
        }
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("设置DoH服务器失败: ") + e.what());
    }
}

static void setNetworkState(JNIEnv *env, jobject instance, jint jstate) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        dns::DNSAppNetState state = static_cast<dns::DNSAppNetState>(jstate);
        dns->set_network_state(state);
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("设置网络状态失败: ") + e.what());
    }
}

static void enableSystemDNS(JNIEnv *env, jobject instance, jboolean enable) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->enable_system_dns(enable);
        }
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("启用系统DNS失败: ") + e.what());
    }
}

static void enableHttpDNS(JNIEnv *env, jobject instance, jboolean enable) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->enable_http_dns(enable);
        }
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("启用HTTP DNS失败: ") + e.what());
    }
}

static void setCacheDir(JNIEnv *env, jobject instance, jstring jdir) {
    std::string dir = jstring_to_string(env, jdir);

    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        auto impl = std::dynamic_pointer_cast<dns::DNSEntranceImpl>(dns);
        if (impl) {
            impl->set_cache_dir(dir);
        }
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("设置缓存目录失败: ") + e.what());
    }
}

static void clearCache(JNIEnv *env, jobject instance) {
    try {
        auto dns = dns::DNSEntranceImpl::get_instance();
        dns->clear();
    } catch (const std::exception &e) {
        dns::Logger::log(dns::LogLevel::ERROR, "JNI",
                         std::string("清空缓存失败: ") + e.what());
    }
}


static void setLogLevel(JNIEnv *env, jobject instance, jint jlevel) {
    dns::LogLevel level = static_cast<dns::LogLevel>(jlevel);
    dns::Logger::setLevel(level);
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
