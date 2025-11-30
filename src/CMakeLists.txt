# pico_semihosting library
cmake_minimum_required(VERSION 3.12)

# Create the pico_semihosting library
add_library(pico_semihosting STATIC
    semihosting_utils.c
    heap_tracker.c
)

# Set the include directories for the library
target_include_directories(pico_semihosting 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link required Pico SDK libraries
target_link_libraries(pico_semihosting
    PUBLIC
        pico_stdlib
        pico_stdio_semihosting
        pico_time
)

# Add compiler warnings for library development
target_compile_options(pico_semihosting PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# Set C standard
set_property(TARGET pico_semihosting PROPERTY C_STANDARD 11)