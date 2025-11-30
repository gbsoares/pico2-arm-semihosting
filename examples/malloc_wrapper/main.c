#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdio.h"
#include "pico/stdio_semihosting.h"

// Forward declarations for the original malloc functions from newlib
void* _malloc_r(struct _reent* r, size_t size);
void _free_r(struct _reent* r, void* ptr);
void* _realloc_r(struct _reent* r, void* ptr, size_t size);
extern struct _reent* _impure_ptr;

// Custom logging malloc functions
void* logged_malloc(size_t size)
{
    void* result = _malloc_r(_impure_ptr, size);
    printf("[MALLOC] Requested %zu bytes, allocated at %p\n", size, result);
    return result;
}

void logged_free(void* ptr)
{
    if (ptr != NULL) {
        printf("[FREE] Releasing memory at %p\n", ptr);
    } else {
        printf("[FREE] Attempted to free NULL pointer\n");
    }
    _free_r(_impure_ptr, ptr);
}

void* logged_realloc(void* ptr, size_t size)
{
    void* result = _realloc_r(_impure_ptr, ptr, size);
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

// Redefine malloc/free/realloc to use our logged versions
#define malloc(size) logged_malloc(size)
#define free(ptr) logged_free(ptr)
#define realloc(ptr, size) logged_realloc(ptr, size)

int main()
{
    stdio_semihosting_init();

    printf("=== Memory Allocation Wrapper Test ===\n");
    printf("This program demonstrates wrapping malloc/free/realloc calls\n\n");

    // Test malloc
    printf("Testing malloc...\n");
    void* ptr1 = malloc(100);

    // Test realloc
    printf("Testing realloc...\n");
    void* ptr2 = realloc(ptr1, 200);

    // Test another malloc
    printf("Testing another malloc...\n");
    void* ptr3 = malloc(50);

    // Test free
    printf("Testing free operations...\n");
    free(ptr2);
    free(ptr3);

    // Test malloc with zero size
    printf("Testing edge case: malloc(0)...\n");
    void* ptr4 = malloc(0);
    free(ptr4);

    // Test realloc with NULL pointer (should behave like malloc)
    printf("Testing edge case: realloc(NULL, size)...\n");
    void* ptr5 = realloc(NULL, 75);
    free(ptr5);

    printf("\n=== All memory allocation tests completed ===\n");

    return 0;
}