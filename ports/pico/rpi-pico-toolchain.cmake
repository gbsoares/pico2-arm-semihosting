# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# Raspberry Pi Pico SDK Toolchain Configuration
# This file separates Pico-specific dependencies from the main project CMake configuration
# following Beman project guidelines for platform-specific toolchain handling.

# Pull in SDK (must be before project)
include(${CMAKE_CURRENT_LIST_DIR}/pico_sdk_import.cmake)

# Note: This file should be included before the project() command in the root CMakeLists.txt
# The actual pico_sdk_init() call should happen after the project() command.
