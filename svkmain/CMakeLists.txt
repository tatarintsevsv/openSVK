cmake_minimum_required(VERSION 3.5)

project(svkmain LANGUAGES CXX)

SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "{CMAKE_SOURCE_DIR}/../../bin")
endif()
link_libraries("${CMAKE_SOURCE_DIR}/../bin/libconfig.so")

add_executable(svkmain main.cpp svkmain.h svkmain.cpp)
