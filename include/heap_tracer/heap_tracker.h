#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file heap_tracker.h
 * @brief Transport-agnostic heap instrumentation API.
 *
 * The instrumentation core records heap operations into a ring buffer and
 * forwards the binary records to a registered transport (semihosting, UDP,
 * serial, etc.). Platform-specific details (timestamps, locks, logging) are
 * injected via callback registration so the core remains portable.
 */

/**
 * @def HEAPTRC_CFG_BUFFER_SIZE
 * @brief Total bytes reserved for the heap trace buffer.
 *
 * This is the amount of memory consumed by the in-memory record buffer. It
 * should be sized to avoid excessive flushing but small enough for the target.
 */
#ifndef HEAPTRC_CFG_BUFFER_SIZE
#define HEAPTRC_CFG_BUFFER_SIZE 4096
#endif

/* Backwards compatibility alias used in existing sources. */
#ifndef HEAP_TRACKER_BUFFER_SIZE
#define HEAP_TRACKER_BUFFER_SIZE HEAPTRC_CFG_BUFFER_SIZE
#endif

/**
 * @brief Heap operation identifiers recorded in the trace stream.
 */
typedef enum {
    HEAP_OP_INIT = 0,
    HEAP_OP_MALLOC,
    HEAP_OP_FREE,
    HEAP_OP_REALLOC,
} heap_operation_t;

/**
 * @brief Encoded heap operation record written to the trace stream.
 *
 * Fields arg1..arg3 are interpreted based on @ref heap_operation_t.
 */
typedef struct heap_operation_record {
    uint8_t operation;     /* heap_operation_t */
    uint8_t padding;       /* reserved for alignment/future flags */
    uint16_t reserved;     /* reserved */
    uint64_t timestamp_us; /* platform-provided timestamp */
    uint32_t arg1;         /* op-specific argument */
    uint32_t arg2;         /* op-specific argument */
    uint32_t arg3;         /* op-specific argument */
} heap_operation_record_t;

/**
 * @brief Transport callbacks used by the instrumentation core.
 *
 * A transport is responsible for moving raw record bytes to a host or sink.
 */
typedef int (*heaptrc_write_fn)(const void* data, size_t len, void* ctx);
typedef int (*heaptrc_flush_fn)(void* ctx);
typedef int (*heaptrc_close_fn)(void* ctx);

typedef struct heaptrc_transport {
    heaptrc_write_fn write; /* required: returns bytes written or <0 on error */
    heaptrc_flush_fn flush; /* optional: force out buffered data */
    heaptrc_close_fn close; /* optional: release transport resources */
    void* ctx;              /* opaque pointer passed to callbacks */
} heaptrc_transport_t;

/**
 * @brief Platform hooks injected by the port layer.
 */
typedef uint64_t (*heaptrc_timestamp_fn)(void* ctx);
typedef void (*heaptrc_log_fn)(const char* msg, void* ctx);
typedef void (*heaptrc_lock_fn)(void* ctx);
typedef void (*heaptrc_unlock_fn)(void* ctx);

typedef struct heaptrc_platform_hooks {
    heaptrc_timestamp_fn timestamp_us; /* required for meaningful records */
    heaptrc_log_fn log;                /* optional debug logging sink */
    heaptrc_lock_fn lock;              /* optional buffer protection */
    heaptrc_unlock_fn unlock;          /* optional buffer protection */
    void* timestamp_ctx;
    void* log_ctx;
    void* lock_ctx;
    void* unlock_ctx;
} heaptrc_platform_hooks_t;

/* Transport and platform registration */
void heap_tracker_register_transport(const heaptrc_transport_t* transport);
void heap_tracker_register_platform_hooks(
    const heaptrc_platform_hooks_t* hooks);

/* Instrumentation lifecycle */
void heap_tracker_init(void);
void heap_tracker_flush(void);
bool heap_tracker_is_initialized(void);

/* Buffer status helpers */
size_t heap_tracker_get_buffer_count(void);
size_t heap_tracker_get_buffer_capacity(void);

/* Tracked allocation wrappers */
void* heap_tracked_malloc(size_t size);
void heap_tracked_free(void* ptr);
void* heap_tracked_realloc(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif

