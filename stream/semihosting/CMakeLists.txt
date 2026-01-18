# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(semihosting)

target_sources(semihosting
    PRIVATE
        semihosting.c
        heapInstStream.c
)

target_sources(
    semihosting
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS
                "${CMAKE_CURRENT_SOURCE_DIR}/include"
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/include/semihosting.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/include/heapInstStream.h"
)

# Export include directories so consumers (heapInstCore, examples) can find heapInstStream.h
target_include_directories(semihosting
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specific dependencies are injected via platform configuration
# For Pico: see ports/pico/pico-semihosting.cmake
if(CFG_PLATFORM STREQUAL "PICO" AND COMMAND semihosting_add_pico_dependencies)
    semihosting_add_pico_dependencies(semihosting)
endif()

# Following [cmake.passive_projects] - do not set CMAKE_C_STANDARD globally
# Instead, verify compiler support at configuration time if needed
set_property(TARGET semihosting PROPERTY C_STANDARD 11)