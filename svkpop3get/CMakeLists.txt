cmake_minimum_required(VERSION 3.5)

project(svkpop3get LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/arise/qt/CBR_SVK/bin")

find_package(curl)
link_libraries(curl)
add_executable(svkpop3get main.c)
