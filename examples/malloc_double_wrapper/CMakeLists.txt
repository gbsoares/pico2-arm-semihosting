# Define target name  
set(TARGET_NAME malloc_double_wrapper)

# Create executable
add_executable(${TARGET_NAME} main.c)

# Link libraries - use pico_stdlib which includes pico_malloc
target_link_libraries(${TARGET_NAME}
    pico_stdlib
    pico_stdio_semihosting
)

# Add semihosting linker specs
# We DON'T use --wrap here, instead we manually call the Pico functions
target_link_options(${TARGET_NAME} PRIVATE 
    --specs=rdimon.specs
)

# Generate additional output formats (map/bin/hex/uf2)
pico_add_extra_outputs(${TARGET_NAME})