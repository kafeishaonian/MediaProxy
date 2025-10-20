cmake_minimum_required(VERSION 3.22.1)

project("proxy")

set(BOOST_ROOT_PATH ${CMAKE_SOURCE_DIR}/../../../libs/boost)
get_filename_component(BOOST_ROOT ${BOOST_ROOT_PATH} ABSOLUTE)

set(PREBUILT_ROOT_PATH ${CMAKE_SOURCE_DIR}/../../../libs/prebuilt)
get_filename_component(PREBUILT_ROOT ${PREBUILT_ROOT_PATH} ABSOLUTE)

if(${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(BOOST_LIB_DIR ${BOOST_ROOT}/${ANDROID_ABI}/lib)
    set(BOOST_INCLUDE_DIR ${BOOST_ROOT}/${ANDROID_ABI}/include)
    set(PREBUILT_LIB_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/lib)
    set(PREBUILT_INCLUDE_DIR ${PREBUILT_ROOT}/${ANDROID_ABI}/include)
endif()


add_library(${CMAKE_PROJECT_NAME} SHARED proxy.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${BOOST_INCLUDE_DIR})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PREBUILT_INCLUDE_DIR})

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
        ${ZLIB_LIB}
)