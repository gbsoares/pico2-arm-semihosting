# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
cmake_minimum_required(VERSION 3.25)

# Pull in Raspberry Pi Pico SDK dependencies (must be before project)
include(ports/pico/rpi-pico-toolchain.cmake)

project(pico.semihosting 
    VERSION 0.1.0
    LANGUAGES C CXX ASM
    DESCRIPTION "ARM semihosting library for Raspberry Pi Pico microcontrollers"
)

# Verify SDK version requirement
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.2.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the Pico SDK
pico_sdk_init()

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