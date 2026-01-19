# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Heap trace example demonstrating heap instrumentation with filesystem transport

set(TARGET_NAME heap_trace_host)

# IMPORTANT: Include heapInst_wrap.c FIRST in the executable sources.
# This ensures our __wrap_malloc is compiled and linked before any other
# definitions that might exist.
add_executable(${TARGET_NAME}
    ${heapInst_SOURCE_DIR}/src/heapInst_wrap.c
    main.c
)

target_link_libraries(${TARGET_NAME}
    heapInstCore
    heapInstFilesystem
)
