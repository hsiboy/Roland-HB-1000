# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

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
    src/parameters/common_selector.cpp
    src/parameters/partial_selector.cpp
    src/ui/interface.cpp
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
