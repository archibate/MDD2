cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_ENABLE_EXPORTS ON)

option(BUILD_SPEED "Enable quick unity build" OFF)
option(RECORD_FACTORS "Record factor to CSV files" ON)
option(LIMIT_SPEED "Limit request speed < 300/s" ON)
option(ALWAYS_BUY "Always purchase regardless model" OFF)
option(DUMMY_QUANTITY "Use dummy 100 quantity for buy" OFF)
option(ASYNC_LOGGER "Enable spdlog async logger" OFF)
option(NO_EXCEPTION "Disable C++ RTTI and exceptions" OFF)
option(DETAIL_LOG "Enable detail log (trace level)" OFF)
option(SELL_GC001 "Sell GC001/R-001 on market end" OFF)
option(BEST_ORDER "Enable best-order-this-side report" OFF)
option(SPLIT_ORDER "Enable split order report" OFF)
option(BYPASS_OES "Buypass trade API, quote API only" OFF)
option(SZ_IS_SECOND "Shenzhen is 2nd machine" OFF)
set(TARGET_SECURITY REPLAY CACHE STRING "Choices: REPLAY NE XC OST")
set(TARGET_MARKET SH CACHE STRING "Choices: SH SZ")

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# find_program(CMAKE_CXX_COMPILER clang++)
find_program(CMAKE_CXX_COMPILER g++)

project(mdd_v2 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


file(GLOB ${PROJECT_NAME}_SOURCES src/*.cpp src/*.h)
if (TARGET_SECURITY MATCHES OST)
    file(GLOB OSTMD_SOURCES src/ostmd/*.cpp src/ostmd/*.h)
    list(APPEND ${PROJECT_NAME}_SOURCES ${OSTMD_SOURCES})
endif()
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
find_package(tsl-robin-map CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE tsl::robin_map)
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
elseif (TARGET_SECURITY MATCHES OST)
    add_subdirectory(3rd/OstMdAPI)
    target_link_libraries(${PROJECT_NAME} PRIVATE OstMdAPI)
    add_subdirectory(3rd/OstTdAPI)
    target_link_libraries(${PROJECT_NAME} PRIVATE OstTdAPI)
endif()
if (TARGET_SECURITY MATCHES XC OR TARGET_SECURITY MATCHES NE)
    add_subdirectory(3rd/XeleAPI)
    target_link_libraries(${PROJECT_NAME} PRIVATE XeleAPI)
endif()
add_subdirectory(3rd/header-only)
target_link_libraries(${PROJECT_NAME} PRIVATE header-only)
add_subdirectory(3rd/string9811)
target_link_libraries(${PROJECT_NAME} PRIVATE string9811)

if (BUILD_SPEED)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        UNITY_BUILD TRUE
        UNITY_BUILD_MODE BATCH
        UNITY_BUILD_BATCH_SIZE 8
    )
endif()

if (DETAIL_LOG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -ggdb3 -Werror=return-type)
endif()

if (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -ggdb3
        -fno-omit-frame-pointer
        -finline-limit=10
        -fno-inline-functions-called-once
        -fno-optimize-sibling-calls
        -Werror=return-type
    )
endif()

if (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo" OR CMAKE_BUILD_TYPE MATCHES "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        if (TARGET_SECURITY MATCHES REPLAY)
            if (CMAKE_BUILD_TYPE MATCHES "Release")
                target_compile_options(${PROJECT_NAME} PRIVATE -march=native -mavx512dq -mavx512f)
            endif()
        elseif (TARGET_SECURITY MATCHES NE)
            target_compile_options(${PROJECT_NAME} PRIVATE -march=btver2 -mcx16 -msahf -mmovbe -maes -mpclmul -mpopcnt -mabm -mno-lwp -mfma -mno-fma4 -mno-xop -mbmi -mbmi2 -mno-tbm -mavx -mavx2 -msse4.2 -msse4.1 -mlzcnt -mno-rtm -mno-hle -mrdrnd -mf16c -mfsgsbase -mrdseed -mprfchw -madx -mfxsr -mxsave -mxsaveopt -mpku -mavx512dq -mavx512f)
        elseif (TARGET_SECURITY MATCHES XC)
            target_compile_options(${PROJECT_NAME} PRIVATE -march=core-avx2 -mcx16 -msahf -mmovbe -maes -mpclmul -mpopcnt -mabm -mno-lwp -mfma -mno-fma4 -mno-xop -mbmi -mbmi2 -mno-tbm -mavx -mavx2 -msse4.2 -msse4.1 -mlzcnt -mno-rtm -mno-hle -mrdrnd -mf16c -mfsgsbase -mrdseed -mprfchw -madx -mfxsr -mxsave -mxsaveopt -mpku -mtune=generic)
        elseif (TARGET_SECURITY MATCHES OST)
            target_compile_options(${PROJECT_NAME} PRIVATE -mavx2)
        endif()
    endif()
endif()

if (NOT CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS ON)
endif()

# if (CMAKE_BUILD_TYPE MATCHES "Release")
    # include(CheckIPOSupported)
    # check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
    # if (IPO_SUPPORTED)
    #     set_target_properties(${PROJECT_NAME} PROPERTIES
    #         INTERPROCEDURAL_OPTIMIZATION TRUE
    #     )
    # endif()
    #
# endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fcf-protection=none
            -fno-stack-clash-protection
            -fno-stack-protector
            -fno-asynchronous-unwind-tables
        )
    endif()
endif()

if (NO_EXCEPTION)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE
            -fno-exception
            -fno-rtti
        )
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -w)
endif()
