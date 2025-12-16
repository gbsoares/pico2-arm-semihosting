# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
# Hello World example for pico.semihosting library
set(TARGET_NAME hello_world)

add_executable(${TARGET_NAME} main.c)

# Link libraries using the new library target name
target_link_libraries(${TARGET_NAME}
    pico_stdlib
    pico.semihosting
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_none)
endif()

# Generate additional output formats (map/bin/hex/uf2)
pico_add_extra_outputs(${TARGET_NAME})