cmake_minimum_required(VERSION 3.5)

project(svkpop3get LANGUAGES C)
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

find_library(CURL_LIB curl)
link_libraries(curl)

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "{CMAKE_SOURCE_DIR}/../../bin")
endif()
link_libraries("${CMAKE_SOURCE_DIR}/../bin/libconfig.so")

add_executable(svkpop3get main.c)
