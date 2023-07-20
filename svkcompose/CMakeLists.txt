cmake_minimum_required(VERSION 3.5)

project(svkcompose LANGUAGES CXX)
SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")
endif()


find_library(LIBUUID_LIBRARY NAMES uuid)
link_libraries(${LIBUUID_LIBRARY})


add_executable(svkcompose main.cpp svkcompose.h svkcompose.cpp base64.cpp base64.h ../common/linuxdir.cpp ../common/linuxdir.h)
