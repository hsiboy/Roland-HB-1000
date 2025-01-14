#include "interface.h"
#include "../hardware/display.h"
#include "../hardware/gpio.h"
#include "../midi/midi.h"
#include "pico/time.h"

namespace pg1000 {
namespace ui {

// Static member initialization
Mode Interface::current_mode = Mode::NORMAL;
MenuItem Interface::current_menu_item = MenuItem::MIDI_CHANNEL;
const Parameter* Interface::current_parameter = nullptr;
uint32_t Interface::last_button_time = 0;
bool Interface::display_needs_update = true;

bool Interface::init() {
    // Set initial parameter to first available
    current_parameter = get_parameter(0);
    
    // Initialize display with welcome screen
    hardware::Display::show_message("D50 Controller", "Initializing...");
    
    return true;
}

void Interface::update() {
    uint32_t now = time_us_32();
    
    // Update based on current mode
    switch (current_mode) {
        case Mode::NORMAL:
            update_normal_mode();
            break;
        case Mode::MENU:
            update_menu_mode();
            break;
        case Mode::PARAMETER_EDIT:
            update_parameter_edit_mode();
            break;
        case Mode::SYSTEM_CONFIG:
            update_system_config_mode();
            break;
    }
    
    // Update display if needed
    if (display_needs_update) {
        switch (current_mode) {
            case Mode::NORMAL:
                update_normal_display();
                break;
            case Mode::MENU:
                update_menu_display();
                break;
            case Mode::PARAMETER_EDIT:
                update_parameter_edit_display();
                break;
            case Mode::SYSTEM_CONFIG:
                update_system_config_display();
                break;
        }
        display_needs_update = false;
    }
}

void Interface::handle_button_press(uint8_t button) {
    last_button_time = time_us_32();
    
    // Map buttons based on current mode
    switch (current_mode) {
        case Mode::NORMAL:
            map_normal_mode_buttons();
            break;
        case Mode::MENU:
            map_menu_mode_buttons();
            break;
        case Mode::PARAMETER_EDIT:
            map_parameter_edit_buttons();
            break;
        case Mode::SYSTEM_CONFIG:
            map_system_config_buttons();
            break;
    }
    
    display_needs_update = true;
}

void Interface::set_mode(Mode mode) {
    if (mode != current_mode) {
        current_mode = mode;
        display_needs_update = true;
        
        // Update LEDs based on mode
        switch (mode) {
            case Mode::NORMAL:
                hardware::GPIO::set_led(0, hardware::LedState::ON);  // UPPER LED
                hardware::GPIO::set_led(1, hardware::LedState::OFF); // LOWER LED
                break;
            case Mode::MENU:
                hardware::GPIO::set_led(0, hardware::LedState::BLINK_SLOW);
                hardware::GPIO::set_led(1, hardware::LedState::BLINK_SLOW);
                break;
            case Mode::PARAMETER_EDIT:
                hardware::GPIO::set_led(0, hardware::LedState::BLINK_FAST);
                break;
            case Mode::SYSTEM_CONFIG:
                hardware::GPIO::set_led(1, hardware::LedState::BLINK_FAST);
                break;
        }
    }
}

void Interface::update_normal_mode() {
    // Check potentiometer changes
    // This is handled in main.cpp through ADC readings
    
    // Check long-press for menu entry
    if (hardware::GPIO::get_button(4) && // MENU button
        (time_us_32() - last_button_time > 1000000)) { // 1 second
        set_mode(Mode::MENU);
    }
}

void Interface::update_menu_mode() {
    // Menu navigation handled by button mapping
}

void Interface::update_parameter_edit_mode() {
    if (!current_parameter) {
        set_mode(Mode::NORMAL);
        return;
    }
    
    // Check for value increment/decrement buttons
    if (hardware::GPIO::get_button(5)) { // INC button
        update_parameter_value(1);
    }
    if (hardware::GPIO::get_button(6)) { // DEC button
        update_parameter_value(-1);
    }
}

void Interface::update_system_config_mode() {
    // System configuration handled by button mapping
}

void Interface::next_parameter() {
    int current_idx = 0;
    for (int i = 0; i < get_parameter_count(); i++) {
        if (get_parameter(i) == current_parameter) {
            current_idx = i;
            break;
        }
    }
    
    current_idx = (current_idx + 1) % get_parameter_count();
    current_parameter = get_parameter(current_idx);
    display_needs_update = true;
}

void Interface::prev_parameter() {
    int current_idx = 0;
    for (int i = 0; i < get_parameter_count(); i++) {
        if (get_parameter(i) == current_parameter) {
            current_idx = i;
            break;
        }
    }
    
    current_idx = (current_idx - 1 + get_parameter_count()) % get_parameter_count();
    current_parameter = get_parameter(current_idx);
    display_needs_update = true;
}

void Interface::update_parameter_value(int16_t change) {
    if (!current_parameter) return;
    
    int16_t new_value = current_parameter->value + change;
    new_value = std::max(0, std::min(static_cast<int16_t>(current_parameter->max_value), new_value));
    
    // Update parameter
    update_parameter_value(current_parameter, static_cast<uint8_t>(new_value));
    display_needs_update = true;
}

void Interface::refresh_display() {
    display_needs_update = true;
}

void Interface::update_normal_display() {
    if (current_parameter) {
        hardware::Display::show_parameter(
            current_parameter->name,
            current_parameter->value,
            current_parameter->max_value
        );
    }
}

void Interface::update_menu_display() {
    const char* menu_text = "";
    switch (current_menu_item) {
        case MenuItem::MIDI_CHANNEL:
            menu_text = "MIDI Channel";
            break;
        case MenuItem::SYSEX_ENABLE:
            menu_text = "SysEx Enable";
            break;
        // Add other menu items
    }
    hardware::Display::show_message("MENU", menu_text);
}

void Interface::update_parameter_edit_display() {
    if (current_parameter) {
        char value_str[16];
        snprintf(value_str, sizeof(value_str), "Value: %d", current_parameter->value);
        hardware::Display::show_message(current_parameter->name, value_str);
    }
}

void Interface::update_system_config_display() {
    hardware::Display::show_message("System Config", "");
}

// Button mapping implementations
void Interface::map_normal_mode_buttons() {
    if (hardware::GPIO::get_button_pressed(0)) { // UPPER
        // Handle UPPER button
    }
    if (hardware::GPIO::get_button_pressed(1)) { // LOWER
        // Handle LOWER button
    }
    // Map other buttons
}

void Interface::map_menu_mode_buttons() {
    if (hardware::GPIO::get_button_pressed(7)) { // ENTER
        execute_menu_item();
    }
    if (hardware::GPIO::get_button_pressed(8)) { // EXIT
        set_mode(Mode::NORMAL);
    }
    if (hardware::GPIO::get_button_pressed(5)) { // INC
        next_menu_item();
    }
    if (hardware::GPIO::get_button_pressed(6)) { // DEC
        prev_menu_item();
    }
}

void Interface::map_parameter_edit_buttons() {
    if (hardware::GPIO::get_button_pressed(8)) { // EXIT
        set_mode(Mode::NORMAL);
    }
}

void Interface::map_system_config_buttons() {
    if (hardware::GPIO::get_button_pressed(8)) { // EXIT
        set_mode(Mode::NORMAL);
    }
}

} // namespace ui
} // namespace pg1000
