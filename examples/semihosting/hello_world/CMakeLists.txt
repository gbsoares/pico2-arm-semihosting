# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Hello World example for pico.semihosting library
set(TARGET_NAME hello_world)

add_executable(${TARGET_NAME} main.c)

target_link_libraries(${TARGET_NAME}
    pico_stdlib
    semihosting
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_none)
endif()

# Generate additional output formats (map/bin/hex/uf2)
if(CFG_PLATFORM STREQUAL "PICO" AND COMMAND pico_add_extra_outputs)
    pico_add_extra_outputs(${TARGET_NAME})
endif()
