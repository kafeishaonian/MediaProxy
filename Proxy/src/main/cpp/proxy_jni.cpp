#include <jni.h>

static const char *JNIPreloadClassName = "com/media/proxy/ProxyPreload";
static const char *TAG = "proxy_jni";


JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        return;
    }

}
