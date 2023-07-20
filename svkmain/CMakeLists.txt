cmake_minimum_required(VERSION 3.5)

project(svkmain LANGUAGES CXX)

SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")
endif()

add_executable(svkmain main.cpp svkmain.h svkmain.cpp ../common/linuxdir.h ../common/linuxdir.cpp)
target_link_libraries(svkmain)

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(svkmain PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(svkmain "${CMAKE_THREAD_LIBS_INIT}")
endif()
