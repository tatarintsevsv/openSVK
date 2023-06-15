cmake_minimum_required(VERSION 3.5)
project(config LANGUAGES C)
find_package(LibXml2 REQUIRED)
link_libraries(${LIBXML2_LIBRARIES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config ${LIBXML2_INCLUDE_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")

#
add_library(config SHARED main.c config.c config.h)
#
#add_executable(config main.c config.c config.h)



