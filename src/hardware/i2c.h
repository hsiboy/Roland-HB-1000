#pragma once

#include <cstdint>

namespace pg1000 {
namespace hardware {

class I2C {
public:
    // I2C Bus identifiers
    enum class Bus {
        BUS0,   // For MCP23017 (GPIO expander)
        BUS1    // For LCD display
    };

    // I2C Configuration for each device
    struct Config {
        Bus bus;
        uint8_t address;
        uint32_t frequency;
        uint8_t sda_pin;
        uint8_t scl_pin;
    };

    // Common I2C device addresses
    static constexpr uint8_t ADDR_MCP23017 = 0x20;  // Default MCP23017 address
    static constexpr uint8_t ADDR_LCD = 0x27;       // Default LCD address

    // Initialize I2C buses
    static bool init(Bus bus);
    static bool init_all();

    // Basic I2C operations
    static bool write_byte(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t data);
    static bool write_bytes(Bus bus, uint8_t device_addr, uint8_t reg, const uint8_t* data, size_t length);
    static bool read_byte(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t& data);
    static bool read_bytes(Bus bus, uint8_t device_addr, uint8_t reg, uint8_t* data, size_t length);

    // Direct register read/write (no register address)
    static bool write_raw(Bus bus, uint8_t device_addr, uint8_t data);
    static bool read_raw(Bus bus, uint8_t device_addr, uint8_t& data);

    // Utility functions
    static bool device_present(Bus bus, uint8_t device_addr);
    static void scan_bus(Bus bus);  // For debugging - scans for devices

private:
    // Hardware configurations
    static constexpr Config BUS0_CONFIG = {
        Bus::BUS0,      // Bus ID
        0,              // Not used for config
        400'000,        // 400kHz for MCP23017
        4,              // SDA pin
        5               // SCL pin
    };

    static constexpr Config BUS1_CONFIG = {
        Bus::BUS1,      // Bus ID
        0,              // Not used for config
        100'000,        // 100kHz for LCD
        6,              // SDA pin
        7               // SCL pin
    };

    // Get hardware i2c instance for bus
    static i2c_inst_t* get_i2c_inst(Bus bus);
    
    // Internal helper functions
    static bool check_bus(Bus bus);
    static const Config& get_config(Bus bus);
};

} // namespace hardware
} // namespace pg1000
