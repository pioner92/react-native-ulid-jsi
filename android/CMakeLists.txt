cmake_minimum_required(VERSION 3.4.1)
project(react-native-ulid-jsi)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)

add_library(react-native-ulid-jsi            SHARED
            ../cpp/react-native-ulid.cpp
            ../cpp/ulid.h
            cpp-adapter.cpp
)

find_package(ReactAndroid REQUIRED CONFIG)

find_package(fbjni REQUIRED CONFIG)

target_include_directories(
        react-native-ulid-jsi PRIVATE
        "${NODE_MODULES_DIR}/react-native/React"
        "${NODE_MODULES_DIR}/react-native/React/Base"
        "${NODE_MODULES_DIR}/react-native/ReactCommon"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
)

# Specifies a path to native header files.
include_directories(
            ../cpp
)

if (ReactAndroid_VERSION_MINOR GREATER_EQUAL 76)
  target_link_libraries(
    react-native-ulid-jsi
    ReactAndroid::jsi
    log
    ReactAndroid::reactnative
    fbjni::fbjni
  )
else()
    target_link_libraries(
            react-native-ulid-jsi
            ReactAndroid::jsi
            log
            ReactAndroid::reactnativejni
            fbjni::fbjni
    )
endif()
