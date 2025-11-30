# Define target name
set(TARGET_NAME heap_tracker)

# Create executable
add_executable(${TARGET_NAME} main.c)

# Link libraries (now using the pico_semihosting library)
target_link_libraries(${TARGET_NAME}
    pico_semihosting
    pico_stdlib
    pico_stdio_semihosting
    pico_time
)

# Add semihosting linker specs
target_link_options(${TARGET_NAME} PRIVATE 
    --specs=rdimon.specs
)

# Generate additional output formats (map/bin/hex/uf2)
pico_add_extra_outputs(${TARGET_NAME})