cmake_minimum_required(VERSION 3.22.1)

project("media_proxy")
# 源代码
set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE SOURCE_FILES
        "${SOURCE_ROOT}/*.cpp"
)

# boost
set(BOOST_ROOT_PATH ${CMAKE_SOURCE_DIR}/../libs/boost)
get_filename_component(BOOST_ROOT ${BOOST_ROOT_PATH} ABSOLUTE)

# curl
set(PREBUILT_ROOT_PATH ${CMAKE_SOURCE_DIR}/../libs/prebuilt)
get_filename_component(PREBUILT_ROOT ${PREBUILT_ROOT_PATH} ABSOLUTE)

#ffmpeg
set(FFMPEG_ROOT_PATH ${CMAKE_SOURCE_DIR}/../libs/ffmpeg)
get_filename_component(FFMPEG_ROOT ${FFMPEG_ROOT_PATH} ABSOLUTE)

if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    #boost
    set(BOOST_LIB_DIR ${BOOST_ROOT}/${ANDROID_ABI}/lib)
    set(BOOST_INCLUDE_DIR ${BOOST_ROOT}/${ANDROID_ABI}/include)

    #curl
    set(PREBUILT_LIB_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/lib)
    set(PREBUILT_INCLUDE_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/include)

    #ffmpeg
    set(FFMPEG_LIB_DIR ${FFMPEG_ROOT}/${ANDROID_ABI}/lib)
    set(FFMPEG_INCLUDE_DIR ${FFMPEG_ROOT}/${ANDROID_ABI}/include)
endif ()


add_library(${CMAKE_PROJECT_NAME} SHARED
        ${SOURCE_FILES}
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${SOURCE_ROOT}
        ${SOURCE_ROOT}/base
        ${SOURCE_ROOT}/cache/disk
        ${SOURCE_ROOT}/common
        ${SOURCE_ROOT}/utils
        ${SOURCE_ROOT}/interface
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${BOOST_INCLUDE_DIR})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PREBUILT_INCLUDE_DIR})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIR})

add_library(boost_atomic STATIC IMPORTED)
set_target_properties(boost_atomic PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_atomic.a)

add_library(boost_chrono STATIC IMPORTED)
set_target_properties(boost_chrono PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_chrono.a)

add_library(boost_iostreams STATIC IMPORTED)
set_target_properties(boost_iostreams PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_iostreams.a)

add_library(boost_system STATIC IMPORTED)
set_target_properties(boost_system PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_system.a)

add_library(boost_thread STATIC IMPORTED)
set_target_properties(boost_thread PROPERTIES IMPORTED_LOCATION ${BOOST_LIB_DIR}/libboost_thread.a)

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libcrypto.a)

add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libcurl.a)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libssl.a)

add_library(zip STATIC IMPORTED)
set_target_properties(zip PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libz.a)

add_library(avcodec STATIC IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavcodec.a)

add_library(avdevice STATIC IMPORTED)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavdevice.a)

add_library(avfilter STATIC IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavfilter.a)

add_library(avformat STATIC IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavformat.a)

add_library(avutil STATIC IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavutil.a)

add_library(postproce STATIC IMPORTED)
set_target_properties(postproce PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libpostproce.a)

add_library(swresample STATIC IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswresample.a)

add_library(swscale STATIC IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswscale.a)


find_library(ZLIB_LIB z)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        my_native_lib
        android
        log
        boost_atomic
        boost_chrono
        boost_iostreams
        boost_system
        boost_thread
        crypto
        curl
        ssl
        zip
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproce
        swresample
        swscale
        ${ZLIB_LIB}
)