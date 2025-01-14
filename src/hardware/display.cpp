#include "display.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include <cstring>
#include <algorithm>
#include <cstdio>    // for snprintf
#include "hardware/gpio.h"  // for GPIO_FUNC_I2C

namespace pg1000 {
namespace hardware {

// Static member initialization
uint8_t Display::backlight_state = LCD_BACKLIGHT;
char Display::display_buffer[ROWS][COLS + 1] = {{0}};
uint32_t Display::last_update = 0;

// Custom character definitions
const uint8_t CUSTOM_CHAR_FULL_DATA[] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
const uint8_t CUSTOM_CHAR_HALF_DATA[] = {0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};

bool Display::init() {
    // Initialize I2C for display
    i2c_init(i2c1, 100000);  // 100kHz
    gpio_set_function(6, GPIO_FUNC_I2C);  // SDA
    gpio_set_function(7, GPIO_FUNC_I2C);  // SCL

    // Wait for LCD to power up
    sleep_ms(50);

    // Initialize in 4-bit mode
    write_4bits(0x03 << 4);
    sleep_ms(5);
    write_4bits(0x03 << 4);
    sleep_ms(5);
    write_4bits(0x03 << 4);
    sleep_us(150);
    write_4bits(0x02 << 4);

    // Set up the LCD
    write_command(LCD_FUNCTIONSET | 0x08);  // 2-line display
    write_command(LCD_DISPLAYCONTROL | LCD_DISPLAY_ON);  // Display on
    write_command(LCD_CLEARDISPLAY);  // Clear display
    sleep_ms(2);
    write_command(LCD_ENTRYMODESET | 0x02);  // Increment cursor

    // Create custom characters
    create_custom_char(CUSTOM_CHAR_FULL, CUSTOM_CHAR_FULL_DATA);
    create_custom_char(CUSTOM_CHAR_HALF, CUSTOM_CHAR_HALF_DATA);

    clear();
    return true;
}

void Display::clear() {
    write_command(LCD_CLEARDISPLAY);
    sleep_ms(2);
    memset(display_buffer, ' ', sizeof(display_buffer));
    for(uint8_t i = 0; i < ROWS; i++) {
        display_buffer[i][COLS] = '\0';
    }
}

void Display::set_cursor(uint8_t col, uint8_t row) {
    const uint8_t row_offsets[] = {0x00, 0x40};
    write_command(LCD_SETDDRAMADDR | (col + row_offsets[row & 0x01]));
}

void Display::print(std::string_view str) {
    for(char c : str) {
        write_data(c);
    }
}

void Display::print_at(uint8_t col, uint8_t row, std::string_view str) {
    if(row >= ROWS) return;
    
    // Check if update is needed
    if(!needs_update(row, str)) return;
    
    set_cursor(col, row);
    size_t len = std::min(str.length(), static_cast<size_t>(COLS - col));
    
    // Update buffer
    memcpy(&display_buffer[row][col], str.data(), len);
    print(str.substr(0, len));
}

void Display::show_parameter(std::string_view name, uint8_t value, uint8_t max_value) {
    // First line: Parameter name
    print_at(0, 0, name);
    
    // Second line: Value and progress bar
    char value_str[6];
    snprintf(value_str, sizeof(value_str), "%3d", value);
    print_at(0, 1, value_str);
    
    // Progress bar
    show_progress_bar(value, max_value);
}

void Display::show_message(std::string_view line1, std::string_view line2) {
    print_at(0, 0, line1);
    if(line2.empty()) {
        // Clear second line if no text provided
        print_at(0, 1, std::string_view("                ", 16));
    } else {
        print_at(0, 1, line2);
    }
}

void Display::show_progress_bar(uint8_t value, uint8_t max_value) {
    const uint8_t BAR_START = 4;  // Start after value display
    const uint8_t BAR_LENGTH = COLS - BAR_START;
    
    float percentage = static_cast<float>(value) / max_value;
    uint8_t filled_chars = static_cast<uint8_t>(percentage * BAR_LENGTH);
    
    set_cursor(BAR_START, 1);
    
    // Draw filled section
    for(uint8_t i = 0; i < filled_chars; i++) {
        write_data(CUSTOM_CHAR_FULL);
    }
    
    // Draw empty section
    for(uint8_t i = filled_chars; i < BAR_LENGTH; i++) {
        write_data(' ');
    }
}

void Display::create_custom_char(uint8_t location, const uint8_t* char_map) {
    location &= 0x7;  // Only 8 custom characters allowed
    write_command(LCD_SETCGRAMADDR | (location << 3));
    for(int i = 0; i < 8; i++) {
        write_data(char_map[i]);
    }
}

void Display::write_command(uint8_t cmd) {
    send(cmd, true);
}

void Display::write_data(uint8_t data) {
    send(data, false);
}

void Display::write_4bits(uint8_t value) {
    pulse_enable(value);
}

void Display::pulse_enable(uint8_t value) {
    uint8_t data = value | backlight_state;
    
    // Enable pulse must be at least 450ns wide
    uint8_t en = data | LCD_ENABLE;
    uint8_t buf[] = {0x00, en};
    i2c_write_blocking(i2c1, I2C_ADDR, buf, 2, false);
    sleep_us(1);
    
    buf[1] = data & ~LCD_ENABLE;
    i2c_write_blocking(i2c1, I2C_ADDR, buf, 2, false);
    sleep_us(50);  // Commands need >37us to settle
}

void Display::send(uint8_t value, bool is_command) {
    uint8_t high = value & 0xF0;
    uint8_t low = (value << 4) & 0xF0;
    
    if(!is_command) {
        high |= LCD_REGISTER_SELECT;
        low |= LCD_REGISTER_SELECT;
    }
    
    write_4bits(high);
    write_4bits(low);
}

bool Display::needs_update(uint8_t row, std::string_view new_text) {
    if(row >= ROWS) return false;
    
    // Compare with current buffer
    size_t len = std::min(new_text.length(), static_cast<size_t>(COLS));
    return memcmp(display_buffer[row], new_text.data(), len) != 0;
}

} // namespace hardware
} // namespace pg1000
