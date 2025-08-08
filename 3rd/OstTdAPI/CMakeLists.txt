cmake_minimum_required(VERSION 3.2)

add_library(OstTdAPI_imp SHARED IMPORTED)
set_target_properties(OstTdAPI_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libutapi.so"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(OstTdAPI INTERFACE)
target_link_libraries(OstTdAPI INTERFACE OstTdAPI_imp)
