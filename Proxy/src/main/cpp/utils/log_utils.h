//
// Created by 64860 on 2025/5/7.
//

#ifndef ANDROIDX_JETPACK_LOG_UTILS_H
#define ANDROIDX_JETPACK_LOG_UTILS_H

#include <android/log.h>

#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)

#endif //ANDROIDX_JETPACK_LOG_UTILS_H
