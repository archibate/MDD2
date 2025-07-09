cmake_minimum_required(VERSION 3.2)

add_library(NescMD_imp STATIC IMPORTED)
set_target_properties(NescMD_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libnescmd.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(NescMD_shared_imp SHARED IMPORTED)
set_target_properties(NescMD_shared_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libnescmd.so"
              INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/libciul.so.1"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(NescMD INTERFACE)
target_link_libraries(NescMD INTERFACE NescMD_imp)
add_library(NescMD_shared INTERFACE)
target_link_libraries(NescMD_shared INTERFACE NescMD_shared_imp)
