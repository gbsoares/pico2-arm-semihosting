# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Heap trace example demonstrating heap instrumentation with semihosting transport
set(TARGET_NAME heap_trace)

add_executable(${TARGET_NAME} main.c)

target_link_libraries(${TARGET_NAME}
    pico_stdlib
    heapInstCore
    semihosting
    pico_platform_hooks
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_none)
endif()

# Generate additional output formats (map/bin/hex/uf2)
if(CFG_PLATFORM STREQUAL "PICO" AND COMMAND pico_add_extra_outputs)
    pico_add_extra_outputs(${TARGET_NAME})
endif()
