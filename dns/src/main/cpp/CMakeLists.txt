cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(dns_cache)

# 源代码
set(SOURCE_ROOT ${CMAKE_SOURCE_DIR})
file(GLOB_RECURSE SOURCE_FILES
        "${SOURCE_ROOT}/*.cpp"
)

set(PREBUILT_ROOT_PATH ${CMAKE_SOURCE_DIR}/../libs/prebuilt)
get_filename_component(PREBUILT_ROOT ${PREBUILT_ROOT_PATH} ABSOLUTE)

#rapidjson
set(RAPIDJSON_ROOT_PATH ${CMAKE_SOURCE_DIR}/../libs/rapidjson)
get_filename_component(RAPIDJSON_ROOT ${RAPIDJSON_ROOT_PATH} ABSOLUTE)

if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(PREBUILT_LIB_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/lib)
    set(PREBUILT_INCLUDE_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/include)

    #rapidjson
    set(RAPIDJSON_INCLUDE_DIR ${RAPIDJSON_ROOT})
endif ()


add_library(${CMAKE_PROJECT_NAME} SHARED
        ${SOURCE_FILES}
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${SOURCE_ROOT}
        ${SOURCE_ROOT}/common
        ${SOURCE_ROOT}/entrance
        ${SOURCE_ROOT}/http
        ${SOURCE_ROOT}/manager
        ${SOURCE_ROOT}/monitor
        ${SOURCE_ROOT}/pool
        ${SOURCE_ROOT}/server
        ${SOURCE_ROOT}/task

)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PREBUILT_INCLUDE_DIR})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIR})

add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libcrypto.a)

add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libcurl.a)

add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libssl.a)

add_library(zip STATIC IMPORTED)
set_target_properties(zip PROPERTIES IMPORTED_LOCATION ${PREBUILT_LIB_DIR}/libz.a)

find_library(ZLIB_LIB z)

target_link_options(dns_cache PRIVATE "-Wl,-z,max-page-size=16384")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        my_native_lib
        android
        log
        crypto
        curl
        ssl
        zip
        ${ZLIB_LIB}
)