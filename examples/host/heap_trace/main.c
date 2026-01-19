/**
 * @file main.c
 * @brief Heap instrumentation demonstration for host systems.
 *
 * This example demonstrates the heap instrumentation library by performing
 * various heap operations (malloc, free, realloc) and writing the trace
 * data to a file on the filesystem.
 *
 * With HEAPINST_AUTO_WRAP enabled (default), all standard malloc/free/realloc
 * calls are automatically instrumented via linker --wrap flags.
 *
 * The trace file can be configured via environment variable:
 *   HEAPINST_TRACE_FILE=/path/to/trace.bin ./heap_trace_host
 *
 * Default output: heap_trace.bin in the current directory.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "heapInst/heapInst.h"

/* Number of allocations to demonstrate */
#define DEMO_ALLOC_COUNT 5

/* Sizes for demonstration allocations */
static const size_t g_alloc_sizes[DEMO_ALLOC_COUNT] = {32, 64, 128, 256, 512};

/**
 * @brief Get current timestamp in microseconds.
 *
 * Uses clock_gettime for microsecond resolution on POSIX systems.
 */
static uint64_t get_timestamp_us(void* ctx)
{
    (void)ctx;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

/**
 * @brief Log callback for diagnostic output.
 */
static void log_message(const char* msg, void* ctx)
{
    (void)ctx;
    fprintf(stderr, "[heapInst] %s", msg);
}

int main(void)
{
    void *ptrs[DEMO_ALLOC_COUNT] = {NULL};
    void *leaked_ptr = NULL;
    void *realloc_ptr = NULL;

    printf("=== Heap Instrumentation Demo (Host) ===\n");

    /*
     * Step 1: Register platform hooks before initializing heap instrumentation.
     * This provides the timestamp function needed for trace records.
     */
    heap_inst_platform_hooks_t hooks = {
        .timestamp_us = get_timestamp_us,
        .log = log_message,
        .lock = NULL,
        .unlock = NULL,
        .timestamp_ctx = NULL,
        .log_ctx = NULL,
        .lock_ctx = NULL,
        .unlock_ctx = NULL,
    };
    heap_inst_register_platform_hooks(&hooks);

    /*
     * Step 2: Initialize the heap instrumentation system.
     * With HEAPINST_AUTO_WRAP enabled (the default), all standard malloc/free/
     * realloc calls are automatically traced via linker --wrap flags.
     */
    heap_inst_init();
    printf("Heap instrumentation initialized\n");
    printf("Buffer capacity: %u records\n",
           (unsigned)heap_inst_get_buffer_capacity());
    printf("Trace output: heap_trace.bin (or HEAPINST_TRACE_FILE env var)\n");

    /*
     * Step 3: Demonstrate malloc operations with various sizes.
     * Each allocation is recorded in the trace buffer with a timestamp.
     */
    printf("\n--- Performing malloc operations ---\n");
    for (int i = 0; i < DEMO_ALLOC_COUNT; i++) {
        ptrs[i] = malloc(g_alloc_sizes[i]);
        if (ptrs[i] != NULL) {
            /* Write a pattern to the memory to simulate actual usage */
            memset(ptrs[i], (uint8_t)(i + 1), g_alloc_sizes[i]);
            printf("malloc(%zu) = %p\n", g_alloc_sizes[i], ptrs[i]);
        } else {
            printf("malloc(%zu) FAILED\n", g_alloc_sizes[i]);
        }
    }

    /*
     * Step 4: Demonstrate realloc operation.
     * Allocate a small block, then grow it. The trace captures both operations.
     */
    printf("\n--- Performing realloc operation ---\n");
    realloc_ptr = malloc(16);
    printf("Initial malloc(16) = %p\n", realloc_ptr);

    if (realloc_ptr != NULL) {
        void *new_ptr = realloc(realloc_ptr, 256);
        if (new_ptr != NULL) {
            printf("realloc(%p, 256) = %p\n", realloc_ptr, new_ptr);
            realloc_ptr = new_ptr;
        } else {
            printf("realloc FAILED, keeping original pointer\n");
        }
    }

    /*
     * Step 5: Demonstrate free operations.
     * Free most allocations but deliberately skip one to simulate a memory leak.
     */
    printf("\n--- Performing free operations ---\n");

    /* Intentionally leak the first allocation for demonstration */
    leaked_ptr = ptrs[0];
    ptrs[0] = NULL;
    printf("Intentionally leaking allocation at %p (simulated leak)\n",
           leaked_ptr);

    /* Free the remaining allocations */
    for (int i = 1; i < DEMO_ALLOC_COUNT; i++) {
        if (ptrs[i] != NULL) {
            printf("free(%p)\n", ptrs[i]);
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }

    /* Free the reallocated block */
    if (realloc_ptr != NULL) {
        printf("free(%p) [realloc block]\n", realloc_ptr);
        free(realloc_ptr);
        realloc_ptr = NULL;
    }

    /*
     * Step 6: Flush the trace buffer to ensure all records are written to disk.
     */
    printf("\n--- Flushing trace buffer ---\n");
    printf("Buffer contains %u records before flush\n",
           (unsigned)heap_inst_get_buffer_count());

    heap_inst_flush();
    printf("Trace buffer flushed\n");

    /*
     * Summary output
     */
    printf("\n=== Demo Complete ===\n");
    printf("Performed:\n");
    printf("  - %d malloc operations\n", DEMO_ALLOC_COUNT + 1);
    printf("  - 1 realloc operation\n");
    printf("  - %d free operations\n", DEMO_ALLOC_COUNT);
    printf("  - 1 intentional leak at %p\n", leaked_ptr);
    printf("\nAnalyze heap_trace.bin to see the full allocation timeline.\n");

    /*
     * Note: The leaked_ptr is intentionally not freed to demonstrate
     * how the trace data can be used to identify memory leaks.
     * In production code, this would be a bug!
     */
    (void)leaked_ptr;

    return 0;
}
