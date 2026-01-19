/**
 * @file heapInstStream.c
 * @brief Filesystem stream port implementation for heap instrumentation.
 *
 * This file implements the stream port interface using standard C file I/O
 * to write trace data directly to the host filesystem. This transport is
 * intended for host-based testing and instrumentation.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "heapInstStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File handle for trace output */
static FILE* g_trace_file = NULL;

/* Default trace filename */
#ifndef HEAPINST_TRACE_FILENAME
#define HEAPINST_TRACE_FILENAME "heap_trace.bin"
#endif

/* Environment variable to override trace filename */
#ifndef HEAPINST_TRACE_FILENAME_ENV
#define HEAPINST_TRACE_FILENAME_ENV "HEAPINST_TRACE_FILE"
#endif

/**
 * @brief Get the trace filename.
 *
 * Checks the environment variable first, falls back to default.
 */
static const char* get_trace_filename(void)
{
    const char* env_filename = getenv(HEAPINST_TRACE_FILENAME_ENV);
    if (env_filename != NULL && env_filename[0] != '\0') {
        return env_filename;
    }
    return HEAPINST_TRACE_FILENAME;
}

int heapInstStreamPort_Init(void)
{
    if (g_trace_file != NULL) {
        /* Already initialized */
        return 0;
    }

    const char* filename = get_trace_filename();
    g_trace_file = fopen(filename, "wb");

    return (g_trace_file != NULL) ? 0 : -1;
}

int heapInstStreamPort_Write(const void* data, size_t len)
{
    if (g_trace_file == NULL) {
        return -1;
    }

    size_t written = fwrite(data, 1, len, g_trace_file);
    if (written != len) {
        return -1;
    }

    return (int)written;
}

int heapInstStreamPort_Flush(void)
{
    if (g_trace_file == NULL) {
        return -1;
    }

    return fflush(g_trace_file);
}

int heapInstStreamPort_Close(void)
{
    if (g_trace_file != NULL) {
        fclose(g_trace_file);
        g_trace_file = NULL;
    }

    return 0;
}
