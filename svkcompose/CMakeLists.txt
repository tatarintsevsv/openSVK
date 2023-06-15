cmake_minimum_required(VERSION 3.5)

project(svkcompose LANGUAGES CXX)

SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")


link_libraries("/home/arise/qt/CBR_SVK/bin/libconfig.so")
find_library(LIBUUID_LIBRARY libuuid.a)
include_directories(${LIBUUID_INCLUDE_DIR})
link_libraries(${LIBXML2_LIBRARIES} ${LIBUUID_LIBRARY})


add_executable(svkcompose main.cpp svkcompose.h svkcompose.cpp base64.cpp base64.h)
