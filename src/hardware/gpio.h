#pragma once

#include <cstdint>
#include <array>

namespace pg1000 {
namespace hardware {

enum class LedState {
    OFF,
    ON,
    BLINK_SLOW,
    BLINK_FAST
};

class GPIO {
public:
    static constexpr uint8_t NUM_BUTTONS = 10;
    static constexpr uint8_t NUM_LEDS = 6;

    struct Button {
        uint8_t bit;
        bool state;
        bool prev_state;
        uint32_t last_debounce;
        const char* name;
    };

    struct Led {
        uint8_t bit;
        LedState state;
        uint32_t last_toggle;
        const char* name;
    };

    // Initialize GPIO system
    static bool init();

    // LED control
    static void set_led(uint8_t led, LedState state);
    static LedState get_led_state(uint8_t led);

    // Button reading
    static bool get_button(uint8_t button);
    static bool get_button_pressed(uint8_t button);  // Returns true on press event

    // Update function (handles debouncing and LED blinking)
    static void update();

private:
    static constexpr uint8_t I2C_ADDR = 0x20;
    static constexpr uint8_t I2C_PORT = 0;  // i2c0

    // MCP23017 registers
    static constexpr uint8_t REG_IODIRA = 0x00;
    static constexpr uint8_t REG_IODIRB = 0x01;
    static constexpr uint8_t REG_GPPUA = 0x0C;
    static constexpr uint8_t REG_GPPUB = 0x0D;
    static constexpr uint8_t REG_GPIOA = 0x12;
    static constexpr uint8_t REG_GPIOB = 0x13;

    // Internal state
    static std::array<Button, NUM_BUTTONS> buttons;
    static std::array<Led, NUM_LEDS> leds;
    
    // I2C utility functions
    static void write_register(uint8_t reg, uint8_t value);
    static uint8_t read_register(uint8_t reg);
    
    // Update functions
    static void update_buttons();
    static void update_leds();
};

} // namespace hardware
} // namespace pg1000
