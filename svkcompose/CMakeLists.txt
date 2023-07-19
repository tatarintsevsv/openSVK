cmake_minimum_required(VERSION 3.5)

project(svkcompose LANGUAGES CXX)

SET(GCC_MY_FLAGS "-Wl,--disable-new-dtags,-rpath,'$ORIGIN'")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED WHOLE_PROJECT)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    link_libraries("${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libconfig.so")
endif()


find_library(LIBUUID_LIBRARY NAMES uuid)
#find_library(LIBCXXFS_LIBRARY NAMES stdc++fs)
#link_libraries(${LIBUUID_LIBRARY} ${LIBCXXFS})
link_libraries(${LIBUUID_LIBRARY} stdc++fs)


add_executable(svkcompose main.cpp svkcompose.h svkcompose.cpp base64.cpp base64.h)
