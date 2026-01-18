# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.25)

# Pull in SDK import (must be before project) for Pico platform
if(CFG_PLATFORM STREQUAL "PICO")
    include(ports/pico/pico_sdk_import.cmake)
endif()

project(heapInst 
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "ARM semihosting library for Raspberry Pi Pico microcontrollers"
)

# -----------------------------------------------------------------------------
# Build controls
# -----------------------------------------------------------------------------
# [CMAKE.SKIP_TESTS]
option(
    CFG_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: OFF. Values: { ON, OFF }."
    OFF
)

option(
    HEAPINST_TESTS_ONLY
    "Skip transport library configuration for host-only test builds."
    OFF
)

option(
    HEAPINST_ENABLE_TESTS
    "Convenience flag: enable tests and skip transport configuration for host-only test builds."
    OFF
)

if(HEAPINST_ENABLE_TESTS)
    set(CFG_BUILD_TESTS ON CACHE BOOL "Enable building tests and test infrastructure." FORCE)
    set(HEAPINST_TESTS_ONLY ON CACHE BOOL "Skip transport library configuration." FORCE)
endif()

# [CMAKE.SKIP_EXAMPLES]
option(
    CFG_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# Include platform-specific initialization (SDK init and dependency injection)
if(CFG_PLATFORM STREQUAL "PICO")
    include(ports/pico/pico-semihosting.cmake)
endif()

# -----------------------------------------------------------------------------
# Stream port interface (header-only)
# -----------------------------------------------------------------------------
# This INTERFACE library provides the common heapInstStream.h header that defines
# the API contract between heapInstCore and stream transport implementations.
add_library(heapInstStream INTERFACE)
target_include_directories(heapInstStream
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

# -----------------------------------------------------------------------------
# Heap instrumentation core
# -----------------------------------------------------------------------------
add_library(heapInstCore STATIC
    src/heapInst.c
)
target_include_directories(heapInstCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/config
)
target_link_libraries(heapInstCore
    PRIVATE
        heapInstStream
)
set_property(TARGET heapInstCore PROPERTY C_STANDARD 11)

# -----------------------------------------------------------------------------
# Stream transport libraries
# -----------------------------------------------------------------------------
# Each stream transport provides heapInstStreamPort_* symbols. Applications link
# both heapInstCore and their chosen stream transport library.
# The common heapInstStream.h interface is in include/ so heapInstCore can compile
# without knowing which transport will be linked.
if(NOT HEAPINST_TESTS_ONLY)
    add_subdirectory(stream/semihosting)
else()
    message(STATUS "HEAPINST_TESTS_ONLY=ON: skipping transport library configuration")
endif()

# -----------------------------------------------------------------------------
# Unit tests (host)
# -----------------------------------------------------------------------------
if(CFG_BUILD_TESTS)
    include(CTest)
    
    if(CMAKE_CROSSCOMPILING)
        message(STATUS "Skipping tests - cross compiling")
    else()
        add_subdirectory(tests)
    endif()
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if(CFG_BUILD_EXAMPLES)
    if(CFG_BUILD_TESTS)
        message(STATUS "CFG_BUILD_TESTS=ON: skipping examples")
    else()
        add_subdirectory(examples)
    endif()
endif()
