#pragma once

#include "../hardware/gpio.h"

namespace pg1000 {
namespace parameters {

class CommonSelector {
public:
    static void init();
    static void update();
    
    // Selection state
    static bool is_upper_selected() { return upper_selected; }
    static bool is_lower_selected() { return lower_selected; }
    static bool is_both_selected() { return upper_selected && lower_selected; }
    static bool is_none_selected() { return !upper_selected && !lower_selected; }

    // Handle button presses
    static void handle_button_press(uint8_t button);

private:
    static bool upper_selected;
    static bool lower_selected;

    // Update LED states based on selection
    static void update_leds();
};

} // namespace parameters
} // namespace pg1000