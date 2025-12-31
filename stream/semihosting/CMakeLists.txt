# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(pico.semihosting)
add_library(pico::semihosting ALIAS pico.semihosting)

target_sources(pico.semihosting
    PRIVATE
        semihosting.c
        semihosting_ops.h
)

target_sources(
    pico.semihosting
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/../../include/semihosting/semihosting.h"
)

# Platform-specific dependencies are injected via platform configuration
# For Pico: see ports/pico/pico-semihosting.cmake
if(SEMIHOSTING_PLATFORM STREQUAL "PICO" AND COMMAND semihosting_add_pico_dependencies)
    semihosting_add_pico_dependencies(pico.semihosting)
endif()

# Following [cmake.passive_projects] - do not set CMAKE_C_STANDARD globally
# Instead, verify compiler support at configuration time if needed
set_property(TARGET pico.semihosting PROPERTY C_STANDARD 11)