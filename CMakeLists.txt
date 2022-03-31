cmake_minimum_required(VERSION 3.16)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(Testing)

add_library(IdBimap id_bimap.h)
set_target_properties(IdBimap PROPERTIES LINKER_LANGUAGE CXX)

# turn off compile error messages
add_compile_definitions(NFAIL)

find_package (Threads)

# add the executable
add_executable(Testing main.cpp)
target_link_libraries(Testing ${CMAKE_THREAD_LIBS_INIT})
