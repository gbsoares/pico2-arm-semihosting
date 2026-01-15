/**
 * @file pico_platform_hooks.c
 * @brief Unified Pico SDK platform hooks implementation for heap instrumentation.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "pico_platform_hooks.h"

#include "heapInst/heapInst.h"
#include "pico/time.h"

uint64_t pico_platform_timestamp_us(void *ctx)
{
    (void)ctx; /* unused */
    return time_us_64();
}

void pico_platform_hooks_register(void)
{
    heap_inst_platform_hooks_t hooks = {
        .timestamp_us = pico_platform_timestamp_us,
        .timestamp_ctx = NULL,
        .log = NULL,
        .log_ctx = NULL,
        .lock = NULL,
        .lock_ctx = NULL,
        .unlock = NULL,
        .unlock_ctx = NULL,
    };

    heap_inst_register_platform_hooks(&hooks);
}
