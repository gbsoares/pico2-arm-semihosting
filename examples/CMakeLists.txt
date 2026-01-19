# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Pico-specific examples (semihosting transport)
if(CFG_PLATFORM STREQUAL "PICO")
    add_subdirectory(pico)
endif()

# Host-specific examples (filesystem transport)
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(host)
endif()