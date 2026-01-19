#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file heapInst.h
 * @brief Transport-agnostic heap instrumentation API.
 *
 * The instrumentation core records heap operations into a ring buffer and
 * forwards the binary records to a registered transport (semihosting, UDP,
 * serial, etc.). Platform-specific details (timestamps, locks, logging) are
 * injected via callback registration so the core remains portable.
 */

/**
 * @def HEAPINST_CFG_BUFFER_SIZE
 * @brief Total bytes reserved for the heap trace buffer.
 *
 * This is the amount of memory consumed by the in-memory record buffer. It
 * should be sized to avoid excessive flushing but small enough for the target.
 */
#ifndef HEAPINST_CFG_BUFFER_SIZE
#define HEAPINST_CFG_BUFFER_SIZE 4096
#endif

#ifndef HEAP_INST_BUFFER_SIZE
#define HEAP_INST_BUFFER_SIZE HEAPINST_CFG_BUFFER_SIZE
#endif

/**
 * @brief Heap operation identifiers recorded in the trace stream.
 */
typedef enum {
    HEAP_OP_INIT = 0,
    HEAP_OP_MALLOC,
    HEAP_OP_FREE,
    HEAP_OP_REALLOC,
} heap_inst_operation_t;

/**
 * @brief Encoded heap operation record written to the trace stream.
 *
 * Fixed 24-byte record format. Fields arg1..arg3 are interpreted based on
 * the operation type:
 *
 * HEAP_OP_INIT:
 *   - arg1: heap_base   - Start address of the heap region
 *   - arg2: heap_size   - Total size of the heap region in bytes
 *   - arg3: flags       - Bit flags (see HEAP_INIT_FLAG_*)
 *
 * HEAP_OP_MALLOC:
 *   - arg1: size        - Requested allocation size in bytes
 *   - arg2: ptr         - Returned pointer (or 0 if allocation failed)
 *   - arg3: (unused)
 *
 * HEAP_OP_FREE:
 *   - arg1: ptr         - Pointer being freed
 *   - arg2: (unused)
 *   - arg3: (unused)
 *
 * HEAP_OP_REALLOC:
 *   - arg1: old_ptr     - Original pointer (or 0 for malloc-like behavior)
 *   - arg2: new_size    - Requested new size
 *   - arg3: new_ptr     - Returned pointer (or 0 if reallocation failed)
 */
typedef struct heap_inst_record {
    uint8_t operation;     /* heap_inst_operation_t */
    uint8_t padding;       /* reserved for alignment/future flags */
    uint16_t reserved;     /* reserved */
    uint64_t timestamp_us; /* platform-provided timestamp */
    uint32_t arg1;         /* op-specific argument */
    uint32_t arg2;         /* op-specific argument */
    uint32_t arg3;         /* op-specific argument */
} heap_inst_record_t;

/**
 * @brief Flags for HEAP_OP_INIT record arg3 field.
 */
#define HEAP_INIT_FLAG_HEAP_INFO_VALID  (1 << 0)  /* heap_base and heap_size are valid */

/**
 * @brief Platform hooks injected by the port layer.
 */
typedef uint64_t (*heap_inst_timestamp_fn)(void* ctx);
typedef void (*heap_inst_log_fn)(const char* msg, void* ctx);
typedef void (*heap_inst_lock_fn)(void* ctx);
typedef void (*heap_inst_unlock_fn)(void* ctx);

typedef struct heap_inst_platform_hooks {
    heap_inst_timestamp_fn timestamp_us; /* required for meaningful records */
    heap_inst_log_fn log;                /* optional debug logging sink */
    heap_inst_lock_fn lock;              /* optional buffer protection */
    heap_inst_unlock_fn unlock;          /* optional buffer protection */
    void* timestamp_ctx;
    void* log_ctx;
    void* lock_ctx;
    void* unlock_ctx;
} heap_inst_platform_hooks_t;

/**
 * @brief Heap region configuration for initialization.
 *
 * Provides information about the heap memory region to enable accurate
 * visualization in analysis tools. On Pico platforms, this is typically
 * auto-detected from linker symbols (__end__ to __StackLimit).
 */
typedef struct heap_inst_heap_info {
    void* heap_start;   /* Start address of heap region (NULL if unknown) */
    size_t heap_size;   /* Total size of heap region in bytes (0 if unknown) */
} heap_inst_heap_info_t;

/* Platform hooks registration */
void heap_inst_register_platform_hooks(const heap_inst_platform_hooks_t* hooks);

/* Instrumentation lifecycle */

/**
 * @brief Initialize heap instrumentation.
 *
 * @param heap_info  Optional heap region configuration. Pass NULL to attempt
 *                   auto-detection from linker symbols (works on Pico platforms).
 *
 * On Pico platforms with NULL heap_info, heap bounds are auto-detected from
 * linker symbols __end__ (heap start) and __StackLimit (heap end).
 *
 * On host platforms or when auto-detection fails, the INIT record will have
 * heap_base=0, heap_size=0, and HEAP_INIT_FLAG_HEAP_INFO_VALID will not be set.
 * The visualization tool will then infer bounds from observed allocations.
 */
void heap_inst_init(const heap_inst_heap_info_t* heap_info);

void heap_inst_flush(void);
bool heap_inst_is_initialized(void);

/* Buffer status helpers */
size_t heap_inst_get_buffer_count(void);
size_t heap_inst_get_buffer_capacity(void);

/*
 * Internal recording functions (used by linker wrappers in heapInst_wrap.c).
 * These record operations without performing the actual allocation, allowing
 * the __wrap_* functions to handle allocation separately.
 *
 * Applications should NOT call these directly - they are called automatically
 * by the linker-wrapped malloc/free/realloc/calloc functions.
 */
void heap_inst_record_malloc(size_t size, void* result);
void heap_inst_record_free(void* ptr);
void heap_inst_record_realloc(void* old_ptr, size_t new_size, void* result);

#ifdef __cplusplus
}
#endif
