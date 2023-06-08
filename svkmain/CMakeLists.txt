cmake_minimum_required(VERSION 3.5)

project(svkmain LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")

link_libraries("/home/arise/qt/CBR_SVK/bin/libconfig.so")

add_executable(svkmain main.cpp)
