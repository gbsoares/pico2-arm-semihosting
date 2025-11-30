#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file semihosting_utils.h
 * @brief Raw semihosting utility functions for Pico
 *
 * This header provides low-level semihosting system call access for
 * file operations and other host interactions.
 */

// Raw semihosting system call numbers
#define SYS_OPEN 0x01
#define SYS_CLOSE 0x02
#define SYS_WRITE 0x05
#define SYS_READ 0x06

// File open modes
#define OPEN_MODE_R 0       ///< Read
#define OPEN_MODE_RB 1      ///< Read binary
#define OPEN_MODE_R_PLUS 2  ///< Read/write
#define OPEN_MODE_W 4       ///< Write
#define OPEN_MODE_WB 6      ///< Write binary
#define OPEN_MODE_A 8       ///< Append
#define OPEN_MODE_AB 10     ///< Append binary

/**
 * @brief Execute a raw semihosting system call
 *
 * @param syscall System call number (SYS_* constants)
 * @param args Pointer to arguments structure
 * @return System call result (varies by call)
 */
int semihosting_call(int reason, void* arg);

/**
 * @brief Open a file via semihosting
 *
 * @param filename Path to file on host system
 * @param mode File open mode (OPEN_MODE_* constants)
 * @return File handle on success, -1 on failure
 */
int semihosting_open(const char* filename, int mode);

/**
 * @brief Close a file via semihosting
 *
 * @param handle File handle from semihosting_open
 * @return 0 on success, non-zero on failure
 */
int semihosting_close(int handle);

/**
 * @brief Write data to file via semihosting
 *
 * @param handle File handle from semihosting_open
 * @param data Pointer to data to write
 * @param length Number of bytes to write
 * @return Number of bytes written, or negative on error
 */
int semihosting_write(int handle, const void* data, size_t length);

/**
 * @brief Read data from file via semihosting
 *
 * @param handle File handle from semihosting_open
 * @param buffer Pointer to buffer for read data
 * @param length Maximum number of bytes to read
 * @return Number of bytes read, or negative on error
 */
int semihosting_read(int handle, void* buffer, size_t length);

#ifdef __cplusplus
}
#endif
