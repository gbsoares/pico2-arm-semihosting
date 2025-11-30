# Define target name
set(TARGET_NAME hello_world)

# Create executable
add_executable(${TARGET_NAME} main.c)

# Link libraries
target_link_libraries(${TARGET_NAME}
    pico_stdlib
    pico_semihosting
)

# Add semihosting linker specs
# target_link_options(${TARGET_NAME} PRIVATE --specs=rdimon.specs)

# Generate additional output formats (map/bin/hex/uf2)
pico_add_extra_outputs(${TARGET_NAME})
