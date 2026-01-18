/**
 * @file main.c
 * @brief Heap instrumentation demonstration for Pico 2.
 *
 * This example demonstrates the heap instrumentation library by performing
 * various heap operations (malloc, free, realloc) and streaming the trace
 * data to the debugger host via ARM semihosting.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <stdio.h>
#include <string.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include "heapInst/heapInst.h"
#include "pico_platform_hooks.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

/* Number of allocations to demonstrate */
#define DEMO_ALLOC_COUNT 5

/* Sizes for demonstration allocations */
static const size_t g_alloc_sizes[DEMO_ALLOC_COUNT] = {32, 64, 128, 256, 512};

/**
 * @brief Toggle LED state for visual feedback.
 *
 * @param on true to turn LED on, false to turn off.
 */
static void led_set(bool on)
{
#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on ? 1 : 0);
#else
    gpio_put(PICO_DEFAULT_LED_PIN, on ? 1 : 0);
#endif
}

/**
 * @brief Blink LED to indicate progress.
 *
 * @param count Number of blinks.
 * @param on_ms Duration LED is on (milliseconds).
 * @param off_ms Duration LED is off (milliseconds).
 */
static void led_blink(int count, uint32_t on_ms, uint32_t off_ms)
{
    for (int i = 0; i < count; i++) {
        led_set(true);
        sleep_ms(on_ms);
        led_set(false);
        sleep_ms(off_ms);
    }
}

/**
 * @brief Initialize the LED GPIO.
 *
 * @return 0 on success, non-zero on failure.
 */
static int led_init(void)
{
#ifdef CYW43_WL_GPIO_LED_PIN
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43\n");
        return -1;
    }
#else
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
    return 0;
}

/**
 * @brief Deinitialize the LED GPIO.
 */
static void led_deinit(void)
{
#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_deinit();
#endif
}

int main(void)
{
    void *ptrs[DEMO_ALLOC_COUNT] = {NULL};
    void *leaked_ptr = NULL;
    void *realloc_ptr = NULL;

    /*
     * Step 1: Register platform hooks before initializing heap instrumentation.
     * This provides the timestamp function needed for trace records.
     */
    pico_platform_hooks_register();

    /* Initialize standard I/O for UART output */
    stdio_init_all();

    /* Initialize LED for visual progress indication */
    if (led_init() != 0) {
        return 1;
    }

    /* Blink twice to indicate program start */
    led_blink(2, 100, 100);

    printf("=== Heap Instrumentation Demo ===\n");

    /*
     * Step 2: Initialize the heap instrumentation system.
     * After this call, all heap_inst_malloc/free/realloc calls will be traced.
     */
    heap_inst_init();
    printf("Heap instrumentation initialized\n");
    printf("Buffer capacity: %u records\n",
                    (unsigned)heap_inst_get_buffer_capacity());

    /*
     * Step 3: Demonstrate malloc operations with various sizes.
     * Each allocation is recorded in the trace buffer with a timestamp.
     */
    printf("\n--- Performing malloc operations ---\n");
    for (int i = 0; i < DEMO_ALLOC_COUNT; i++) {
        ptrs[i] = heap_inst_malloc(g_alloc_sizes[i]);
        if (ptrs[i] != NULL) {
            /* Write a pattern to the memory to simulate actual usage */
            memset(ptrs[i], (uint8_t)(i + 1), g_alloc_sizes[i]);
            printf("malloc(%u) = %p\n", (unsigned)g_alloc_sizes[i],
                            ptrs[i]);
        } else {
            printf("malloc(%u) FAILED\n", (unsigned)g_alloc_sizes[i]);
        }
    }

    /* Blink to indicate malloc phase complete */
    led_blink(1, 200, 100);

    /*
     * Step 4: Demonstrate realloc operation.
     * Allocate a small block, then grow it. The trace captures both operations.
     */
    printf("\n--- Performing realloc operation ---\n");
    realloc_ptr = heap_inst_malloc(16);
    printf("Initial malloc(16) = %p\n", realloc_ptr);

    if (realloc_ptr != NULL) {
        void *new_ptr = heap_inst_realloc(realloc_ptr, 256);
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
            heap_inst_free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }

    /* Free the reallocated block */
    if (realloc_ptr != NULL) {
        printf("free(%p) [realloc block]\n", realloc_ptr);
        heap_inst_free(realloc_ptr);
        realloc_ptr = NULL;
    }

    /* Blink to indicate free phase complete */
    led_blink(1, 200, 100);

    /*
     * Step 6: Flush the trace buffer to ensure all records are sent to the host.
     * In a real application, you might flush periodically or when the buffer
     * reaches a certain threshold.
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
    printf("\nAnalyze the trace data on the host to see the full\n");
    printf("allocation timeline with timestamps.\n");

    /* Final blink pattern to indicate completion (3 rapid blinks) */
    led_blink(3, 50, 50);

    /* Keep LED on for a few seconds to indicate success */
    led_set(true);
    sleep_ms(3000);
    led_set(false);

    led_deinit();

    /*
     * Note: The leaked_ptr is intentionally not freed to demonstrate
     * how the trace data can be used to identify memory leaks.
     * In production code, this would be a bug!
     */
    (void)leaked_ptr;

    return 0;
}
