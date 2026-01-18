/**
 * @file heapInstStream.c
 * @brief Test streamport implementation for heap instrumentation unit tests.
 *
 * This mock streamport captures all written data into a buffer for test
 * verification. It also supports a "fail mode" to simulate transport failures.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "heapInstStream.h"
#include <string.h>

#define TEST_STREAM_BUFFER_SIZE 4096

static uint8_t g_test_buffer[TEST_STREAM_BUFFER_SIZE];
static size_t g_test_buffer_pos = 0;
static int g_fail_after_bytes = -1;  /* -1 = never fail */

int heapInstStreamPort_Init(void)
{
    g_test_buffer_pos = 0;
    /* Don't reset fail_after_bytes here - let tests control it */
    return 0;
}

int heapInstStreamPort_Write(const void* data, size_t len)
{
    /* Check if we should simulate a failure */
    if (g_fail_after_bytes >= 0) {
        if ((int)g_test_buffer_pos >= g_fail_after_bytes) {
            return -1;
        }
        /* Partial write up to failure point */
        size_t allowed = (size_t)(g_fail_after_bytes - (int)g_test_buffer_pos);
        if (len > allowed) {
            len = allowed;
        }
    }

    /* Check buffer overflow */
    if (g_test_buffer_pos + len > TEST_STREAM_BUFFER_SIZE) {
        return -1;
    }

    memcpy(g_test_buffer + g_test_buffer_pos, data, len);
    g_test_buffer_pos += len;
    return (int)len;
}

int heapInstStreamPort_Flush(void)
{
    return 0;
}

int heapInstStreamPort_Close(void)
{
    return 0;
}

const uint8_t* test_get_stream_buffer(void)
{
    return g_test_buffer;
}

size_t test_get_stream_buffer_size(void)
{
    return g_test_buffer_pos;
}

void test_reset_stream_buffer(void)
{
    g_test_buffer_pos = 0;
    g_fail_after_bytes = -1;
    memset(g_test_buffer, 0, TEST_STREAM_BUFFER_SIZE);
}

void test_set_stream_fail_mode(int fail_after_bytes)
{
    g_fail_after_bytes = fail_after_bytes;
}
