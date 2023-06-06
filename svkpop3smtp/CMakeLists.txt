cmake_minimum_required(VERSION 3.5)

project(svksmtp LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")

find_package(curl)
link_libraries(curl)
link_libraries("/home/arise/qt/CBR_SVK/bin/libconfig.so")

add_executable(svksmtp main.c)
