cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if (NOT DEFINED BUILD_SPEED)
    set(BUILD_SPEED OFF)
endif()
if (NOT DEFINED BUILD_FOR_SZ)
    set(BUILD_FOR_SZ OFF)
endif()
if (NOT DEFINED REPLAY)
    set(REPLAY ON)
endif()
if (NOT DEFINED ASYNC_LOGGER)
    set(ASYNC_LOGGER OFF)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
find_program(CMAKE_CXX_COMPILER clang++)

project(mdd_v2 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


file(GLOB ${PROJECT_NAME}_SOURCES src/*.cpp src/*.h)
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
find_package(absl CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE absl::btree absl::flat_hash_map)
find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)
find_package(TBB CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE TBB::tbb TBB::tbbmalloc)

configure_file(src/config.h.in include/config.h @ONLY)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(3rd/XeleAPI)
target_link_libraries(${PROJECT_NAME} PRIVATE XeleAPI)
add_subdirectory(3rd/XeleMD)
target_link_libraries(${PROJECT_NAME} PRIVATE XeleMD)
add_subdirectory(3rd/Disruptor)
target_link_libraries(${PROJECT_NAME} PRIVATE Disruptor::Disruptor)
add_subdirectory(3rd/header-only)
target_link_libraries(${PROJECT_NAME} PRIVATE header-only)

if (CMAKE_BUILD_TYPE MATCHES "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
    if (IPO_SUPPORTED)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE
        )
    endif()

    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-mavx2 HAS_AVX2)
    if (HAS_AVX2)
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mfma -mf16c)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE -w)
endif()

if (BUILD_SPEED)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        UNITY_BUILD TRUE
        UNITY_BUILD_MODE BATCH
        UNITY_BUILD_BATCH_SIZE 8
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

if (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" OR CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -ggdb -Werror=return-type)
endif()

if (BUILD_FOR_SZ)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSZ)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSH)
endif()
