# pico_semihosting library
cmake_minimum_required(VERSION 3.12)

# Create the pico_semihosting library
add_library(semihosting STATIC
    semihosting.c
)

# Set the include directories for the library
target_include_directories(semihosting 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link required Pico SDK libraries
target_link_libraries(semihosting
    PUBLIC
        pico_stdlib
        hardware_exception
)

# Add compiler warnings for library development
target_compile_options(semihosting PRIVATE
    -Wall
    -Wextra
    # -Wno-unused-parameter
)

# Set C standard
set_property(TARGET semihosting PROPERTY C_STANDARD 11)