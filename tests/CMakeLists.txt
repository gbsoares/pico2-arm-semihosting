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

add_library(heap_inst_obj OBJECT
    ${PROJECT_SOURCE_DIR}/src/heapInst.c
)
target_include_directories(heap_inst_obj
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/config
)
target_compile_definitions(heap_inst_obj PRIVATE HEAPINST_TEST_API HEAPINST_CFG_BUFFER_SIZE=256)
set_property(TARGET heap_inst_obj PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable(heap_inst_tests
    heapInstTest.cpp
    heapInstTestShims.c
)
target_include_directories(heap_inst_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/config
)
target_compile_definitions(heap_inst_tests PRIVATE HEAPINST_TEST_API)
target_link_libraries(heap_inst_tests
    PRIVATE
        ${_gtest_target}
        GTest::gtest_main
        heap_inst_obj
)

include(GoogleTest)
gtest_discover_tests(heap_inst_tests)
