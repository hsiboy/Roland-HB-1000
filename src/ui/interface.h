#pragma once

#include <cstdint>
#include "../parameters/parameters.h"

namespace pg1000 {
namespace ui {

// UI States/Modes
enum class Mode {
    NORMAL,         // Normal operation mode
    MENU,           // Menu navigation
    PARAMETER_EDIT, // Direct parameter value editing
    SYSTEM_CONFIG   // System configuration
};

// Menu Items
enum class MenuItem {
    MIDI_CHANNEL,
    SYSEX_ENABLE,
    CC_ENABLE,
    DISPLAY_CONTRAST,
    CALIBRATE,
    SAVE_CONFIG,
    LOAD_CONFIG,
    FACTORY_RESET
};

class Interface {
public:
   // Core functions
   static bool init();
   static void update();
   
   // Button handling
   static void handle_button_press(uint8_t button);
   static void handle_button_release(uint8_t button);
   static void handle_button_hold(uint8_t button);

   // Mode management
   static Mode get_current_mode() { return current_mode; }
   static void set_mode(Mode mode);

   // UI state
   static bool is_editing() { return current_mode == Mode::PARAMETER_EDIT; }
   static bool is_in_menu() { return current_mode == Mode::MENU; }
   static const Parameter* get_current_parameter() { return current_parameter; }

   // Display updates
   static void refresh_display();
   static void update_parameter_display();
   static void show_message(const char* line1, const char* line2 = nullptr);
   static void update_parameter_value(int16_t change);
   static void update_parameter_value(const Parameter* param, uint8_t value);

private:
   // State
   static Mode current_mode;
   static MenuItem current_menu_item;
   static const Parameter* current_parameter;
   static uint32_t last_button_time;
   static bool display_needs_update;

   // Display functions
   static void update_display();
   static void update_normal_display();
   static void update_menu_display();
   static void update_parameter_edit_display();
   static void update_system_config_display();

   // Mode updates
   static void update_normal_mode();
   static void update_menu_mode();
   static void update_parameter_edit_mode();  
   static void update_system_config_mode();

   // Navigation
   static void next_menu_item();
   static void prev_menu_item();
   static void execute_menu_item();
   static void next_parameter();
   static void prev_parameter();

   // Button mapping
   static void map_normal_mode_buttons(uint8_t button);
   static void map_menu_mode_buttons(uint8_t button);
   static void map_parameter_edit_buttons(uint8_t button);
   static void map_system_config_buttons(uint8_t button);
};

} // namespace ui
} // namespace pg1000
