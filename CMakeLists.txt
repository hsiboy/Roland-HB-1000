cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(roland_pg1000 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(roland_pg1000
    src/main.cpp
    src/hardware/adc.cpp
    src/hardware/display.cpp
    src/hardware/gpio.cpp
    src/hardware/i2c.cpp
    src/hardware/hardware.cpp
    src/midi/midi.cpp
    src/midi/sysex.cpp
    src/parameters/parameters.cpp
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(roland_pg1000 
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_uart
)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(roland_pg1000)

# Enable USB output, disable uart output
pico_enable_stdio_usb(roland_pg1000 1)
pico_enable_stdio_uart(roland_pg1000 0)
