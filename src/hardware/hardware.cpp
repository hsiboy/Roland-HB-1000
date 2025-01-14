#include "hardware.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "display.h"
#include "pico/stdlib.h"
#include <stdio.h>

namespace pg1000 {
namespace hardware {

// Static member initialization
InitResult Hardware::last_init_result = {HardwareStatus::OK, 0, false, false, 0};
HardwareStatus Hardware::last_error = HardwareStatus::OK;

InitResult Hardware::init_all() {
    InitResult result = {HardwareStatus::OK, 0, false, false, 0};
    
    // Initialize I2C buses first (needed for MCP23017 and LCD)
    if (!I2C::init_all()) {
        result.status = HardwareStatus::ERROR_I2C0_INIT;
        last_error = result.status;
        return result;
    }
    
    // Initialize GPIO expander
    if (!GPIO::init()) {
        result.status = HardwareStatus::ERROR_MCP23017_MISSING;
        last_error = result.status;
        return result;
    }
    result.mcp23017_present = true;
    
    // Initialize ADC system
    if (!ADC::init()) {
        result.status = HardwareStatus::ERROR_SPI_INIT;
        last_error = result.status;
        return result;
    }

    // Detect MCP3008 chips
    result.detected_mcp3008 = 0;
    for (uint8_t i = 0; i < Config::NUM_MCP3008; i++) {
        if (detect_mcp3008(i)) {
            result.detected_mcp3008 |= (1 << i);
        }
    }

    if (result.detected_mcp3008 == 0) {
        result.status = HardwareStatus::ERROR_MCP3008_MISSING;
        last_error = result.status;
        return result;
    }

    // Initialize display
    if (!Display::init()) {
        result.status = HardwareStatus::ERROR_LCD_MISSING;
        last_error = result.status;
        return result;
    }
    result.lcd_present = true;

    // Store result
    last_init_result = result;
    return result;
}

HardwareStatus Hardware::init_spi() {
    if (!ADC::init()) {
        last_error = HardwareStatus::ERROR_SPI_INIT;
        return last_error;
    }
    return HardwareStatus::OK;
}

HardwareStatus Hardware::init_i2c() {
    if (!I2C::init_all()) {
        last_error = HardwareStatus::ERROR_I2C0_INIT;
        return last_error;
    }
    return HardwareStatus::OK;
}

HardwareStatus Hardware::init_midi() {
    // MIDI initialization is handled in the MIDI class
    return HardwareStatus::OK;
}

bool Hardware::self_test() {
    bool all_passed = true;
    
    // Test I2C devices
    printf("Testing I2C devices...\n");
    I2C::scan_bus(I2C::Bus::BUS0);
    I2C::scan_bus(I2C::Bus::BUS1);
    
    // Test MCP3008s
    printf("Testing MCP3008 chips...\n");
    for (uint8_t i = 0; i < Config::NUM_MCP3008; i++) {
        if (detect_mcp3008(i)) {
            printf("MCP3008 #%d: OK\n", i);
        } else {
            printf("MCP3008 #%d: Not found\n", i);
            all_passed = false;
        }
    }
    
    // Test GPIO expander
    printf("Testing MCP23017...\n");
    if (test_gpio_expander()) {
        printf("MCP23017: OK\n");
    } else {
        printf("MCP23017: Failed\n");
        all_passed = false;
    }
    
    return all_passed;
}

void Hardware::print_status() {
    printf("\nHardware Status:\n");
    printf("Last Error: %d\n", static_cast<int>(last_error));
    printf("MCP3008s detected: %02x\n", last_init_result.detected_mcp3008);
    printf("MCP23017 present: %s\n", last_init_result.mcp23017_present ? "Yes" : "No");
    printf("LCD present: %s\n", last_init_result.lcd_present ? "Yes" : "No");
    printf("Error flags: %08x\n", last_init_result.errors);
}

HardwareStatus Hardware::get_last_error() {
    return last_error;
}

bool Hardware::detect_mcp3008(uint8_t chip_num) {
    // Try to read channel 0
    uint16_t value = ADC::read_channel(chip_num, 0);
    
    // If we get a reading that's not all 1's or all 0's, chip is probably present
    return (value != 0 && value != 0x3FF);
}

bool Hardware::test_gpio_expander() {
    // Try to read from MCP23017
    uint8_t test_data;
    return I2C::read_byte(I2C::Bus::BUS0, Config::MCP23017_ADDRESS, 0x00, test_data);
}

} // namespace hardware
} // namespace pg1000
