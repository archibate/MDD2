cmake_minimum_required(VERSION 3.2)

project(NescMD LANGUAGES CXX)

add_library(NescMD::NescMD STATIC IMPORTED)
set_target_properties(NescMD::NescMD PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libnescmd.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(NescMD::NescMD_shared SHARED IMPORTED)
set_target_properties(NescMD::NescMD_shared PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libnescmd.so"
              INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/libciul.so.1"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")
