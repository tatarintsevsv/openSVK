cmake_minimum_required(VERSION 3.5)

project(svkpop3list LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")

find_package(LibXml2 REQUIRED)
link_libraries(${LIBXML2_LIBRARIES})
find_package(curl)
link_libraries(curl)
#find_library(config SHARED)
#link_libraries(config)
link_libraries("/home/arise/qt/CBR_SVK/bin/libconfig.so")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config ${LIBXML2_INCLUDE_DIR})


add_executable(svkpop3list main.c ../config/config.c ../config/config.h)
