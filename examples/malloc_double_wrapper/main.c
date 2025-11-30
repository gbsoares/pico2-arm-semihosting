#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdio.h"
#include "pico/stdio_semihosting.h"

// Forward declarations for the Pico SDK's wrapper functions
// These are the actual wrapper functions in pico_malloc
extern void* __wrap_malloc(size_t size);
extern void __wrap_free(void* ptr);
extern void* __wrap_realloc(void* ptr, size_t size);

// Our custom malloc functions that call the Pico SDK wrappers and add logging
void* logged_pico_malloc(size_t size) {
    void* result = __wrap_malloc(size);
    printf("[MALLOC] Pico SDK allocated %zu bytes at %p\n", size, result);
    return result;
}

void logged_pico_free(void* ptr) {
    if (ptr != NULL) {
        printf("[FREE] Pico SDK freeing memory at %p\n", ptr);
    } else {
        printf("[FREE] Attempted to free NULL pointer (handled by Pico SDK)\n");
    }
    __wrap_free(ptr);
}

void* logged_pico_realloc(void* ptr, size_t size) {
    void* result = __wrap_realloc(ptr, size);
    if (ptr == NULL) {
        printf("[REALLOC] Pico SDK: NULL -> %zu bytes (like malloc), allocated at %p\n", size, result);
    } else if (size == 0) {
        printf("[REALLOC] Pico SDK: %p -> 0 bytes (like free)\n", ptr);
    } else {
        printf("[REALLOC] Pico SDK: %p -> %zu bytes, new address: %p\n", ptr, size, result);
    }
    return result;
}

int main() {
    stdio_semihosting_init();
    
    printf("=== Double-Wrapped Memory Allocation Test ===\n");
    printf("This program wraps the Pico SDK's malloc wrappers\n");
    printf("Benefits: Thread safety, heap initialization, platform optimizations\n\n");
    
    // Test malloc
    printf("Testing malloc...\n");
    void* ptr1 = logged_pico_malloc(100);
    
    // Test realloc
    printf("Testing realloc (expand)...\n");
    void* ptr2 = logged_pico_realloc(ptr1, 200);
    
    // Test another malloc
    printf("Testing another malloc...\n");
    void* ptr3 = logged_pico_malloc(50);
    
    // Test realloc shrink
    printf("Testing realloc (shrink)...\n");
    void* ptr4 = logged_pico_realloc(ptr2, 75);
    
    // Test free
    printf("Testing free operations...\n");
    logged_pico_free(ptr4);
    logged_pico_free(ptr3);
    
    // Test edge cases
    printf("Testing edge cases...\n");
    
    // malloc with zero size
    printf("Testing malloc(0)...\n");
    void* ptr5 = logged_pico_malloc(0);
    logged_pico_free(ptr5);
    
    // realloc with NULL pointer (should behave like malloc)
    printf("Testing realloc(NULL, size)...\n");
    void* ptr6 = logged_pico_realloc(NULL, 64);
    
    // realloc with zero size (should behave like free)
    printf("Testing realloc(ptr, 0)...\n");
    void* ptr7 = logged_pico_realloc(ptr6, 0);
    
    // free NULL pointer
    printf("Testing free(NULL)...\n");
    logged_pico_free(NULL);
    
    printf("\n=== All double-wrapped memory tests completed ===\n");
    printf("All allocations went through Pico SDK's optimized malloc implementation\n");
    
    return 0;
}