#pragma once

#include "../hardware/gpio.h"
#include "parameters.h"

namespace pg1000 {
namespace parameters {

class PartialSelector {
public:
    static void init();
    static void update();
    
    // Selection state queries
    static bool is_upper1_selected() { return upper1_selected; }
    static bool is_upper2_selected() { return upper2_selected; }
    static bool is_lower1_selected() { return lower1_selected; }
    static bool is_lower2_selected() { return lower2_selected; }
    
    // Group selection checks
    static bool is_any_upper_selected() { return upper1_selected || upper2_selected; }
    static bool is_any_lower_selected() { return lower1_selected || lower2_selected; }
    static bool is_partial_selected(ParamGroup group);

    // Handle button presses
    static void handle_button_press(uint8_t button);

private:
    static bool upper1_selected;
    static bool upper2_selected;
    static bool lower1_selected;
    static bool lower2_selected;

    // Update LED states based on selection
    static void update_leds();
};

} // namespace parameters
} // namespace pg1000
