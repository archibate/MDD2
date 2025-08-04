cmake_minimum_required(VERSION 3.10)

project(string9811 CXX)

add_library(string9811 OBJECT
    string98.cpp
    string11.cpp
    )

target_include_directories(string9811 PUBLIC .)
