cmake_minimum_required(VERSION 3.5)

project(svksmtp LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

find_package(curl)
link_libraries(curl)
link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")

add_executable(svksmtp main.c)
