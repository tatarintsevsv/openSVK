project(runTests)

find_package(GTest REQUIRED)
find_package(LibXml2 REQUIRED)

link_libraries(${LIBXML2_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/config ${LIBXML2_INCLUDE_DIR} ${GTEST_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_executable(test_config test_config.cpp ../config/config.h ../config/config.c)
target_link_libraries(test_config ${GTEST_LIBRARIES} pthread)
