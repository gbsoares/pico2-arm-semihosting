# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.25)

# Pull in SDK import (must be before project) for Pico platform
if(CFG_PLATFORM STREQUAL "PICO")
    include(ports/pico/pico_sdk_import.cmake)
endif()

project(heapTracer 
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "ARM semihosting library for Raspberry Pi Pico microcontrollers"
)

# Select heap trace stream backend (set by toolchain/platform; default to semihosting)
if(NOT DEFINED CFG_STREAM_METHOD)
    set(CFG_STREAM_METHOD "semihosting" CACHE STRING "Heap trace stream backend (e.g. semihosting, udp)" FORCE)
endif()
set_property(CACHE CFG_STREAM_METHOD PROPERTY STRINGS semihosting udp)

# Include platform-specific initialization (SDK init and dependency injection)
if(CFG_PLATFORM STREQUAL "PICO")
    include(ports/pico/pico-semihosting.cmake)
endif()

# [CMAKE.SKIP_TESTS]
option(
    PICO_SEMIHOSTING_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    PICO_SEMIHOSTING_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

set(_heaptrc_stream_dir "${CMAKE_CURRENT_SOURCE_DIR}/stream/${CFG_STREAM_METHOD}")
if(EXISTS "${_heaptrc_stream_dir}/CMakeLists.txt")
    add_subdirectory("${_heaptrc_stream_dir}" "stream/${CFG_STREAM_METHOD}")
else()
    message(FATAL_ERROR "CFG_STREAM_METHOD='${CFG_STREAM_METHOD}' has no CMakeLists.txt under ${_heaptrc_stream_dir}")
endif()

# Conditionally build tests
if(PICO_SEMIHOSTING_BUILD_TESTS)
    # add_subdirectory(tests)  # Uncomment when tests are added
endif()

# Conditionally build examples
if(PICO_SEMIHOSTING_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
