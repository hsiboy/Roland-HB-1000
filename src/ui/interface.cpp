#include "interface.h"
#include "../hardware/display.h"
#include "../hardware/gpio.h"
#include "../midi/midi.h"
#include <cstdio>
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
    current_parameter = get_parameter(0);
    hardware::Display::show_message("D50 Controller", "Initializing...");
    return true;
}

void Interface::update() {
    uint32_t now = time_us_32();
    
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
    
    if (display_needs_update) {
        update_display();
        display_needs_update = false;
    }
}

void Interface::handle_button_press(uint8_t button) {
    last_button_time = time_us_32();
    
    switch (current_mode) {
        case Mode::NORMAL:
            map_normal_mode_buttons(button);
            break;
        case Mode::MENU:
            map_menu_mode_buttons(button);
            break;
        case Mode::PARAMETER_EDIT:
            map_parameter_edit_buttons(button);
            break;
        case Mode::SYSTEM_CONFIG:
            map_system_config_buttons(button);
            break;
    }
    
    display_needs_update = true;
}

void Interface::set_mode(Mode mode) {
    if (mode != current_mode) {
        current_mode = mode;
        display_needs_update = true;
        
        switch (mode) {
            case Mode::NORMAL:
                hardware::GPIO::set_led(0, hardware::LedState::ON);
                hardware::GPIO::set_led(1, hardware::LedState::OFF);
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

void Interface::next_parameter() {
    if (!current_parameter) return;
    
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
    if (!current_parameter) return;
    
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
    new_value = std::max<int16_t>(0, std::min<int16_t>(current_parameter->max_value, new_value));
    
    update_parameter_value(current_parameter, static_cast<uint8_t>(new_value));
    display_needs_update = true;
}

void Interface::update_parameter_value(const Parameter* param, uint8_t value) {
    if (!param) return;
    const_cast<Parameter*>(param)->value = value;
    midi::MIDI::send_sysex(param);
    display_needs_update = true;
}

void Interface::update_normal_mode() {
    if (hardware::GPIO::get_button(4) && 
        (time_us_32() - last_button_time > 1000000)) {
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
    
    if (hardware::GPIO::get_button(5)) {  // INC button
        update_parameter_value(1);
    }
    if (hardware::GPIO::get_button(6)) {  // DEC button
        update_parameter_value(-1);
    }
}

void Interface::update_system_config_mode() {
    // System configuration handled by button mapping
}

void Interface::update_display() {
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

void Interface::map_normal_mode_buttons(uint8_t button) {
    switch (button) {
        case 0:  // UPPER
            break;
        case 1:  // LOWER
            break;
        case 5:  // INC
            next_parameter();
            break;
        case 6:  // DEC
            prev_parameter();
            break;
        case 7:  // ENTER
            set_mode(Mode::PARAMETER_EDIT);
            break;
    }
}

void Interface::map_menu_mode_buttons(uint8_t button) {
    switch (button) {
        case 7:  // ENTER
            execute_menu_item();
            break;
        case 8:  // EXIT
            set_mode(Mode::NORMAL);
            break;
    }
}

void Interface::map_parameter_edit_buttons(uint8_t button) {
    if (button == 8) {  // EXIT
        set_mode(Mode::NORMAL);
    }
}

void Interface::map_system_config_buttons(uint8_t button) {
    if (button == 8) {  // EXIT
        set_mode(Mode::NORMAL);
    }
}

void Interface::execute_menu_item() {
    // Implement menu actions
    set_mode(Mode::NORMAL);
}

} // namespace ui
} // namespace pg1000
