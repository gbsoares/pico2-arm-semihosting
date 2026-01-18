/**
 * @file heapInst_wrap.c
 * @brief Link-time malloc/free/realloc/calloc wrappers for automatic instrumentation.
 *
 * This file provides __wrap_* functions that intercept allocation calls via the
 * linker's --wrap feature. When linked, all malloc/free/realloc/calloc calls in
 * the application are automatically redirected through these wrappers, which
 * record the operations before calling the real implementations.
 *
 * IMPORTANT: The Pico SDK also defines __wrap_malloc (in pico_malloc). To ensure
 * our wrappers take precedence, this file is compiled as an OBJECT library and
 * linked BEFORE pico_stdlib (which pulls in pico_malloc).
 *
 * Call chain:
 *   User code: malloc(100)
 *     -> Linker redirects to: __wrap_malloc() [OUR wrapper - this file]
 *       -> Records operation
 *       -> Calls __real_malloc() [actual newlib malloc]
 *
 * Note: This bypasses Pico SDK's malloc wrapper (mutex, panic checks). If you
 * need thread-safety, use the heap_inst_register_platform_hooks() to register
 * lock/unlock callbacks that provide equivalent protection.
 *
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include "heapInst/heapInst.h"

#include <stdint.h>
#include <string.h>

/*
 * The linker's --wrap=malloc option:
 * - Redirects all calls to malloc() -> __wrap_malloc()
 * - Provides __real_malloc() to call the actual implementation
 *
 * This allows transparent interception without modifying user code.
 */

/* Real allocation functions provided by the linker */
extern void *__real_malloc(size_t size);
extern void *__real_calloc(size_t nmemb, size_t size);
extern void *__real_realloc(void *ptr, size_t size);
extern void __real_free(void *ptr);

/**
 * @brief Wrapped malloc - intercepts all malloc calls.
 *
 * Records the allocation in the trace buffer, then calls the real malloc.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 */
void *__wrap_malloc(size_t size)
{
    void *result = __real_malloc(size);
    heap_inst_record_malloc(size, result);
    return result;
}

/**
 * @brief Wrapped calloc - intercepts all calloc calls.
 *
 * Records the allocation as a malloc (with total size) in the trace buffer,
 * then calls the real calloc.
 *
 * @param nmemb Number of elements.
 * @param size Size of each element.
 * @return Pointer to zero-initialized allocated memory, or NULL on failure.
 */
void *__wrap_calloc(size_t nmemb, size_t size)
{
    void *result = __real_calloc(nmemb, size);
    /* Record as malloc with total size for simplicity */
    heap_inst_record_malloc(nmemb * size, result);
    return result;
}

/**
 * @brief Wrapped realloc - intercepts all realloc calls.
 *
 * Records the reallocation in the trace buffer, then calls the real realloc.
 *
 * @param ptr Pointer to previously allocated memory (or NULL).
 * @param size New size in bytes.
 * @return Pointer to reallocated memory, or NULL on failure.
 */
void *__wrap_realloc(void *ptr, size_t size)
{
    void *result = __real_realloc(ptr, size);
    heap_inst_record_realloc(ptr, size, result);
    return result;
}

/**
 * @brief Wrapped free - intercepts all free calls.
 *
 * Records the deallocation in the trace buffer, then calls the real free.
 *
 * @param ptr Pointer to memory to free (may be NULL).
 */
void __wrap_free(void *ptr)
{
    heap_inst_record_free(ptr);
    __real_free(ptr);
}
