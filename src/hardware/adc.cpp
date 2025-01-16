#include "adc.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace pg1000 {
namespace hardware {

// Static member initialization
std::array<std::array<ValueSmoother<8>, ADC::CHANNELS_PER_CHIP>, ADC::NUM_CHIPS> ADC::smoothers;
std::array<std::array<uint16_t, ADC::CHANNELS_PER_CHIP>, ADC::NUM_CHIPS> ADC::cached_values;
std::array<std::array<bool, ADC::CHANNELS_PER_CHIP>, ADC::NUM_CHIPS> ADC::value_changed;
std::array<std::array<uint16_t, ADC::CHANNELS_PER_CHIP>, ADC::NUM_CHIPS> ADC::min_values;
std::array<std::array<uint16_t, ADC::CHANNELS_PER_CHIP>, ADC::NUM_CHIPS> ADC::max_values;

bool ADC::init() {
    // Initialize SPI
    spi_init(spi_default, SPI_BAUDRATE);
    
    // Configure SPI pins
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Configure chip select pins
    for (uint8_t i = 0; i < NUM_CHIPS; i++) {
        gpio_init(PIN_CS_BASE + i);
        gpio_set_dir(PIN_CS_BASE + i, GPIO_OUT);
        gpio_put(PIN_CS_BASE + i, 1);  // Deselect all chips
    }

    // Initialize arrays
    for (auto& chip_smoothers : smoothers) {
        for (auto& smoother : chip_smoothers) {
            smoother.reset();
        }
    }

    for (auto& chip_values : cached_values) {
        chip_values.fill(0);
    }

    for (auto& chip_changed : value_changed) {
        chip_changed.fill(false);
    }

    // Set initial calibration values
    for (auto& chip_min : min_values) {
        chip_min.fill(MAX_VALUE);  // Start high for min detection
    }
    
    for (auto& chip_max : max_values) {
        chip_max.fill(0);  // Start low for max detection
    }
    
    return true;
}

uint16_t ADC::read_channel(uint8_t chip, uint8_t channel) {
    if (chip >= NUM_CHIPS || channel >= CHANNELS_PER_CHIP) {
        return 0;
    }
    
    uint16_t raw_value = transfer(chip, channel);
    uint16_t smoothed_value = smoothers[chip][channel].update(raw_value);
    uint16_t normalized_value = normalize_value(smoothed_value, min_values[chip][channel], max_values[chip][channel]);
    
    // Apply hysteresis and update cached value
    uint16_t previous_value = cached_values[chip][channel];
    uint16_t final_value = smoothers[chip][channel].apply_hysteresis(normalized_value, previous_value, HYSTERESIS_THRESHOLD);
    
    // Update cache and change flag
    if (final_value != previous_value) {
        cached_values[chip][channel] = final_value;
        value_changed[chip][channel] = true;
    }
    
    return final_value;
}

void ADC::read_all() {
    for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) {
        for (uint8_t channel = 0; channel < CHANNELS_PER_CHIP; channel++) {
            read_channel(chip, channel);
        }
    }
}

uint16_t ADC::get_value(uint8_t chip, uint8_t channel) {
    if (chip >= NUM_CHIPS || channel >= CHANNELS_PER_CHIP) {
        return 0;
    }
    return cached_values[chip][channel];
}

bool ADC::has_changed(uint8_t chip, uint8_t channel) {
    if (chip >= NUM_CHIPS || channel >= CHANNELS_PER_CHIP) {
        return false;
    }
    bool changed = value_changed[chip][channel];
    value_changed[chip][channel] = false;  // Clear the flag
    return changed;
}

void ADC::calibrate() {
    // Reset calibration values
    for (auto& chip_min : min_values) {
        chip_min.fill(MAX_VALUE);
    }
    for (auto& chip_max : max_values) {
        chip_max.fill(0);
    }
    
    // Sample multiple times to find min/max values
    for (uint16_t sample = 0; sample < CALIBRATION_SAMPLES; sample++) {
        for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) {
            for (uint8_t channel = 0; channel < CHANNELS_PER_CHIP; channel++) {
                uint16_t value = transfer(chip, channel);
                
                // Update min/max values
                if (value < min_values[chip][channel]) {
                    min_values[chip][channel] = value;
                }
                if (value > max_values[chip][channel]) {
                    max_values[chip][channel] = value;
                }
            }
        }
        sleep_ms(10);  // Small delay between samples
    }
    
    // Reset smoothers after calibration
    reset();
}

void ADC::reset() {
    for (auto& chip_smoothers : smoothers) {
        for (auto& smoother : chip_smoothers) {
            smoother.reset();
        }
    }
    
    for (auto& chip_values : cached_values) {
        chip_values.fill(0);
    }
    
    for (auto& chip_changed : value_changed) {
        chip_changed.fill(false);
    }
}

void ADC::chip_select(uint8_t chip, bool select) {
    if (chip < NUM_CHIPS) {
        gpio_put(PIN_CS_BASE + chip, !select);  // Active low
    }
}

uint16_t ADC::transfer(uint8_t chip, uint8_t channel) {
    // MCP3008 command bits
    static constexpr uint8_t START_BIT = 0x01;
    static constexpr uint8_t SINGLE_ENDED = 0x80;
    
    // Prepare command bytes
    uint8_t tx_data[3] = {
        START_BIT,                    // Start bit
        static_cast<uint8_t>(SINGLE_ENDED | ((channel & 0x07) << 4)),  // Single-ended mode + channel
        0x00                          // Don't care byte
    };
    uint8_t rx_data[3] = {0};
    
    chip_select(chip, true);
    
    spi_write_read_blocking(spi_default, tx_data, rx_data, 3);
    
    chip_select(chip, false);
    
    // Combine result bits from received bytes
    // MCP3008 returns:
    // Byte 1: Null
    // Byte 2: [B9][B8][B7][B6][B5][B4][B3][B2]
    // Byte 3: [B1][B0][x][x][x][x][x][x]
    return ((rx_data[1] & 0x03) << 8) | rx_data[2];
}

uint16_t ADC::normalize_value(uint16_t value, uint16_t min_val, uint16_t max_val) {
    // Prevent division by zero
    if (min_val >= max_val) {
        return value;
    }
    
    // Clamp value to min/max range
    if (value <= min_val) return 0;
    if (value >= max_val) return MAX_VALUE;
    
    // Scale to full range
    uint32_t scaled = static_cast<uint32_t>(value - min_val) * MAX_VALUE / (max_val - min_val);
    return static_cast<uint16_t>(scaled);
}

} // namespace hardware
} // namespace pg1000
