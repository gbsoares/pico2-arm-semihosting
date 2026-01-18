/**
 * @file heapInstStream.c
 * @brief Semihosting stream port implementation for heap instrumentation.
 *
 * This file implements the stream port interface using ARM semihosting
 * to transfer trace data to the debugger host.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "heapInstStream.h"
#include "semihosting.h"

/* File handle for trace output */
static int g_trace_handle = -1;

/* Default trace filename on host */
#ifndef HEAPINST_TRACE_FILENAME
#define HEAPINST_TRACE_FILENAME "heap_trace.bin"
#endif

int heapInstStreamPort_Init(void)
{
    /* Initialize semihosting with fault protection */
    semihosting_init();

    /* Check if debugger is attached */
    if (!semihosting_is_available()) {
        return -1;
    }

    /* Open trace file on host for binary write */
    g_trace_handle = semihosting_open(HEAPINST_TRACE_FILENAME, OPEN_MODE_WB);

    return (g_trace_handle >= 0) ? 0 : -1;
}

int heapInstStreamPort_Write(const void* data, size_t len)
{
    if (g_trace_handle < 0) {
        return -1;
    }

    return semihosting_write(g_trace_handle, data, len);
}

int heapInstStreamPort_Flush(void)
{
    /* Semihosting has no explicit flush mechanism */
    return 0;
}

int heapInstStreamPort_Close(void)
{
    if (g_trace_handle >= 0) {
        semihosting_close(g_trace_handle);
        g_trace_handle = -1;
    }

    return 0;
}
