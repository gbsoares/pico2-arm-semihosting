# ARM Semihosting on the RPi Pico 2

A library and examples for ARM semihosting on the Raspberry Pi Pico microcontrollers, providing heap tracking and semihosting utilities.

- **Semihosting Utilities** - Low-level semihosting system calls for file I/O
- **Heap Instrumentation** - Memory allocation tracking with binary trace output

## Architecture (planned boundaries)
- **Instrumentation core**: records heap operations into a buffer and exposes a transport-agnostic API for flushing/closing. No direct Pico SDK dependencies.
- **Transports**: pluggable sinks that move bytes to the host (first transport is semihosting; future options include UART/serial and UDP). Transports own formatting/wire protocol and expose a minimal `write`/`close` surface.
- **Platform ports**: platform-specific glue (e.g., HardFault handler install, SDK init) lives under `ports/<platform>/` and injects dependencies into transports at CMake time.
- **Examples/tools**: platform-specific sample apps plus host-side parsers for the trace format to verify transport + instrumentation end-to-end.

## Development Container

This project includes a VS Code dev container configuration for easy setup:

1. Open the project in VS Code
2. When prompted, click "Reopen in Container" or use Command Palette: "Dev Containers: Reopen in Container"
3. The container includes all necessary tools: Pico SDK, CMake, GDB/OpenOCD, and VS Code extensions

> NOTE: if you're running Docker with the WSL backend, you might need to forward the CMSIS-DAP USB device to WSL. Follow [these instructions](https://learn.microsoft.com/en-us/windows/wsl/connect-usb#install-the-usbipd-win-project) for how to set up usbipd.  
> The devcontainer forwards `/dev/bus/usb` devices from host to the container so you can attach to the device and run the debugger (comment out this config if you're not going to be forwarding debugger access to container).

## Requirements

- ARM debugger supporting semihosting (e.g., OpenOCD with GDB)
  - I like to use another Pico device set up as a [debugprobe](https://github.com/raspberrypi/debugprobe)

## Building

Use the command pallete (`CTRL+SHIFT+P`) to access build commands under `Tasks: Run Task` (or `CTRL+SHIFT+B` for quick access to build commands).
