cmake_minimum_required(VERSION 3.2)

project(XeleMD LANGUAGES CXX)

add_library(XeleMD::XeleMD STATIC IMPORTED)
set_target_properties(XeleMD::XeleMD PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libxelemd.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")
