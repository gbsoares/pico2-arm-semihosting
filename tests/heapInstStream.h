/**
 * @file heapInstStream.h
 * @brief Test streamport interface for heap instrumentation unit tests.
 *
 * This is a mock streamport implementation that captures all written data
 * into a buffer for test verification.
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

/* Standard streamport interface */
int heapInstStreamPort_Init(void);
int heapInstStreamPort_Write(const void* data, size_t len);
int heapInstStreamPort_Flush(void);
int heapInstStreamPort_Close(void);

/* Test-specific accessors */
const uint8_t* test_get_stream_buffer(void);
size_t test_get_stream_buffer_size(void);
void test_reset_stream_buffer(void);

/* Test configuration */
void test_set_stream_fail_mode(int fail_after_bytes);

#ifdef __cplusplus
}
#endif

#endif /* HEAP_INST_STREAM_H */
