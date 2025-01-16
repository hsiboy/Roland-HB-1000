#include "partial_selector.h"

namespace pg1000 {
namespace parameters {

// Static member initialization
bool PartialSelector::upper1_selected = false;
bool PartialSelector::upper2_selected = false;
bool PartialSelector::lower1_selected = false;
bool PartialSelector::lower2_selected = false;

void PartialSelector::init() {
    upper1_selected = false;
    upper2_selected = false;
    lower1_selected = false;
    lower2_selected = false;
    update_leds();
}

void PartialSelector::update() {
    // Check for button presses
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_PARTIAL_UP1)) {
        handle_button_press(hardware::GPIO::BTN_PARTIAL_UP1);
    }
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_PARTIAL_UP2)) {
        handle_button_press(hardware::GPIO::BTN_PARTIAL_UP2);
    }
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_PARTIAL_LOW1)) {
        handle_button_press(hardware::GPIO::BTN_PARTIAL_LOW1);
    }
    if (hardware::GPIO::get_button_pressed(hardware::GPIO::BTN_PARTIAL_LOW2)) {
        handle_button_press(hardware::GPIO::BTN_PARTIAL_LOW2);
    }
}

void PartialSelector::handle_button_press(uint8_t button) {
    switch (button) {
        case hardware::GPIO::BTN_PARTIAL_UP1:
            upper1_selected = !upper1_selected;
            break;
        case hardware::GPIO::BTN_PARTIAL_UP2:
            upper2_selected = !upper2_selected;
            break;
        case hardware::GPIO::BTN_PARTIAL_LOW1:
            lower1_selected = !lower1_selected;
            break;
        case hardware::GPIO::BTN_PARTIAL_LOW2:
            lower2_selected = !lower2_selected;
            break;
    }
    update_leds();
}

bool PartialSelector::is_partial_selected(ParamGroup group) {
    switch (group) {
        case ParamGroup::UPPER_PARTIAL_1:
            return upper1_selected;
        case ParamGroup::UPPER_PARTIAL_2:
            return upper2_selected;
        case ParamGroup::LOWER_PARTIAL_1:
            return lower1_selected;
        case ParamGroup::LOWER_PARTIAL_2:
            return lower2_selected;
        default:
            return true;  // Non-partial parameters are always available
    }
}

void PartialSelector::update_leds() {
    hardware::GPIO::set_led(hardware::GPIO::LED_PARTIAL_UP1, 
        upper1_selected ? hardware::LedState::ON : hardware::LedState::OFF);
    hardware::GPIO::set_led(hardware::GPIO::LED_PARTIAL_UP2, 
        upper2_selected ? hardware::LedState::ON : hardware::LedState::OFF);
    hardware::GPIO::set_led(hardware::GPIO::LED_PARTIAL_LOW1, 
        lower1_selected ? hardware::LedState::ON : hardware::LedState::OFF);
    hardware::GPIO::set_led(hardware::GPIO::LED_PARTIAL_LOW2, 
        lower2_selected ? hardware::LedState::ON : hardware::LedState::OFF);
}

} // namespace parameters
} // namespace pg1000