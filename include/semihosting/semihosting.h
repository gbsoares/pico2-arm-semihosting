#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file semihosting.h
 * @brief Raw semihosting utility functions for Pico
 *
 * This header provides low-level semihosting system call access for
 * file operations and other host interactions.
 */

/* File open modes */
#define OPEN_MODE_R      0 /*!< Read */
#define OPEN_MODE_RB     1 /*!< Read binary */
#define OPEN_MODE_R_PLUS 2 /*!< Read/write */
#define OPEN_MODE_W      4 /*!< Write */
#define OPEN_MODE_WB     5 /*!< Write binary */
#define OPEN_MODE_A      8 /*!< Append */
#define OPEN_MODE_AB     9 /*!< Append binary */

/* Semihosting error codes */
#define SEMIHOSTING_ERROR_NO_DEBUGGER -2
#define SEMIHOSTING_ERROR_FAULT       -3

/**
 * @brief Initialize semihosting with fault protection
 *
 * This must be called before using any semihosting functions.
 * It sets up the HardFault handler to catch semihosting faults.
 */
void semihosting_init(void);

/**
 * @brief Check if semihosting is available (debugger attached)
 *
 * @return true if semihosting is available, false otherwise
 */
bool semihosting_is_available(void);

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

/**
 * @brief Get current time via semihosting
 *
 * Returns the number of seconds since January 1, 1970 (Unix epoch time).
 *
 * @return Time in seconds since Unix epoch, or -1 on error
 */
int semihosting_getTime(void);

#ifdef __cplusplus
}
#endif
