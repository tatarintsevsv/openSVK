cmake_minimum_required(VERSION 3.5)

project(svkcompose LANGUAGES CXX)

SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")

find_library(LIBUUID_LIBRARY libuuid.a)
link_libraries(${LIBUUID_LIBRARY})

add_executable(svkcompose main.cpp svkcompose.h svkcompose.cpp base64.cpp base64.h)
