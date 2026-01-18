# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Heap trace example demonstrating heap instrumentation with semihosting transport
set(TARGET_NAME heap_trace)

# IMPORTANT: Include heapInst_wrap.c FIRST in the executable sources.
# This ensures our __wrap_malloc is compiled and linked before Pico SDK's pico_malloc
# INTERFACE sources (which also define __wrap_malloc).
add_executable(${TARGET_NAME}
    ${heapInst_SOURCE_DIR}/src/heapInst_wrap.c
    main.c
)

target_link_libraries(${TARGET_NAME}
    heapInstCore
    pico_platform_hooks
    semihosting
    pico_stdlib
)

# Add Pico-specific linker options (--allow-multiple-definition for pico_malloc compatibility)
if(HEAPINST_PICO_LINK_OPTIONS)
    target_link_options(${TARGET_NAME} PRIVATE ${HEAPINST_PICO_LINK_OPTIONS})
endif()

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_none)
endif()

# Generate additional output formats (map/bin/hex/uf2)
if(CFG_PLATFORM STREQUAL "PICO" AND COMMAND pico_add_extra_outputs)
    pico_add_extra_outputs(${TARGET_NAME})
endif()
