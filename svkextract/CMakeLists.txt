cmake_minimum_required(VERSION 3.5)

project(svkextract LANGUAGES CXX)

SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")
endif()

add_executable(svkextract main.cpp svkextract.h svkextract.cpp ../svkcompose/base64.cpp ../svkcompose/base64.h ../config/configcxx.cpp ../config/configcxx.h)
