cmake_minimum_required(VERSION 3.10)

project(asynclog CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(asynclog OBJECT
    asynclog.cpp
    )

target_include_directories(asynclog PUBLIC .)

find_package(fmt)
target_link_libraries(asynclog PUBLIC fmt::fmt)
