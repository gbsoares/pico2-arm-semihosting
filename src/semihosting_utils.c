#include "pico_semihosting/semihosting_utils.h"
#include <string.h>

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
inline int semihosting_call(int reason, void* arg)
{
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

    return result;
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