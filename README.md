# ARM Semihosting on the RPi Pico 2

Project exploring ARM semihosting on the Raspberry Pi Pico microcontrollers.

## Development Container

This project includes a VS Code dev container configuration for easy setup:

1. Open the project in VS Code
2. When prompted, click "Reopen in Container" or use Command Palette: "Dev Containers: Reopen in Container"
3. The container includes all necessary tools: Pico SDK, CMake, GDB/OpenOCD, and VS Code extensions

> NOTE: if you're running Docker with the WSL backend, you might need to forward the CMSIS-DAP USB device to WSL. Follow [these instructions](https://learn.microsoft.com/en-us/windows/wsl/connect-usb#install-the-usbipd-win-project) for how to set up usbipd.  
> The devcontainer forwards `/dev/bus/usb` devices from host to the container so you can attach to the device and run the debugger.

## Requirements

- Raspberry Pi Pico SDK 2.2.0 or later
- CMake 3.12 or later
- ARM debugger supporting semihosting (e.g., OpenOCD with GDB)
  - I like to use another Pico device set up as a [debugprobe](https://github.com/raspberrypi/debugprobe)

## Building

Use the command pallete (`CTRL+SHIFT+P`) to access build commands under `Tasks: Run Task` (or `CTRL+SHIFT+B` for quick access to build commands), or invoke the cmake commands manually:

```bash
# Configure the build
cmake -S . -B build -DPICO_PLATFORM=rp2350 -DPICO_BOARD=pico2
# Build the project
cmake --build build --target hello_world
```

