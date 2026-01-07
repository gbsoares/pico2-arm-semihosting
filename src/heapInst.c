#include "heapInst/heapInst.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __has_include(<sys/reent.h>)
#include <sys/reent.h>
#else
/* Minimal newlib-compatible stub for host builds that lack sys/reent.h */
struct _reent {
    int _errno;
};
extern struct _reent* _impure_ptr;
#endif

// Forward declarations for the original malloc functions from newlib
void* _malloc_r(struct _reent* r, size_t size);
void _free_r(struct _reent* r, void* ptr);
void* _realloc_r(struct _reent* r, void* ptr, size_t size);
extern struct _reent* _impure_ptr;

// Static buffer for tracking heap operations
static heap_inst_record_t
    heap_buffer[HEAP_INST_BUFFER_SIZE / sizeof(heap_inst_record_t)];
static size_t buffer_index = 0;
static bool tracker_initialized = false;

static heap_inst_transport_t g_transport = {0};
static heap_inst_platform_hooks_t g_platform_hooks = {0};

static void heapInst_lock(void)
{
    if (g_platform_hooks.lock) {
        g_platform_hooks.lock(g_platform_hooks.lock_ctx);
    }
}

static void heapInst_unlock(void)
{
    if (g_platform_hooks.unlock) {
        g_platform_hooks.unlock(g_platform_hooks.unlock_ctx);
    }
}

