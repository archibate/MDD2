cmake_minimum_required(VERSION 3.2)

project(Disruptor LANGUAGES CXX)

file(GLOB_RECURSE header include/*.h)
file(GLOB_RECURSE source src/*.cpp)

add_library(Disruptor OBJECT ${header} ${source})
target_include_directories(Disruptor PRIVATE include/Disruptor)
target_include_directories(Disruptor PUBLIC include)
target_compile_features(Disruptor PRIVATE cxx_std_11)

add_library(Disruptor::Disruptor ALIAS Disruptor)

if (ZHAO_BUILD_SPEED)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        UNITY_BUILD TRUE
        UNITY_BUILD_MODE BATCH
        UNITY_BUILD_BATCH_SIZE 8
    )
    target_compile_options(${PROJECT_NAME} PRIVATE -w)
endif()
