cmake_minimum_required(VERSION 3.16)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(Testing)

add_library(IdBimap INTERFACE)

target_include_directories(IdBimap INTERFACE include/)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# Add the test files
set(TEST_SOURCES
    test/simple_test.cpp
)

# Create an executable for the tests
add_executable(simple_test ${TEST_SOURCES})

# Link the library and the test executable
target_link_libraries(simple_test gtest gtest_main IdBimap ${CMAKE_THREAD_LIBS_INIT})

# Discover and run the tests
gtest_discover_tests(simple_test)

enable_testing()