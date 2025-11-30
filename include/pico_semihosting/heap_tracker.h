#ifndef PICO_SEMIHOSTING_HEAP_TRACKER_H
#define PICO_SEMIHOSTING_HEAP_TRACKER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file heap_tracker.h
 * @brief Heap allocation tracking library for Pico semihosting
 * 
 * This library provides heap allocation tracking functionality that can log
 * memory operations to binary files via semihosting or console output.
 */

// Configurable buffer size - defaults to 4KB
#ifndef HEAP_TRACKER_BUFFER_SIZE
#define HEAP_TRACKER_BUFFER_SIZE 4096
#endif

/**
 * @brief Heap operation types for tracking
 */
typedef enum {
    HEAP_OP_INIT = 0,       ///< Heap tracker initialization
    HEAP_OP_MALLOC,         ///< malloc() operation
    HEAP_OP_FREE,           ///< free() operation  
    HEAP_OP_REALLOC         ///< realloc() operation
} heap_operation_type_t;

/**
 * @brief Heap operation record structure
 * 
 * Fixed size record for easier binary parsing. All fields are in host byte order.
 */
typedef struct __attribute__((packed)) {
    uint32_t operation;           ///< Operation type (heap_operation_type_t)
    uint64_t timestamp_us;        ///< Microsecond timestamp from time_us_64()
    uint32_t arg1;               ///< First argument (varies by operation)
    uint32_t arg2;               ///< Second argument (varies by operation)  
    uint32_t arg3;               ///< Third argument (only used by realloc)
    uint32_t padding;            ///< Padding for alignment
} heap_operation_record_t;

/**
 * @brief Initialize the heap tracker
 * 
 * This function is automatically called on first use of tracked functions,
 * but can be called manually for explicit initialization.
 */
void heap_tracker_init(void);

/**
 * @brief Flush any buffered heap tracking data
 * 
 * Forces any buffered tracking data to be written to the output file
 * or console. Should be called before program exit.
 */
void heap_tracker_flush(void);

/**
 * @brief Tracked malloc function
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void* heap_tracked_malloc(size_t size);

/**
 * @brief Tracked free function
 * 
 * @param ptr Pointer to memory to free (can be NULL)
 */
void heap_tracked_free(void* ptr);

/**
 * @brief Tracked realloc function
 * 
 * @param ptr Pointer to existing memory (can be NULL)
 * @param size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 */
void* heap_tracked_realloc(void* ptr, size_t size);

/**
 * @brief Check if heap tracker is initialized
 * 
 * @return true if initialized, false otherwise
 */
bool heap_tracker_is_initialized(void);

/**
 * @brief Get the current number of buffered operations
 * 
 * @return Number of operations currently in the buffer
 */
size_t heap_tracker_get_buffer_count(void);

/**
 * @brief Get the maximum buffer capacity
 * 
 * @return Maximum number of operations that can be buffered
 */
size_t heap_tracker_get_buffer_capacity(void);

#ifdef __cplusplus
}
#endif

#endif // PICO_SEMIHOSTING_HEAP_TRACKER_H