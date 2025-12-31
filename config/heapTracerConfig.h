/*
 * Main configuration parameters for the heap trace library.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def HEAPTRC_CFG_HARDWARE_PORT
 * @brief Specify what hardware port to use.
 *
 * See heapTrcDefines.h for available ports.
 */
#define HEAPTRC_CFG_HARDWARE_PORT HEAPTRC_HARDWARE_PORT_ARM_Cortex_M

#ifdef __cplusplus
}
#endif