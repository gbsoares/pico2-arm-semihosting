#include "pico_semihosting/heap_tracker.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/time.h"
#include "semihosting/semihosting.h"

// Forward declarations for the original malloc functions from newlib
void* _malloc_r(struct _reent* r, size_t size);
void _free_r(struct _reent* r, void* ptr);
void* _realloc_r(struct _reent* r, void* ptr, size_t size);
extern struct _reent* _impure_ptr;

// Static buffer for tracking heap operations
static heap_operation_record_t
    heap_buffer[HEAP_TRACKER_BUFFER_SIZE / sizeof(heap_operation_record_t)];
static size_t buffer_index = 0;
static bool tracker_initialized = false;
static int log_file_handle = -1;

// Function to flush buffer to file via semihosting
static void flush_buffer_to_file(void)
{
    if (buffer_index == 0) return;

    // Open file using semihosting (create/append mode)
    if (log_file_handle == -1) {
        printf("[HEAP_TRACKER] Opening heap_trace.bin using semihosting...\n");

        log_file_handle = semihosting_open("/tmp/heap_trace.bin", OPEN_MODE_AB);

        if (log_file_handle == -1) {
            printf(
                "[HEAP_TRACKER] Semihosting file open failed, trying write "
                "mode...\n");
            log_file_handle =
                semihosting_open("/tmp/heap_trace.bin", OPEN_MODE_WB);
        }

        if (log_file_handle == -1) {
            printf(
                "[HEAP_TRACKER] Semihosting failed. Falling back to console "
                "output:\n");
            printf("--- HEAP_TRACE_START ---\n");

            // Output buffer as structured text that can be parsed
            for (size_t i = 0; i < buffer_index; i++) {
                const heap_operation_record_t* rec = &heap_buffer[i];
                printf("RECORD:%zu,OP:%u,TIME:%llu", i,
                       (unsigned int)rec->operation,
                       (unsigned long long)rec->timestamp_us);

                switch (rec->operation) {
                    case HEAP_OP_INIT:
                        printf(",HEAP_SIZE:%" PRIu32, rec->arg1);
                        break;
                    case HEAP_OP_MALLOC:
                        printf(",SIZE:%" PRIu32 ",PTR:0x%" PRIx32, rec->arg1,
                               rec->arg2);
                        break;
                    case HEAP_OP_FREE:
                        printf(",PTR:0x%" PRIx32, rec->arg1);
                        break;
                    case HEAP_OP_REALLOC:
                        printf(",OLD_PTR:0x%" PRIx32 ",SIZE:%" PRIu32
                               ",NEW_PTR:0x%" PRIx32,
                               rec->arg1, rec->arg2, rec->arg3);
                        break;
                }
                printf("\n");
            }
            printf("--- HEAP_TRACE_END ---\n");

            buffer_index = 0;  // Clear buffer since we can't write it
            return;
        } else {
            printf(
                "[HEAP_TRACKER] Semihosting file opened successfully (handle: "
                "%d)\n",
                log_file_handle);
        }
    }

    // Write buffer contents using semihosting
    int bytes_written =
        semihosting_write(log_file_handle, heap_buffer,
                          buffer_index * sizeof(heap_operation_record_t));
    size_t expected_bytes = buffer_index * sizeof(heap_operation_record_t);

    printf(
        "[HEAP_TRACKER] Write details: %zu records, %zu bytes each, %zu total "
        "bytes\n",
        buffer_index, sizeof(heap_operation_record_t), expected_bytes);
    printf("[HEAP_TRACKER] Semihosting write returned: %d bytes\n",
           bytes_written);

    // Debug first few bytes of what we're writing
    uint8_t* data_ptr = (uint8_t*)heap_buffer;
    printf("[HEAP_TRACKER] First 16 bytes: ");
    for (int i = 0; i < 16 && i < (int)expected_bytes; i++) {
        printf("%02x ", data_ptr[i]);
    }
    printf("\n");

    if (bytes_written != (int)expected_bytes) {
        printf("WARNING: Only wrote %d of %zu bytes to file\n", bytes_written,
               expected_bytes);
    } else {
        printf(
            "[HEAP_TRACKER] Successfully flushed %zu records (%zu bytes) to "
            "trace file\n",
            buffer_index, expected_bytes);
    }

    // Reset buffer
    buffer_index = 0;
}

