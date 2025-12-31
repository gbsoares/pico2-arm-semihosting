# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Examples are currently Pico-specific
if(CFG_PLATFORM STREQUAL "PICO")
    add_subdirectory(semihosting)
else()
    message(STATUS "Skipping examples - only available for Pico platform")
endif()