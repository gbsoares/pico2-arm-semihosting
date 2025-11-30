# Memory Allocation Wrapper Target

This directory contains a program that demonstrates how to wrap and log memory allocation calls (malloc, free, realloc) in a Pico 2 project using ARM semihosting.

## Features

- **Memory Allocation Logging**: Prints detailed information whenever malloc(), free(), or realloc() is called
- **ARM Semihosting**: Uses ARM semihosting to output debug information to the host debugger console
- **Edge Case Handling**: Properly handles and logs edge cases like freeing NULL pointers and zero-size allocations

## How It Works

The program uses C preprocessor macros to redirect all calls to the standard memory allocation functions:

- `malloc(size)` → `logged_malloc(size)`
- `free(ptr)` → `logged_free(ptr)` 
- `realloc(ptr, size)` → `logged_realloc(ptr, size)`

Each wrapper function:
1. Calls the original newlib function directly using `_malloc_r()`, `_free_r()`, `_realloc_r()`
2. Logs the operation with details like size, addresses, and operation type
3. Returns the result from the original function

## Building

The target is included in the main CMake build system. You can build it using:

```bash
cmake --build build --target malloc_wrapper
```

Or use the VS Code task "CMake Build malloc_wrapper".

## Output Files

After building, you'll find these files in `build/malloc_wrapper/`:

- `malloc_wrapper.uf2` - UF2 format for easy flashing to Pico 2
- `malloc_wrapper.elf` - ELF executable for debugging
- `malloc_wrapper.bin` - Raw binary format
- `malloc_wrapper.hex` - Intel HEX format

## Example Output

When run with a debugger that supports ARM semihosting, you'll see output like:

```
=== Memory Allocation Wrapper Test ===
This program demonstrates wrapping malloc/free/realloc calls

Testing malloc...
[MALLOC] Requested 100 bytes, allocated at 0x20001234

Testing realloc...
[REALLOC] 0x20001234 -> 200 bytes, new address: 0x20001234

Testing another malloc...
[MALLOC] Requested 50 bytes, allocated at 0x20001300

Testing free operations...
[FREE] Releasing memory at 0x20001234
[FREE] Releasing memory at 0x20001300

Testing edge case: malloc(0)...
[MALLOC] Requested 0 bytes, allocated at (nil)
[FREE] Attempted to free NULL pointer

Testing edge case: realloc(NULL, size)...
[REALLOC] NULL -> 75 bytes (like malloc), allocated at 0x20001234
[FREE] Releasing memory at 0x20001234

=== All memory allocation tests completed ===
```

## Implementation Details

### CMake Configuration

The target uses standard Pico SDK libraries but avoids linking conflicts by using macro redirection instead of linker-level wrapping:

- Links with `pico_stdlib` and `pico_stdio_semihosting`
- Uses `--specs=rdimon.specs` for ARM semihosting support
- No special linker wrapper flags needed

### Code Structure

- **Direct newlib calls**: Uses `_malloc_r()`, `_free_r()`, `_realloc_r()` to bypass any SDK wrappers
- **Macro redirection**: Preprocessor macros redirect standard calls to logging functions
- **Thread-safe**: Uses the global `_impure_ptr` reent structure for thread safety

## Debugging

To see the semihosting output, you'll need a debugger that supports ARM semihosting, such as:

- OpenOCD with GDB
- Segger J-Link
- ST-Link with appropriate tools

The debug output will appear in the debugger console, not on the Pico's UART.