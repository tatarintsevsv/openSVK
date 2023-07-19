cmake_minimum_required(VERSION 3.5)

project(svkpop3list LANGUAGES C)
SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

find_library(CURL_LIB curl)
link_libraries(curl)
if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")
endif()

add_executable(svkpop3list main.c)