// Function to add operation to buffer
static void log_heap_operation(const heap_operation_record_t* record)
{
    // Check if buffer is full
    if (buffer_index >= (sizeof(heap_buffer) / sizeof(heap_buffer[0]))) {
        flush_buffer_to_file();
    }

    // Add record to buffer
    heap_buffer[buffer_index++] = *record;
}

void heap_tracker_init(void)
{
    if (tracker_initialized) return;

    tracker_initialized = true;
    buffer_index = 0;

    uint64_t current_time = time_us_64();
    printf("[HEAP_TRACKER] Current time_us_64(): %llu\n",
           (unsigned long long)current_time);

    heap_operation_record_t init_record = {
        .operation = HEAP_OP_INIT,
        .timestamp_us = current_time,
        .arg1 = 0,  // initial_heap_size (not available on Pico)
        .arg2 = 0,  // unused
        .arg3 = 0,  // unused
        .padding = 0};

    log_heap_operation(&init_record);
    printf("[HEAP_TRACKER] Initialized - buffer size: %zu records\n",
           sizeof(heap_buffer) / sizeof(heap_buffer[0]));
    printf("[HEAP_TRACKER] Record size: %zu bytes\n",
           sizeof(heap_operation_record_t));

    // Debug the first record
    printf("[HEAP_TRACKER] First record - op:%u, time:%llu, args:[%u,%u,%u]\n",
           (unsigned int)init_record.operation,
           (unsigned long long)init_record.timestamp_us,
           (unsigned int)init_record.arg1, (unsigned int)init_record.arg2,
           (unsigned int)init_record.arg3);
}

void heap_tracker_flush(void)
{
    if (tracker_initialized && buffer_index > 0) {
        flush_buffer_to_file();
    }
    if (log_file_handle != -1) {
        semihosting_close(log_file_handle);
        log_file_handle = -1;
        printf("[HEAP_TRACKER] Closed trace file\n");
    }
}

void* heap_tracked_malloc(size_t size)
{
    if (!tracker_initialized) {
        heap_tracker_init();
    }

    void* result = _malloc_r(_impure_ptr, size);

    heap_operation_record_t record = {
        .operation = HEAP_OP_MALLOC,
        .timestamp_us = time_us_64(),
        .arg1 = (uint32_t)size,               // size
        .arg2 = (uint32_t)(uintptr_t)result,  // result_ptr
        .arg3 = 0,                            // unused
        .padding = 0};

    log_heap_operation(&record);
    printf("[MALLOC] Requested %zu bytes, allocated at %p\n", size, result);
    return result;
}

void heap_tracked_free(void* ptr)
{
    if (!tracker_initialized) {
        heap_tracker_init();
    }

    heap_operation_record_t record = {.operation = HEAP_OP_FREE,
                                      .timestamp_us = time_us_64(),
                                      .arg1 = (uint32_t)(uintptr_t)ptr,  // ptr
                                      .arg2 = 0,                         // unused
                                      .arg3 = 0,                         // unused
                                      .padding = 0};

    log_heap_operation(&record);

    if (ptr != NULL) {
        printf("[FREE] Releasing memory at %p\n", ptr);
    } else {
        printf("[FREE] Attempted to free NULL pointer\n");
    }
    _free_r(_impure_ptr, ptr);
}

void* heap_tracked_realloc(void* ptr, size_t size)
{
    if (!tracker_initialized) {
        heap_tracker_init();
    }

    void* result = _realloc_r(_impure_ptr, ptr, size);

    heap_operation_record_t record = {
        .operation = HEAP_OP_REALLOC,
        .timestamp_us = time_us_64(),
        .arg1 = (uint32_t)(uintptr_t)ptr,     // old_ptr
        .arg2 = (uint32_t)size,               // new_size
        .arg3 = (uint32_t)(uintptr_t)result,  // result_ptr
        .padding = 0};

    log_heap_operation(&record);

    if (ptr == NULL) {
        printf("[REALLOC] NULL -> %zu bytes (like malloc), allocated at %p\n",
               size, result);
    } else if (size == 0) {
        printf("[REALLOC] %p -> 0 bytes (like free)\n", ptr);
    } else {
        printf("[REALLOC] %p -> %zu bytes, new address: %p\n", ptr, size,
               result);
    }
    return result;
}

bool heap_tracker_is_initialized(void) { return tracker_initialized; }

size_t heap_tracker_get_buffer_count(void) { return buffer_index; }

size_t heap_tracker_get_buffer_capacity(void)
{
    return sizeof(heap_buffer) / sizeof(heap_buffer[0]);
}