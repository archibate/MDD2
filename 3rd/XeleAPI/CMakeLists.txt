cmake_minimum_required(VERSION 3.2)

project(XeleAPI LANGUAGES CXX)

add_library(XeleAPI::XeleAPI SHARED IMPORTED)
set_target_properties(XeleAPI::XeleAPI PROPERTIES
              IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libXeleSecuritiesAPI.so"
              INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include")
