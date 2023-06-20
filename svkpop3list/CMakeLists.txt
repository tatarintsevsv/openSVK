cmake_minimum_required(VERSION 3.5)

project(svkpop3list LANGUAGES C)
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

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
