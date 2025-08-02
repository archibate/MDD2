cmake_minimum_required(VERSION 3.2)

add_library(fmtlog OBJECT fmtlog.cpp)
target_include_directories(fmtlog PUBLIC include)

find_package(fmt REQUIRED)
target_link_libraries(fmtlog PUBLIC fmt::fmt)
