cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(BUILD_SPEED "Enable quick unity build" OFF)
option(RECORD_FACTORS "Record factor to CSV files" ON)
option(ALWAYS_BUY "Always purchase regardless model" OFF)
option(ASYNC_LOGGER "Enable spdlog async logger" OFF)
set(TARGET_SECURITY REPLAY CACHE STRING "Choices: REPLAY NE XC")
set(TARGET_MARKET SH CACHE STRING "Choices: SH SZ")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# find_program(CMAKE_CXX_COMPILER clang++)
find_program(CMAKE_CXX_COMPILER g++)

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

if (TARGET_SECURITY MATCHES NE)
    add_subdirectory(3rd/NescMD)
    target_link_libraries(${PROJECT_NAME} PRIVATE NescMD)
elseif (TARGET_SECURITY MATCHES XC)
    add_subdirectory(3rd/XeleMD)
    target_link_libraries(${PROJECT_NAME} PRIVATE XeleMD)
endif()
if (TARGET_SECURITY MATCHES XC OR TARGET_SECURITY MATCHES NE)
    add_subdirectory(3rd/XeleAPI)
    target_link_libraries(${PROJECT_NAME} PRIVATE XeleAPI)
endif()
add_subdirectory(3rd/header-only)
target_link_libraries(${PROJECT_NAME} PRIVATE header-only)

if (BUILD_SPEED)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        UNITY_BUILD TRUE
        UNITY_BUILD_MODE BATCH
        UNITY_BUILD_BATCH_SIZE 8
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -ggdb -Werror=return-type)
endif()

if (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -ggdb3
        -fno-omit-frame-pointer
        -finline-limit=50
    )
endif()

if (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" OR CMAKE_BUILD_TYPE MATCHES "Release")
    if (TARGET_SECURITY MATCHES REPLAY)
        target_compile_options(${PROJECT_NAME} PRIVATE -march=native -mavx512dq -mavx512f)
    elseif (TARGET_SECURITY MATCHES NE)
        target_compile_options(${PROJECT_NAME} PRIVATE -march=btver2 -mcx16 -msahf -mmovbe -maes -mpclmul -mpopcnt -mabm -mno-lwp -mfma -mno-fma4 -mno-xop -mbmi -mbmi2 -mno-tbm -mavx -mavx2 -msse4.2 -msse4.1 -mlzcnt -mno-rtm -mno-hle -mrdrnd -mf16c -mfsgsbase -mrdseed -mprfchw -madx -mfxsr -mxsave -mxsaveopt -mpku -mavx512dq -mavx512f)
    elseif (TARGET_SECURITY MATCHES XC)
        target_compile_options(${PROJECT_NAME} PRIVATE -march=core-avx2 -mcx16 -msahf -mmovbe -maes -mpclmul -mpopcnt -mabm -mno-lwp -mfma -mno-fma4 -mno-xop -mbmi -mbmi2 -mno-tbm -mavx -mavx2 -msse4.2 -msse4.1 -mlzcnt -mno-rtm -mno-hle -mrdrnd -mf16c -mfsgsbase -mrdseed -mprfchw -madx -mfxsr -mxsave -mxsaveopt -mpku -mtune=generic)
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
    if (IPO_SUPPORTED)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION TRUE
        )
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE -w)
endif()
