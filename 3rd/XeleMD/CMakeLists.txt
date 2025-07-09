cmake_minimum_required(VERSION 3.2)

add_library(XeleMD_imp STATIC IMPORTED)
set_target_properties(XeleMD_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libxelemd.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(XeleMD INTERFACE)
target_link_libraries(XeleMD INTERFACE XeleMD_imp)