static void heaptrc_logf(const char* fmt, ...)
{
    char msg[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (g_platform_hooks.log) {
        g_platform_hooks.log(msg, g_platform_hooks.log_ctx);
    } else {
        fputs(msg, stdout);
    }
}

static uint64_t heaptrc_timestamp_us(void)
{
    if (g_platform_hooks.timestamp_us) {
        return g_platform_hooks.timestamp_us(g_platform_hooks.timestamp_ctx);
    }
    return 0;
}

static int heaptrc_transport_write(const void* data, size_t len)
{
    if (!g_transport.write) {
        return -1;
    }
    return g_transport.write(data, len, g_transport.ctx);
}

// Function to flush buffer to the registered transport (or console fallback)
static void flush_buffer_to_transport(void)
{
    if (buffer_index == 0) return;

    size_t expected_bytes = buffer_index * sizeof(heap_inst_record_t);
    bool wrote_all = false;

    if (g_transport.write) {
        int bytes_written =
            heaptrc_transport_write(heap_buffer, expected_bytes);
        wrote_all = (bytes_written >= 0) &&
                    ((size_t)bytes_written == expected_bytes);

        if (!wrote_all) {
            heaptrc_logf(
                "[HEAP_TRACKER] Transport write short (%d/%zu bytes)\n",
                bytes_written, expected_bytes);
        }

        if (g_transport.flush) {
            g_transport.flush(g_transport.ctx);
        }
    }

    if (!wrote_all) {
        if (!g_transport.write) {
            heaptrc_logf(
                "[HEAP_TRACKER] No transport registered; emitting text "
                "trace\n");
        } else {
            heaptrc_logf("[HEAP_TRACKER] Falling back to text trace\n");
        }
        heaptrc_logf("--- HEAP_TRACE_START ---\n");
        for (size_t i = 0; i < buffer_index; i++) {
            const heap_inst_record_t* rec = &heap_buffer[i];
            heaptrc_logf("RECORD:%zu,OP:%u,TIME:%llu", i,
                         (unsigned int)rec->operation,
                         (unsigned long long)rec->timestamp_us);

            switch (rec->operation) {
                case HEAP_OP_INIT:
                    heaptrc_logf(",HEAP_SIZE:%" PRIu32, rec->arg1);
                    break;
                case HEAP_OP_MALLOC:
                    heaptrc_logf(",SIZE:%" PRIu32 ",PTR:0x%" PRIx32, rec->arg1,
                                 rec->arg2);
                    break;
                case HEAP_OP_FREE:
                    heaptrc_logf(",PTR:0x%" PRIx32, rec->arg1);
                    break;
                case HEAP_OP_REALLOC:
                    heaptrc_logf(",OLD_PTR:0x%" PRIx32 ",SIZE:%" PRIu32
                                 ",NEW_PTR:0x%" PRIx32,
                                 rec->arg1, rec->arg2, rec->arg3);
                    break;
                default:
                    break;
            }
            heaptrc_logf("\n");
        }
        heaptrc_logf("--- HEAP_TRACE_END ---\n");
    }

    buffer_index = 0;
}

// Function to add operation to buffer
static void log_heap_operation(const heap_inst_record_t* record)
{
    heapInst_lock();

    // Check if buffer is full
    if (buffer_index >= (sizeof(heap_buffer) / sizeof(heap_buffer[0]))) {
        flush_buffer_to_transport();
    }

    // Add record to buffer
    heap_buffer[buffer_index++] = *record;

    heapInst_unlock();
}

void heap_inst_init(void)
{
    if (tracker_initialized) return;

    tracker_initialized = true;
    buffer_index = 0;

    uint64_t current_time = heaptrc_timestamp_us();
    heaptrc_logf("[HEAP_TRACKER] Current timestamp_us(): %llu\n",
                 (unsigned long long)current_time);

    heap_inst_record_t init_record = {
        .operation = HEAP_OP_INIT,
        .timestamp_us = current_time,
        .arg1 = 0,  // initial_heap_size (not available on Pico)
        .arg2 = 0,  // unused
        .arg3 = 0,  // unused
        .padding = 0};

    log_heap_operation(&init_record);
    heaptrc_logf("[HEAP_TRACKER] Initialized - buffer size: %zu records\n",
                 sizeof(heap_buffer) / sizeof(heap_buffer[0]));
    heaptrc_logf("[HEAP_TRACKER] Record size: %zu bytes\n",
                 sizeof(heap_inst_record_t));
}

void heap_inst_flush(void)
{
    if (tracker_initialized && buffer_index > 0) {
        flush_buffer_to_transport();
    }
    if (g_transport.close) {
        g_transport.close(g_transport.ctx);
    }
}

void* heap_inst_malloc(size_t size)
{
    if (!tracker_initialized) {
        heap_inst_init();
    }

    void* result = _malloc_r(_impure_ptr, size);

    heap_inst_record_t record = {
        .operation = HEAP_OP_MALLOC,
        .timestamp_us = heaptrc_timestamp_us(),
        .arg1 = (uint32_t)size,               // size
        .arg2 = (uint32_t)(uintptr_t)result,  // result_ptr
        .arg3 = 0,                            // unused
        .padding = 0};

    log_heap_operation(&record);
    heaptrc_logf("[MALLOC] Requested %zu bytes, allocated at %p\n", size,
                 result);
    return result;
}

void heap_inst_free(void* ptr)
{
    if (!tracker_initialized) {
        heap_inst_init();
    }

    heap_inst_record_t record = {.operation = HEAP_OP_FREE,
                                 .timestamp_us = heaptrc_timestamp_us(),
                                 .arg1 = (uint32_t)(uintptr_t)ptr,  // ptr
                                 .arg2 = 0,                         // unused
                                 .arg3 = 0,                         // unused
                                 .padding = 0};

    log_heap_operation(&record);

    if (ptr != NULL) {
        heaptrc_logf("[FREE] Releasing memory at %p\n", ptr);
    } else {
        heaptrc_logf("[FREE] Attempted to free NULL pointer\n");
    }
    _free_r(_impure_ptr, ptr);
}

void* heap_inst_realloc(void* ptr, size_t size)
{
    if (!tracker_initialized) {
        heap_inst_init();
    }

    void* result = _realloc_r(_impure_ptr, ptr, size);

    heap_inst_record_t record = {
        .operation = HEAP_OP_REALLOC,
        .timestamp_us = heaptrc_timestamp_us(),
        .arg1 = (uint32_t)(uintptr_t)ptr,     // old_ptr
        .arg2 = (uint32_t)size,               // new_size
        .arg3 = (uint32_t)(uintptr_t)result,  // result_ptr
        .padding = 0};

    log_heap_operation(&record);

    if (ptr == NULL) {
        heaptrc_logf(
            "[REALLOC] NULL -> %zu bytes (like malloc), allocated at %p\n",
            size, result);
    } else if (size == 0) {
        heaptrc_logf("[REALLOC] %p -> 0 bytes (like free)\n", ptr);
    } else {
        heaptrc_logf("[REALLOC] %p -> %zu bytes, new address: %p\n", ptr, size,
                     result);
    }
    return result;
}

bool heap_inst_is_initialized(void) { return tracker_initialized; }

size_t heap_inst_get_buffer_count(void) { return buffer_index; }

size_t heap_inst_get_buffer_capacity(void)
{
    return sizeof(heap_buffer) / sizeof(heap_buffer[0]);
}

void heap_inst_register_transport(const heap_inst_transport_t* transport)
{
    if (transport) {
        g_transport = *transport;
    } else {
        memset(&g_transport, 0, sizeof(g_transport));
    }
}

void heap_inst_register_platform_hooks(
    const heap_inst_platform_hooks_t* hooks)
{
    if (hooks) {
        g_platform_hooks = *hooks;
    } else {
        memset(&g_platform_hooks, 0, sizeof(g_platform_hooks));
    }
}

#ifdef HEAPINST_TEST_API
void heap_inst_test_reset(void)
{
    tracker_initialized = false;
    buffer_index = 0;
    memset(heap_buffer, 0, sizeof(heap_buffer));
    memset(&g_transport, 0, sizeof(g_transport));
    memset(&g_platform_hooks, 0, sizeof(g_platform_hooks));
}
#endif
