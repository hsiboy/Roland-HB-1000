#include "common_selector.h"

namespace pg1000 {
namespace parameters {

// Static member initialization
bool CommonSelector::upper_selected = false;
bool CommonSelector::lower_selected = false;

void CommonSelector::init() {
    upper_selected = false;
    lower_selected = false;
    update_leds();
}

void CommonSelector::update() {
    // Check for button presses
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_COMMON_UPPER)) {
        handle_button_press(hardware::GPIO::BTN_COMMON_UPPER);
    }
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_COMMON_LOWER)) {
        handle_button_press(hardware::GPIO::BTN_COMMON_LOWER);
    }
}

void CommonSelector::handle_button_press(uint8_t button) {
    if (button == hardware::GPIO::BTN_COMMON_UPPER) {
        upper_selected = !upper_selected;
    } else if (button == hardware::GPIO::BTN_COMMON_LOWER) {
        lower_selected = !lower_selected;
    }
    update_leds();
}

void CommonSelector::update_leds() {
    // Update LED states based on selection
    hardware::GPIO::set_led(hardware::GPIO::LED_COMMON_UPPER, 
        upper_selected ? hardware::LedState::ON : hardware::LedState::OFF);
    
    hardware::GPIO::set_led(hardware::GPIO::LED_COMMON_LOWER,
        lower_selected ? hardware::LedState::ON : hardware::LedState::OFF);
}

} // namespace parameters
} // namespace pg1000