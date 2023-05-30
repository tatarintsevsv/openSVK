cmake_minimum_required(VERSION 3.5)

project(svkpop3list LANGUAGES C)
find_package(curl)
link_libraries(curl)

add_executable(svkpop3list main.c)
