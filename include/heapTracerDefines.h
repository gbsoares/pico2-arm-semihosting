#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Supported ports
 *
 * TRC_HARDWARE_PORT_HWIndependent

 *
 * TRC_HARDWARE_PORT_APPLICATION_DEFINED
 * Allows for defining the port macros in other source code files.
 *
 * TRC_HARDWARE_PORT_Win32
 * "Accurate" timestamping based on the Windows performance counter for Win32
 * builds. Note that this gives the host machine time, not the kernel time.
 *
 * Hardware specific ports
 * To get accurate timestamping, a hardware timer is necessary. Below are the
 * available ports. Some of these are "unofficial", meaning that
 * they have not yet been verified by Percepio but have been contributed by
 * external developers. They should work, otherwise let us know by emailing
 * support@percepio.com. Some work on any OS platform, while other are specific
 * to a certain operating system.
 *****************************************************************************/

#define TRC_HARDWARE_PORT_NOT_SET          99
#define HEAPTRC_HARDWARE_PORT_ARM_Cortex_M 1

#ifdef __cplusplus
}
#endif