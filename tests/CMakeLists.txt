# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

include(FetchContent)

set(_gtest_target "GTest::gtest")
if(NOT TARGET ${_gtest_target})
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        FIND_PACKAGE_ARGS NAMES GTest)
    FetchContent_MakeAvailable(googletest)
endif()

add_library(heap_tracker_obj OBJECT
    ${PROJECT_SOURCE_DIR}/src/heap_instrumentation/heap_tracker.c
)
target_include_directories(heap_tracker_obj
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/config
)
target_compile_definitions(heap_tracker_obj PRIVATE HEAPTRC_TEST_API HEAPTRC_CFG_BUFFER_SIZE=256)
set_property(TARGET heap_tracker_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(heap_tracker_tests
    heap_tracker_test.cpp
    heap_tracker_test_shims.c
)
target_include_directories(heap_tracker_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/config
)
target_link_libraries(heap_tracker_tests
    PRIVATE
        ${_gtest_target}
        GTest::gtest_main
        heap_tracker_obj
)

include(GoogleTest)
gtest_discover_tests(heap_tracker_tests)
