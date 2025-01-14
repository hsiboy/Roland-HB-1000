#pragma once

#include <cstdint>

namespace pg1000 {
namespace hardware {

// Pin Assignments
struct Pins {
    // SPI (MCP3008)
    static constexpr uint8_t SPI_MISO = 16;
    static constexpr uint8_t SPI_CS_BASE = 17;  // CS0-CS6: 17-23
    static constexpr uint8_t SPI_SCK = 18;
    static constexpr uint8_t SPI_MOSI = 19;

    // I2C0 (MCP23017)
    static constexpr uint8_t I2C0_SDA = 4;
    static constexpr uint8_t I2C0_SCL = 5;

    // I2C1 (LCD)
    static constexpr uint8_t I2C1_SDA = 6;
    static constexpr uint8_t I2C1_SCL = 7;

    // MIDI UART
    static constexpr uint8_t MIDI_TX = 0;
    static constexpr uint8_t MIDI_RX = 1;
};

// Hardware Configuration
struct Config {
    // SPI Configuration
    static constexpr uint32_t SPI_FREQUENCY = 3'000'000;  // 3MHz
    static constexpr uint8_t NUM_MCP3008 = 7;
    static constexpr uint8_t MCP3008_CHANNELS = 8;

    // I2C Configuration
    static constexpr uint32_t I2C0_FREQUENCY = 400'000;   // 400kHz for MCP23017
    static constexpr uint32_t I2C1_FREQUENCY = 100'000;   // 100kHz for LCD
    static constexpr uint8_t MCP23017_ADDRESS = 0x20;     // Default address
    static constexpr uint8_t LCD_ADDRESS = 0x27;          // Default address

    // MIDI Configuration
    static constexpr uint32_t MIDI_BAUD_RATE = 31250;
};

// Hardware Status/Error Codes
enum class HardwareStatus {
    OK = 0,
    ERROR_SPI_INIT,
    ERROR_I2C0_INIT,
    ERROR_I2C1_INIT,
    ERROR_MIDI_INIT,
    ERROR_MCP3008_MISSING,
    ERROR_MCP23017_MISSING,
    ERROR_LCD_MISSING,
    ERROR_DEVICE_TIMEOUT,
    ERROR_INVALID_PARAMETER
};

// Hardware Initialization Results
struct InitResult {
    HardwareStatus status;
    uint8_t detected_mcp3008;    // Bitmap of detected MCP3008s
    bool mcp23017_present;
    bool lcd_present;
    uint32_t errors;             // Detailed error flags if needed
};

// System-wide Hardware Interface
class Hardware {
public:
    // Initialize all hardware
    static InitResult init_all();
    
    // Individual subsystem initialization
    static HardwareStatus init_spi();
    static HardwareStatus init_i2c();
    static HardwareStatus init_midi();
    
    // Hardware status checking
    static bool self_test();
    static void print_status();
    static HardwareStatus get_last_error();

private:
    static InitResult last_init_result;
    static HardwareStatus last_error;
    
    // Internal helper functions
    static bool detect_devices();
    static void test_connections();
};

} // namespace hardware
} // namespace pg1000
