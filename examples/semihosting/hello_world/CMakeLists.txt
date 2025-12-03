# Define target name
set(TARGET_NAME hello_world)

# Create executable
add_executable(${TARGET_NAME} main.c)

# Link libraries
target_link_libraries(${TARGET_NAME}
    pico_stdlib
    semihosting
)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(${TARGET_NAME} pico_cyw43_arch_none)
endif()

# Generate additional output formats (map/bin/hex/uf2)
pico_add_extra_outputs(${TARGET_NAME})
