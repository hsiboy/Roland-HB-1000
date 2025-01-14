#pragma once

#include <cstdint>
#include <string_view>

namespace pg1000 {
namespace hardware {

class Display {
public:
    static constexpr uint8_t COLS = 16;
    static constexpr uint8_t ROWS = 2;

    // Initialize display
    static bool init();

    // Basic display control
    static void clear();
    static void set_cursor(uint8_t col, uint8_t row);
    static void print(std::string_view str);
    static void print_at(uint8_t col, uint8_t row, std::string_view str);

    // Specialized display functions
    static void show_parameter(std::string_view name, uint8_t value, uint8_t max_value);
    static void show_message(std::string_view line1, std::string_view line2 = "");
    static void show_progress_bar(uint8_t value, uint8_t max_value);

    // Custom character support
    static void create_custom_char(uint8_t location, const uint8_t* char_map);

private:
    static constexpr uint8_t I2C_ADDR = 0x27;
    static constexpr uint8_t I2C_PORT = 1;  // i2c1

    // LCD commands
    static constexpr uint8_t LCD_CLEARDISPLAY = 0x01;
    static constexpr uint8_t LCD_RETURNHOME = 0x02;
    static constexpr uint8_t LCD_ENTRYMODESET = 0x04;
    static constexpr uint8_t LCD_DISPLAYCONTROL = 0x08;
    static constexpr uint8_t LCD_FUNCTIONSET = 0x20;
    static constexpr uint8_t LCD_SETCGRAMADDR = 0x40;
    static constexpr uint8_t LCD_SETDDRAMADDR = 0x80;

    // LCD flags
    static constexpr uint8_t LCD_DISPLAY_ON = 0x04;
    static constexpr uint8_t LCD_BACKLIGHT = 0x08;
    static constexpr uint8_t LCD_ENABLE = 0x04;
    static constexpr uint8_t LCD_REGISTER_SELECT = 0x01;

    // Custom characters
    static constexpr uint8_t CUSTOM_CHAR_FULL = 0;
    static constexpr uint8_t CUSTOM_CHAR_HALF = 1;

    // Internal state
    static uint8_t backlight_state;
    static char display_buffer[ROWS][COLS + 1];
    static uint32_t last_update;

    // Low-level functions
    static void write_command(uint8_t cmd);
    static void write_data(uint8_t data);
    static void write_4bits(uint8_t value);
    static void pulse_enable(uint8_t value);
    static void send(uint8_t value, bool is_command);
    
    // Buffer management
    static void update_buffer();
    static bool needs_update(uint8_t row, std::string_view new_text);
};

} // namespace hardware
} // namespace pg1000
