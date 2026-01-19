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
option(
    CFG_BUILD_TESTS
    "Enable building tests. Tests run on host; skipped when cross-compiling."
    OFF
)

option(
    HEAPINST_AUTO_WRAP
    "Enable automatic malloc/free/realloc instrumentation via linker wrapping."
    ON
)

# [CMAKE.SKIP_EXAMPLES]
option(
    CFG_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    HEAPINST_DEBUG_LOG
    "Enable debug logging in heap instrumentation library. Default: ON."
    ON
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
target_sources(heapInstStream
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
        FILES ${PROJECT_SOURCE_DIR}/include/heapInstStream.h
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

# Configure debug logging
if(HEAPINST_DEBUG_LOG)
    target_compile_definitions(heapInstCore PRIVATE HEAPINST_CFG_DEBUG_LOG=1)
else()
    target_compile_definitions(heapInstCore PRIVATE HEAPINST_CFG_DEBUG_LOG=0)
endif()

# -----------------------------------------------------------------------------
# Automatic malloc/free wrapping (works on any platform with GNU linker)
# -----------------------------------------------------------------------------
# When enabled, all malloc/free/realloc/calloc calls are automatically
# instrumented via linker --wrap flags. This works on Linux hosts and
# embedded targets using the GNU toolchain.
#
# Applications must include heapInst_wrap.c in their sources:
#   add_executable(my_app
#       ${heapInst_SOURCE_DIR}/src/heapInst_wrap.c
#       main.c
#   )
if(HEAPINST_AUTO_WRAP)
    # Export source directory for applications to reference heapInst_wrap.c
    set(heapInst_SOURCE_DIR ${PROJECT_SOURCE_DIR} CACHE PATH
        "heapInst source directory for accessing heapInst_wrap.c")

    # Add linker wrap flags to redirect malloc/free/realloc/calloc
    target_link_options(heapInstCore PUBLIC
        -Wl,--wrap=malloc
        -Wl,--wrap=calloc
        -Wl,--wrap=realloc
        -Wl,--wrap=free
    )

    message(STATUS "HEAPINST_AUTO_WRAP=ON: malloc/free/realloc/calloc will be automatically instrumented")
endif()

# -----------------------------------------------------------------------------
# Stream transport libraries
# -----------------------------------------------------------------------------
# Each stream transport provides heapInstStreamPort_* symbols. Applications link
# both heapInstCore and their chosen stream transport library.

# Semihosting transport (Pico only - requires SDK and debugger)
if(CFG_PLATFORM STREQUAL "PICO")
    add_subdirectory(stream/semihosting)
endif()

# Filesystem transport (host only - uses standard C file I/O)
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(stream/filesystem)
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
    add_subdirectory(examples)
endif()
