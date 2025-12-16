# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.25)

# Pull in SDK import (must be before project) for Pico platform
if(SEMIHOSTING_PLATFORM STREQUAL "PICO")
    include(ports/pico/pico_sdk_import.cmake)
endif()

project(pico.semihosting 
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "ARM semihosting library for Raspberry Pi Pico microcontrollers"
)

# Include platform-specific initialization (SDK init and dependency injection)
if(SEMIHOSTING_PLATFORM STREQUAL "PICO")
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

add_subdirectory(src/semihosting)

# Conditionally build tests
if(PICO_SEMIHOSTING_BUILD_TESTS)
    # add_subdirectory(tests)  # Uncomment when tests are added
endif()

# Conditionally build examples
if(PICO_SEMIHOSTING_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()