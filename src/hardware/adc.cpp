#include "adc.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace pg1000 {
namespace hardware {

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
    
    return true;
}

uint16_t ADC::read_channel(uint8_t chip, uint8_t channel) {
    if (chip >= NUM_CHIPS || channel >= CHANNELS_PER_CHIP) {
        return 0;
    }
    
    return transfer(chip, channel);
}

void ADC::read_all() {
    for (uint8_t chip = 0; chip < NUM_CHIPS; chip++) {
        for (uint8_t channel = 0; channel < CHANNELS_PER_CHIP; channel++) {
            read_channel(chip, channel);
        }
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

} // namespace hardware
} // namespace pg1000
