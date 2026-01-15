/**
 * @file pico_platform_hooks.h
 * @brief Unified Pico SDK platform hooks for heap instrumentation.
 *
 * Provides platform-specific implementations of timestamp, logging, and locking
 * hooks for the RP2040/RP2350 platforms. These hooks can be registered with the
 * heap instrumentation core via heap_inst_register_platform_hooks().
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register all available Pico platform hooks with heap instrumentation.
 *
 * Convenience function that registers all implemented platform hooks (timestamp,
 * logging, locking) with the heap instrumentation core. This should be called
 * before heap_inst_init() to ensure all hooks are available from the start.
 *
 * Currently registers:
 * - Timestamp hook using the Pico SDK timer peripheral
 *
 * Future hooks (not yet implemented):
 * - Logging hook
 * - Lock/unlock hooks for thread safety
 */
void pico_platform_hooks_register(void);

/* -------------------------------------------------------------------------
 * Individual hook functions for fine-grained control
 * ------------------------------------------------------------------------- */

/**
 * @brief Get current timestamp in microseconds.
 *
 * Returns the number of microseconds since boot using the Pico SDK timer
 * peripheral. This function is compatible with both RP2040 (Pico 1) and
 * RP2350 (Pico 2) platforms.
 *
 * This function can be used directly when registering hooks manually via
 * heap_inst_register_platform_hooks() for fine-grained control over which
 * hooks are enabled.
 *
 * @param ctx Unused context pointer (for API compatibility).
 * @return Current timestamp in microseconds since boot.
 */
uint64_t pico_platform_timestamp_us(void *ctx);

#ifdef __cplusplus
}
#endif
