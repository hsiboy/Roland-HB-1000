#pragma once

#include <cstdint>

namespace pg1000 {
namespace hardware {

class ADC {
public:
    static constexpr uint8_t NUM_CHIPS = 7;
    static constexpr uint8_t CHANNELS_PER_CHIP = 8;
    static constexpr uint16_t MAX_VALUE = 1023;  // 10-bit ADC

    // Initialize ADC system
    static bool init();

    // Read a single channel from a specific chip
    static uint16_t read_channel(uint8_t chip, uint8_t channel);

    // Read all channels from all chips
    static void read_all();

private:
    // SPI pins
    static constexpr uint8_t PIN_MISO = 16;
    static constexpr uint8_t PIN_CS_BASE = 17;  // CS0-CS6: 17-23
    static constexpr uint8_t PIN_SCK = 18;
    static constexpr uint8_t PIN_MOSI = 19;

    // SPI configuration
    static constexpr uint32_t SPI_BAUDRATE = 3'000'000;  // 3MHz
    static constexpr uint8_t SPI_PORT = 0;  // SPI0

    // Utility functions
    static void chip_select(uint8_t chip, bool select);
    static uint16_t transfer(uint8_t chip, uint8_t channel);
};

} // namespace hardware
} // namespace pg1000
