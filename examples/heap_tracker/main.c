#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include "pico/stdio.h"
#include "pico/stdio_semihosting.h"
#include "pico/time.h"
#include "pico_semihosting/heap_tracker.h"
#include "pico_semihosting/semihosting_utils.h"

// Redefine malloc/free/realloc to use our tracked versions
#define malloc(size) heap_tracked_malloc(size)
#define free(ptr) heap_tracked_free(ptr)
#define realloc(ptr, size) heap_tracked_realloc(ptr, size)

int main() {
    stdio_semihosting_init();
    
    printf("=== Heap Tracker Test ===\n");
    printf("Buffer size: %zu records\n", heap_tracker_get_buffer_capacity());
    
    // Test raw semihosting capabilities at startup
    printf("Testing raw semihosting file I/O capability...\n");
    
    int test_handle = semihosting_open("test_pattern.bin", OPEN_MODE_W);
    if (test_handle != -1) {
        // Write a known test pattern to verify endianness and structure
        uint8_t test_pattern[] = {
            0x01, 0x02, 0x03, 0x04,  // Should be 0x04030201 in little-endian uint32_t
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,  // 64-bit value
            0xAA, 0xBB, 0xCC, 0xDD,  // Another 32-bit value
            0xEE, 0xFF, 0x00, 0x11
        };
        
        int written = semihosting_write(test_handle, test_pattern, sizeof(test_pattern));
        printf("[TEST] Wrote %d bytes of test pattern\n", written);
        
        semihosting_close(test_handle);
        
        printf("✓ Raw semihosting file I/O is working\n");
        printf("This program will write binary heap trace data to heap_trace.bin\n\n");
    } else {
        printf("⚠ Raw semihosting file I/O not available - will use console output\n");
        printf("This program will output heap trace data to console in structured format\n\n");
    }
    
    printf("Starting heap operation tracking...\n\n");
    
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
    
    // Stress test to force buffer flush
    printf("\nStress testing to force buffer flush...\n");
    void* ptrs[20];
    for (int i = 0; i < 20; i++) {
        ptrs[i] = malloc(32 + i);
    }
    
    for (int i = 0; i < 20; i++) {
        free(ptrs[i]);
    }
    
    printf("\n=== Flushing remaining buffer contents ===\n");
    heap_tracker_flush();
    
    printf("\n=== All heap tracking tests completed ===\n");
    printf("Check trace file for binary trace data (location depends on semihosting setup)\n");
    printf("Possible locations: heap_trace.bin, /tmp/heap_trace.bin, ./build/heap_trace.bin\n");
    
    return 0;
}