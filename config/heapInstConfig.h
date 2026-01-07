/*
 * Main configuration parameters for the heap trace library.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def HEAPINST_CFG_HARDWARE_PORT
 * @brief Specify what hardware port to use.
 *
 * See heapInstDefines.h for available ports.
 */
#define HEAPINST_CFG_HARDWARE_PORT HEAPINST_HARDWARE_PORT_ARM_Cortex_M

#ifdef __cplusplus
}
#endif
