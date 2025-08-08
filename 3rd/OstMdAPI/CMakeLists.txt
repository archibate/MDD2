cmake_minimum_required(VERSION 3.2)

add_library(libciul1_imp STATIC IMPORTED)
set_target_properties(libciul1_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libciul1.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(libonload_ext_imp STATIC IMPORTED)
set_target_properties(libonload_ext_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libonload_ext.a"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(OstMdAPI INTERFACE)
target_link_libraries(OstMdAPI INTERFACE libciul1_imp libonload_ext_imp)
