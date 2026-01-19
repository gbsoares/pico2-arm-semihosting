# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

add_library(heapInstFilesystem STATIC)

target_sources(heapInstFilesystem
    PRIVATE
        heapInstStream.c
)

# Link to heapInstStream interface for the common stream port API header
target_link_libraries(heapInstFilesystem
    PRIVATE
        heapInstStream
)

set_property(TARGET heapInstFilesystem PROPERTY C_STANDARD 11)
