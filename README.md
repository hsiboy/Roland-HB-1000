# HB-1000 - A homebrew Roland PG-1000

A DIY hardware controller for the Roland D50 synthesizer, providing hands-on control of parameters typically accessed through menu diving. This project aims to replicate the functionality of the Roland PG-1000 programmer unit, which is now rare and expensive in the second-hand market.

## Background

The Roland D50 is a classic digital synthesizer from 1987. While powerful, its interface relies heavily on menu diving for parameter adjustments. Roland released the PG-1000 programmer unit to provide direct hardware control, but these are now rare and often sell for more than the D50 itself ($500+). While commercial alternatives exist (around $400+), this project provides a maker-friendly solution at a significantly lower cost.

## Features

- 56 potentiometers for direct parameter control
- 10 push buttons for mode selection and navigation
- 6 status LEDs
- 16x2 LCD display for parameter values
- MIDI In/Out via standard DIN connectors
- Compatible with original D50 SysEx protocol
- Optional CC output for DAW integration
- Parameter value smoothing to prevent jumps
- Group switching (UPPER/LOWER/COMMON)

## Hardware Requirements

### Core Components
- Raspberry Pi Pico (RP2040) or compatible board
- 7x MCP3008 ADC chips (10-bit, 8-channel)
- 1x MCP23017 I/O expander
- 1x 16x2 LCD display with I2C backpack
- 56x 50kΩ B-taper potentiometers
- 10x SPST momentary push buttons
- 6x 3mm LEDs (with 330Ω resistors)
- MIDI ports (standard 5-pin DIN connectors)

### Additional Components
- 5V power supply
- 0.1µF decoupling capacitors (one per IC)
- PCB or prototype board - TODO
- Enclosure - TODO
- Knobs for potentiometers
- Wire, headers, etc.

## Pin Assignments

### RP2040 Connections

#### SPI0 (MCP3008s)
- GPIO 16: MISO
- GPIO 17-23: Chip Select (CS0-CS6)
- GPIO 18: SCK
- GPIO 19: MOSI

#### I2C0 (MCP23017)
- GPIO 4: SDA
- GPIO 5: SCL

#### I2C1 (LCD Display)
- GPIO 6: SDA
- GPIO 7: SCL

#### MIDI
- GPIO 0: UART TX (MIDI Out)
- GPIO 1: UART RX (MIDI In)

### MCP23017 Connections
- PORTA: 10 buttons (with internal pull-ups)
- PORTB: 6 LEDs (with 330Ω current limiting resistors)

```txt
src/
├── hardware/
│   ├── adc.cpp         // MCP3008 handling
│   ├── adc.h
│   ├── gpio.cpp        // LED and button handling via MCP23017
│   ├── gpio.h
│   ├── display.cpp     // LCD control
│   ├── display.h
│   └── hardware.h      // Common hardware definitions
├── midi/
│   ├── midi.cpp        // MIDI message handling
│   ├── midi.h
│   ├── sysex.cpp       // SysEx specific handling
│   └── sysex.h
├── parameters/
│   ├── parameters.cpp  // Parameter definitions and handling
│   └── parameters.h
├── ui/
│   ├── interface.cpp   // User interface logic
│   └── interface.h
└── main.cpp            // Main program loop
```

## Software Requirements

### Development Tools
- ARM GCC Toolchain
- CMake (3.13 or newer)
- Python 3.x (for build scripts)
- Visual Studio Code (recommended)
- Raspberry Pi Pico SDK

### Installation (Ubuntu/Debian)
```bash
# Install required packages
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential

# Clone Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ..

# Set Pico SDK path
export PICO_SDK_PATH=/path/to/pico-sdk
```

### Building the Project
```bash
# Clone this repository
git clone https://github.com/hsiboy/Roland-PG-1000.git
cd d50-controller

# Create build directory
mkdir build
cd build

# Generate build files
cmake ..

# Build the project
make
```

### Uploading to RP2040
1. Hold the BOOTSEL button on the Pico
2. Connect the Pico to your computer via USB
3. Release BOOTSEL
4. The Pico will appear as a mass storage device
5. Copy `Roland-PG-1000.uf2` to the Pico drive

## Usage

1. Connect MIDI Out to D50 MIDI In
2. Power on the controller
3. Set D50 to receive MIDI on channel 1 (or configure controller to match D50)
4. Press UPPER or LOWER button to select parameter group
5. Adjust parameters using knobs
6. LCD will show current parameter and value

## Configuration

The controller can be configured via the menu system:
- MIDI channel selection
- Display contrast
- LED brightness
- Parameter calibration
- MIDI message type (SysEx/CC/Both)

## Contributing

Contributions are welcome! Please read our contributing guidelines before submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Original PG-1000 SysEx protocol documentation
- Roland D50 service manual
- Community research on parameter mappings

## Support

For questions, issues, or suggestions:
- Open an issue on GitHub

## Future Plans

- PCB design files
- 3D printable enclosure
- Additional parameter mappings
- Firmware updates
- Web-based configuration tool
