cmake_minimum_required(VERSION 3.5)
set(WHOLE_PROJECT ON)
project(openSvk)

enable_testing()

option(BUILD_config "Собирать config" ON)
option(BUILD_svkcompose "Собирать svkcompose" ON)
option(BUILD_svkextract "Собирать svkextract" ON)
option(BUILD_svkmain "Собирать svkmain" ON)

option(BUILD_svkpop3get "Собирать svkpop3get" ON)
option(BUILD_svkpop3list "Собирать svkpop3list" ON)
option(BUILD_svksmtp "Собирать svksmtp" ON)
option(BUILD_svktelnet "Собирать svktelnet" ON)

option(BUILD_tests "Собирать Test" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "./../../bin")


if(BUILD_config)
add_subdirectory(config)
endif()
if(BUILD_svkcompose)
    add_subdirectory(svkcompose)
endif()
if(BUILD_svkextract)
    add_subdirectory(svkextract)
endif()
if(BUILD_svkmain)
    add_subdirectory(svkmain)
endif()

if(BUILD_svkpop3get)
    add_subdirectory(svkpop3get)
endif()
if(BUILD_svkpop3list)
    add_subdirectory(svkpop3list)
endif()
if(BUILD_svksmtp)
    add_subdirectory(svksmtp)
endif()
if(BUILD_svktelnet)
    add_subdirectory(svktelnet)
endif()

if(BUILD_tests)
    add_subdirectory(test)
endif()
