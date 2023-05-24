cmake_minimum_required(VERSION 3.16)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(Testing)

add_library(IdBimap INTERFACE)

target_include_directories(IdBimap INTERFACE include/)

# turn off compile error messages
add_compile_definitions(NFAIL)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

add_executable(runTests tests.cpp)

target_link_libraries(runTests GTest::Main IdBimap ${CMAKE_THREAD_LIBS_INIT})