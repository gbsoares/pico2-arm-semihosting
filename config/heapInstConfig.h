/*
 * Main configuration parameters for the heap trace library.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def HEAPINST_CFG_DEBUG_LOG
 * @brief Enable/disable debug logging via heap_inst_logf() calls.
 *
 * When enabled (1), the library prints diagnostic messages about heap
 * operations, initialization, and buffer flushes via the registered
 * log callback or stdout.
 *
 * When disabled (0), the library remains silent and only writes binary
 * trace data to the stream transport.
 *
 * Default: 1 (enabled)
 */
#ifndef HEAPINST_CFG_DEBUG_LOG
#define HEAPINST_CFG_DEBUG_LOG 1
#endif

#ifdef __cplusplus
}
#endif
