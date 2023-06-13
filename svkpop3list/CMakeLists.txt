cmake_minimum_required(VERSION 3.5)

project(svkpop3list LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

find_package(curl)
link_libraries(curl)
#find_library(config SHARED)
#link_libraries(config)
link_libraries("/home/arise/qt/CBR_SVK/bin/libconfig.so")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config ${LIBXML2_INCLUDE_DIR})


add_executable(svkpop3list main.c)
