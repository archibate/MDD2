cmake_minimum_required(VERSION 3.2)

add_library(XeleAPI_imp SHARED IMPORTED)
set_target_properties(XeleAPI_imp PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libXeleSecuritiesAPI.so"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(XeleAPI INTERFACE)
target_link_libraries(XeleAPI INTERFACE XeleAPI_imp)
