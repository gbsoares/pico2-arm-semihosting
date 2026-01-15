# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Raspberry Pi Pico platform-specific configuration for semihosting library

# Initialize the Pico SDK
pico_sdk_init()

# Verify SDK version requirement (must be after pico_sdk_init)
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.2.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Function to add Pico-specific dependencies to the semihosting library
function(semihosting_add_pico_dependencies target)
    # Link required Pico SDK libraries for hardware functionality
    target_link_libraries(${target}
        PUBLIC
            pico_stdlib
            hardware_exception
    )

    # Optionally add any Pico-specific compile definitions
    # target_compile_definitions(${target} PRIVATE VAR_NAME=VALUE)
endfunction()

# -----------------------------------------------------------------------------
# Pico platform hooks library
# -----------------------------------------------------------------------------
# Provides unified platform hook implementations (timestamp, logging, locking)
# using RP2040/RP2350 peripherals. This library can be linked by applications
# that need platform-specific hook support for heap instrumentation.

add_library(pico_platform_hooks STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/pico_platform_hooks.c
)

target_include_directories(pico_platform_hooks
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pico_platform_hooks
    PUBLIC
        heapInstCore
        pico_time
)
