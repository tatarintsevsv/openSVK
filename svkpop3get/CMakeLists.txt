cmake_minimum_required(VERSION 3.5)

project(svkpop3get LANGUAGES C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")
SET(GCC_MY_FLAGS "-Wl,-rpath,.")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_MY_FLAGS}")

find_package(curl)
link_libraries(curl)
#find_package(LibXml2 REQUIRED)
#link_libraries(${LIBXML2_LIBRARIES})
link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/../bin/libconfig.so")

add_executable(svkpop3get main.c)
