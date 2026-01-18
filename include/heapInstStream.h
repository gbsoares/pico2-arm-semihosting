/**
 * @file heapInstStream.h
 * @brief Stream port interface for heap instrumentation.
 *
 * This header defines the common interface that all stream ports must implement.
 * The heap instrumentation core uses these functions to output trace data.
 * Each transport (semihosting, UDP, etc.) provides its own implementation.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef HEAP_INST_STREAM_H
#define HEAP_INST_STREAM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the stream port.
 *
 * Called during heap_inst_init() to set up the transport.
 * For semihosting, this opens the trace file on the host.
 *
 * @return 0 on success, negative error code on failure.
 */
int heapInstStreamPort_Init(void);

/**
 * @brief Write data to the stream.
 *
 * Sends trace data to the host via the transport.
 *
 * @param data Pointer to data buffer to write.
 * @param len  Number of bytes to write.
 * @return Number of bytes written on success, negative error code on failure.
 */
int heapInstStreamPort_Write(const void* data, size_t len);

/**
 * @brief Flush any buffered data.
 *
 * Forces any internally buffered data to be sent.
 * Some transports may not buffer and can return 0 immediately.
 *
 * @return 0 on success, negative error code on failure.
 */
int heapInstStreamPort_Flush(void);

/**
 * @brief Close the stream and release resources.
 *
 * Called during shutdown to cleanly close the transport.
 * For semihosting, this closes the trace file on the host.
 *
 * @return 0 on success, negative error code on failure.
 */
int heapInstStreamPort_Close(void);

#ifdef __cplusplus
}
#endif

#endif /* HEAP_INST_STREAM_H */
