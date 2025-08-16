cmake_minimum_required(VERSION 3.2)

add_library(libmds_api_static STATIC IMPORTED)
set_target_properties(libmds_api_static PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libmds_api.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(libmds_api_shared STATIC IMPORTED)
set_target_properties(libmds_api_shared PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libmds_api.so"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(KrMdsAPI INTERFACE)
target_link_libraries(KrMdsAPI INTERFACE libmds_api_static)
add_library(KrMdsAPI_shared INTERFACE)
target_link_libraries(KrMdsAPI_shared INTERFACE libmds_api_shared)
