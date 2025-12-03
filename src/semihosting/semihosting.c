#include "semihosting/semihosting.h"
#include "semihosting_ops.h"
#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/exception.h"

/* Global state for semihosting fault detection */
static volatile bool semihosting_fault_occurred = false;
static volatile bool semihosting_available = false;
static volatile bool semihosting_initialized = false;

/* Store original HardFault handler */
static exception_handler_t original_hardfault_handler = NULL;

/**
 * @brief Custom HardFault handler that detects semihosting faults
 *
 * This handler checks if a HardFault occurred due to a semihosting call
 * (BKPT instruction when no debugger is attached). If so, it handles the
 * fault gracefully instead of hanging the system.
 */
void semihosting_hardfault_handler(void)
{
    // Get the stacked PC (Program Counter) from the fault
    uint32_t* fault_stack;
    uint32_t control_reg;

    // Get CONTROL register using inline assembly
    __asm volatile("mrs %0, CONTROL" : "=r"(control_reg));

    // Determine which stack was being used
    if (control_reg & 2) {
        // Get PSP (Process Stack Pointer) using inline assembly
        __asm volatile("mrs %0, PSP" : "=r"(fault_stack));
    } else {
        // Get MSP (Main Stack Pointer) using inline assembly
        __asm volatile("mrs %0, MSP" : "=r"(fault_stack));
    }

    // The stacked PC is at offset 6 in the exception stack frame
    uint32_t fault_pc = fault_stack[6];

    // Check if the faulting instruction was a BKPT (0xBEAB in little-endian)
    // This is the semihosting breakpoint instruction
    uint16_t* instruction = (uint16_t*)(fault_pc - 2);
    if (*instruction == 0xBEAB) {
        // This is a semihosting fault - set flag and skip the instruction
        semihosting_fault_occurred = true;
        semihosting_available = false;

        // Skip the BKPT instruction by advancing PC by 2 bytes
        fault_stack[6] = fault_pc;

        // Set return value to indicate error (R0 = -1)
        fault_stack[0] = (uint32_t)-1;

        return;  // Return from fault handler
    }

    // If it's not a semihosting fault, call the original handler
    if (original_hardfault_handler) {
        original_hardfault_handler();
    } else {
        // Default behavior - infinite loop or reset
        while (1) {
            tight_loop_contents();
        }
    }
}

bool semihosting_is_available(void)
{
    if (!semihosting_initialized) {
        semihosting_init();
    }
    return semihosting_available;
}

/**
 * @brief Performs a semihosting call to the host debugger.
 *
 * This function executes a semihosting operation by invoking the appropriate
 * instruction (BKPT on ARM Cortex-M) with the semihosting call number and
 * argument pointer.
 *
 * @param reason The semihosting operation number (e.g., SYS_OPEN, SYS_WRITE,
 * etc.)
 * @param arg Pointer to the argument block for the semihosting operation
 * @return int The return value from the semihosting operation (host-dependent)
 *
 * @note This function is typically used for debugging purposes and requires
 *       a debugger that supports ARM semihosting to be attached.
 * @warning Calling this function without a debugger attached may cause the
 *          program to hang or fault.
 */
static inline int semihosting_call(int reason, void* arg)
{
    if (!semihosting_initialized) {
        semihosting_init();
    }

    if (!semihosting_available) {
        return SEMIHOSTING_ERROR_NO_DEBUGGER;
    }

    int result;
    __asm volatile(
#ifdef __riscv
        "mv a0, %[rsn] \n"  // operation
        "mv a1, %[arg] \n"  // args
        // Magic three-instruction sequence for RISC-V semihosting
        ".option push      \n"
        ".option norvc     \n"
        "slli x0, x0, 0x1f \n"
        "ebreak            \n"
        "srai x0, x0, 0x07 \n"
        ".option pop       \n"
        "mv %[val], a0     \n"  // debugger has stored result code in a0

        : [val] "=r"(result)                 // outputs
        : [rsn] "r"(reason), [arg] "r"(arg)  // inputs
        : "a0", "a1", "memory"               // clobbered list
#else
        "mov r0, %[rsn] \n"  // operation
        "mov r1, %[arg] \n"  // args
        "bkpt 0xAB      \n"  // semihosting call
        "mov %[val], r0 \n"  // debugger has stored result code in R0

        : [val] "=r"(result)                 // outputs
        : [rsn] "r"(reason), [arg] "r"(arg)  // inputs
        : "r0", "r1", "cc", "memory"         // clobbered list
#endif
    );

    if (semihosting_fault_occurred) {
        semihosting_available = false;
        return SEMIHOSTING_ERROR_FAULT;
    }

    return result;
}

void semihosting_init(void)
{
    if (semihosting_initialized) {
        return;
    }

    // Install our custom HardFault handler
    // TODO: this is using specifics of Pico-SDK hardware_exception lib -
    // - to make this generic I need to move this to the /port dir
    original_hardfault_handler = exception_set_exclusive_handler(HARDFAULT_EXCEPTION, semihosting_hardfault_handler);

    // Test if semihosting is available by attempting a simple call
    semihosting_fault_occurred = false;

    // Try a simple semihosting call to test availability
    volatile int test_result;
    __asm volatile(
        "mov r0, #0x11 \n"  // SYS_TIME
        "mov r1, #0    \n"  // NULL argument
        "bkpt 0xAB     \n"  // semihosting call
        "mov %[val], r0\n"  // store result
        : [val] "=r"(test_result)
        :
        : "r0", "r1", "cc", "memory");

    // If no fault occurred, semihosting is available
    semihosting_available = !semihosting_fault_occurred;
    semihosting_initialized = true;
}

int semihosting_open(const char* filename, int mode)
{
    struct {
        const char* filename;
        int mode;
        size_t filename_len;
    } args = {
        .filename = filename, .mode = mode, .filename_len = strlen(filename)};

    return semihosting_call(SYS_OPEN, &args);
}

int semihosting_close(int handle)
{
    struct {
        int handle;
    } args = {.handle = handle};

    return semihosting_call(SYS_CLOSE, &args);
}

int semihosting_write(int handle, const void* data, size_t length)
{
    struct {
        int handle;
        const void* data;
        size_t length;
    } args = {.handle = handle, .data = data, .length = length};

    return semihosting_call(SYS_WRITE, &args);
}

int semihosting_read(int handle, void* buffer, size_t length)
{
    struct {
        int handle;
        void* buffer;
        size_t length;
    } args = {.handle = handle, .buffer = buffer, .length = length};

    return semihosting_call(SYS_READ, &args);
}

int semihosting_getTime(void) { return semihosting_call(SYS_TIME, NULL); }