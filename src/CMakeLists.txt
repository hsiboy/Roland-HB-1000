cmake_minimum_required(VERSION 3.13)

# Initialize pico-sdk from submodule
# uncomment if you want to use the SDK in-tree
# add_subdirectory(pico-sdk)

include(pico_sdk_import.cmake)

project(d50_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(d50_controller
    Roland-PG-1000.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(Roland-PG-1000 
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(Roland-PG-1000)

# Enable USB output, disable uart output
pico_enable_stdio_usb(Roland-PG-1000 1)
pico_enable_stdio_uart(Roland-PG-1000 0)
