#pragma once

#include <cstdint>
#include <array>
#include "value_smoother.h"

namespace pg1000 {
namespace hardware {

class ADC {
public:
    static constexpr uint8_t NUM_CHIPS = 7;
    static constexpr uint8_t CHANNELS_PER_CHIP = 8;
    static constexpr uint16_t MAX_VALUE = 1023;  // 10-bit ADC
    static constexpr uint16_t HYSTERESIS_THRESHOLD = 4;  // Prevent small value fluctuations
    static constexpr uint16_t CALIBRATION_SAMPLES = 16;  // Number of samples for calibration

    // Initialize ADC system
    static bool init();

    // Read a single channel from a specific chip
    static uint16_t read_channel(uint8_t chip, uint8_t channel);

    // Read all channels from all chips and apply smoothing
    static void read_all();

    // Get the last read value for a channel
    static uint16_t get_value(uint8_t chip, uint8_t channel);

    // Check if a channel's value has changed significantly
    static bool has_changed(uint8_t chip, uint8_t channel);

    // Calibrate all channels
    static void calibrate();

    // Reset all smoothers
    static void reset();

private:
    // SPI pins
    static constexpr uint8_t PIN_MISO = 16;
    static constexpr uint8_t PIN_CS_BASE = 17;  // CS0-CS6: 17-23
    static constexpr uint8_t PIN_SCK = 18;
    static constexpr uint8_t PIN_MOSI = 19;

    // SPI configuration
    static constexpr uint32_t SPI_BAUDRATE = 3'000'000;  // 3MHz
    static constexpr uint8_t SPI_PORT = 0;  // SPI0

    // Value smoothing and caching
    static std::array<std::array<ValueSmoother<8>, CHANNELS_PER_CHIP>, NUM_CHIPS> smoothers;
    static std::array<std::array<uint16_t, CHANNELS_PER_CHIP>, NUM_CHIPS> cached_values;
    static std::array<std::array<bool, CHANNELS_PER_CHIP>, NUM_CHIPS> value_changed;

    // Calibration values
    static std::array<std::array<uint16_t, CHANNELS_PER_CHIP>, NUM_CHIPS> min_values;
    static std::array<std::array<uint16_t, CHANNELS_PER_CHIP>, NUM_CHIPS> max_values;

    // Utility functions
    static void chip_select(uint8_t chip, bool select);
    static uint16_t transfer(uint8_t chip, uint8_t channel);
    static uint16_t normalize_value(uint16_t value, uint16_t min_val, uint16_t max_val);
};

} // namespace hardware
} // namespace pg1000
