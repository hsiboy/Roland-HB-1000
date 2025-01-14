#include "gpio.h"
#include "hardware/i2c.h"
#include "pico/time.h"

namespace pg1000 {
namespace hardware {

// Static member initialization
std::array<GPIO::Button, GPIO::NUM_BUTTONS> GPIO::buttons = {{
    {0, false, false, 0, "UPPER"},
    {1, false, false, 0, "LOWER"},
    {2, false, false, 0, "PARTIAL"},
    {3, false, false, 0, "WRITE"},
    {4, false, false, 0, "MODE"},
    {5, false, false, 0, "INC"},
    {6, false, false, 0, "DEC"},
    {7, false, false, 0, "ENTER"},
    {8, false, false, 0, "EXIT"},
    {9, false, false, 0, "MENU"}
}};

std::array<GPIO::Led, GPIO::NUM_LEDS> GPIO::leds = {{
    {0, LedState::OFF, 0, "UPPER"},
    {1, LedState::OFF, 0, "LOWER"},
    {2, LedState::OFF, 0, "PARTIAL"},
    {3, LedState::OFF, 0, "MIDI"},
    {4, LedState::OFF, 0, "WRITE"},
    {5, LedState::OFF, 0, "ERROR"}
}};

bool GPIO::init() {
    // Initialize I2C
    i2c_init(i2c0, 400000);  // 400kHz
    gpio_set_function(4, GPIO_FUNC_I2C);  // SDA
    gpio_set_function(5, GPIO_FUNC_I2C);  // SCL
    
    // Configure MCP23017
    // Port A: Inputs (buttons)
    write_register(REG_IODIRA, 0xFF);
    write_register(REG_GPPUA, 0xFF);  // Enable pull-ups
    
    // Port B: Outputs (LEDs)
    write_register(REG_IODIRB, 0x00);
    write_register(REG_GPIOB, 0x00);  // All LEDs off
    
    return true;
}

void GPIO::set_led(uint8_t led, LedState state) {
    if (led < NUM_LEDS) {
        leds[led].state = state;
        leds[led].last_toggle = time_us_32();
    }
}

LedState GPIO::get_led_state(uint8_t led) {
    return (led < NUM_LEDS) ? leds[led].state : LedState::OFF;
}

bool GPIO::get_button(uint8_t button) {
    return (button < NUM_BUTTONS) ? buttons[button].state : false;
}

bool GPIO::get_button_pressed(uint8_t button) {
    if (button < NUM_BUTTONS) {
        bool current = buttons[button].state;
        bool previous = buttons[button].prev_state;
        return current && !previous;
    }
    return false;
}

void GPIO::update() {
    update_buttons();
    update_leds();
}

void GPIO::write_register(uint8_t reg, uint8_t value) {
    uint8_t buf[] = {reg, value};
    i2c_write_blocking(i2c0, I2C_ADDR, buf, 2, false);
}

uint8_t GPIO::read_register(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c0, I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, I2C_ADDR, &value, 1, false);
    return value;
}

void GPIO::update_buttons() {
    // Read button states
    uint8_t button_states = read_register(REG_GPIOA);
    uint32_t current_time = time_us_32();
    
    // Update button states with debouncing
    for (auto& button : buttons) {
        bool raw_state = !(button_states & (1 << button.bit));
        
        if (raw_state != button.state) {
            if (current_time - button.last_debounce > 5000) {  // 5ms debounce
                button.prev_state = button.state;
                button.state = raw_state;
            }
            button.last_debounce = current_time;
        }
    }
}

void GPIO::update_leds() {
    uint8_t led_state = 0;
    uint32_t current_time = time_us_32();
    
    for (const auto& led : leds) {
        bool should_be_on = false;
        
        switch (led.state) {
            case LedState::ON:
                should_be_on = true;
                break;
            case LedState::BLINK_SLOW:
                should_be_on = ((current_time - led.last_toggle) / 500000) % 2;
                break;
            case LedState::BLINK_FAST:
                should_be_on = ((current_time - led.last_toggle) / 100000) % 2;
                break;
            default:
                break;
        }
        
        if (should_be_on) {
            led_state |= (1 << led.bit);
        }
    }
    
    write_register(REG_GPIOB, led_state);
}

} // namespace hardware
} // namespace pg1000
