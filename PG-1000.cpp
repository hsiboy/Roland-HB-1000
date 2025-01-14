#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"

// LCD Configuration (I2C)
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// MCP23017 Configuration
#define MCP23017_ADDR 0x20
#define IODIRA 0x00
#define IODIRB 0x01
#define GPPUA  0x0C
#define GPPUB  0x0D
#define GPIOA  0x12
#define GPIOB  0x13

// LED States
enum LedState {
    LED_OFF,
    LED_ON,
    LED_BLINK_SLOW,
    LED_BLINK_FAST
};

// LED Definitions
struct Led {
    uint8_t bit;
    LedState state;
    uint32_t last_toggle;
    const char* name;
};

Led leds[] = {
    {0, LED_OFF, 0, "UPPER"},
    {1, LED_OFF, 0, "LOWER"},
    {2, LED_OFF, 0, "PARTIAL"},
    {3, LED_OFF, 0, "MIDI"},
    {4, LED_OFF, 0, "WRITE"},
    {5, LED_OFF, 0, "ERROR"}
};

const int NUM_LEDS = sizeof(leds) / sizeof(Led);

// Button Definitions
struct Button {
    uint8_t bit;
    bool state;
    bool prev_state;
    uint32_t last_debounce;
    const char* name;
};

Button buttons[] = {
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
};

const int NUM_BUTTONS = sizeof(buttons) / sizeof(Button);

// Enhanced value smoothing with exponential filter
struct ValueFilter {
    float current;
    float alpha;  // Filter coefficient (0-1)
};

ValueFilter pot_filters[NUM_PARAMETERS];

// Display state
struct DisplayState {
    char line1[LCD_COLS + 1];
    char line2[LCD_COLS + 1];
    uint32_t last_update;
    bool needs_update;
} display = {0};

// Enhanced MIDI handling with CC output
struct MidiState {
    bool sysex_active;
    bool cc_active;
    uint8_t channel;
    uint32_t last_cc_time;
    uint32_t last_sysex_time;
} midi = {
    .sysex_active = true,
    .cc_active = true,
    .channel = 1,
    .last_cc_time = 0,
    .last_sysex_time = 0
};

// Function prototypes
void init_lcd(void);
void lcd_write(uint8_t value, bool is_command);
void lcd_print(const char* str);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_clear(void);
void update_display(void);
void handle_leds(void);
void process_buttons(void);
void send_cc_message(uint8_t cc, uint8_t value);
float filter_value(ValueFilter* filter, float new_value);

// LCD Implementation
void init_lcd() {
    // Initialize LCD in 4-bit mode
    sleep_ms(50);  // Wait for LCD to power up
    
    // Standard LCD initialization sequence
    lcd_write(0x03, true);
    sleep_ms(5);
    lcd_write(0x03, true);
    sleep_ms(5);
    lcd_write(0x03, true);
    sleep_ms(1);
    lcd_write(0x02, true);
    
    // Set 4-bit mode, 2 lines, 5x8 font
    lcd_write(0x28, true);
    // Display on, cursor off, blink off
    lcd_write(0x0C, true);
    // Clear display
    lcd_write(0x01, true);
    sleep_ms(2);
    // Entry mode set
    lcd_write(0x06, true);
}

void lcd_write(uint8_t value, bool is_command) {
    uint8_t high = value & 0xF0;
    uint8_t low = (value << 4) & 0xF0;
    
    uint8_t data[4];
    // High nibble
    data[0] = high | (is_command ? 0x00 : 0x01);  // RS bit
    data[1] = data[0] | 0x04;  // Set E bit
    data[2] = data[0];         // Clear E bit
    // Low nibble
    data[3] = low | (is_command ? 0x00 : 0x01);   // RS bit
    
    i2c_write_blocking(i2c1, LCD_ADDR, data, 4, false);
    if (is_command && value <= 3) {
        sleep_ms(2);  // Commands need more time
    }
}

void update_display() {
    static uint32_t last_update = 0;
    uint32_t now = time_us_32();
    
    // Update display at most every 50ms
    if (now - last_update < 50000) return;
    last_update = now;
    
    // Get current parameter
    Parameter* current_param = NULL;
    for (int i = 0; i < NUM_PARAMETERS; i++) {
        if (parameters[i].active && parameters[i].group == state.current_group) {
            current_param = &parameters[i];
            break;
        }
    }
    
    if (current_param) {
        // Format first line: Parameter name
        snprintf(display.line1, LCD_COLS + 1, "%-16s", current_param->name);
        
        // Format second line: Value and bar graph
        int value_chars = snprintf(display.line2, 6, "%3d", current_param->value);
        int bar_length = (current_param->value * (LCD_COLS - value_chars)) / current_param->max_value;
        
        // Fill rest with bar graph
        for (int i = value_chars; i < LCD_COLS; i++) {
            display.line2[i] = i < (value_chars + bar_length) ? 0xFF : ' ';
        }
        display.line2[LCD_COLS] = '\0';
        
        // Update LCD
        lcd_set_cursor(0, 0);
        lcd_print(display.line1);
        lcd_set_cursor(0, 1);
        lcd_print(display.line2);
    }
}

void handle_leds() {
    static uint32_t last_update = 0;
    uint32_t now = time_us_32();
    
    // Update LEDs every 50ms
    if (now - last_update < 50000) return;
    last_update = now;
    
    uint8_t led_state = 0;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        bool should_be_on = false;
        
        switch (leds[i].state) {
            case LED_ON:
                should_be_on = true;
                break;
            case LED_BLINK_SLOW:
                should_be_on = (now / 500000) % 2;
                break;
            case LED_BLINK_FAST:
                should_be_on = (now / 100000) % 2;
                break;
            default:
                should_be_on = false;
        }
        
        if (should_be_on) {
            led_state |= (1 << leds[i].bit);
        }
    }
    
    // Update MCP23017 LED outputs
    uint8_t buf[] = {GPIOB, led_state};
    i2c_write_blocking(i2c0, MCP23017_ADDR, buf, 2, false);
}

void process_buttons() {
    static uint32_t last_read = 0;
    uint32_t now = time_us_32();
    
    // Read buttons every 1ms
    if (now - last_read < 1000) return;
    last_read = now;
    
    // Read MCP23017 button inputs
    uint8_t reg = GPIOA;
    uint8_t button_states;
    i2c_write_blocking(i2c0, MCP23017_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, MCP23017_ADDR, &button_states, 1, false);
    
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool current_state = !(button_states & (1 << buttons[i].bit));
        
        if (current_state != buttons[i].prev_state) {
            buttons[i].last_debounce = now;
        }
        
        if ((now - buttons[i].last_debounce) > 5000) {  // 5ms debounce
            if (current_state != buttons[i].state) {
                buttons[i].state = current_state;
                if (current_state) {  // Button pressed
                    handle_button_press(i);
                }
            }
        }
        
        buttons[i].prev_state = current_state;
    }
}

void send_cc_message(uint8_t cc, uint8_t value) {
    if (!midi.cc_active) return;
    
    uint8_t status = 0xB0 | (midi.channel - 1);
    uart_putc(MIDI_UART, status);
    uart_putc(MIDI_UART, cc);
    uart_putc(MIDI_UART, value);
    
    midi.last_cc_time = time_us_32();
}

float filter_value(ValueFilter* filter, float new_value) {
    filter->current = filter->current + filter->alpha * (new_value - filter->current);
    return filter->current;
}

void process_parameter_change(Parameter* param, uint16_t new_value) {
    // Apply exponential filter
    float filtered = filter_value(&pot_filters[param->pot_number], new_value);
    
    // Scale to parameter range
    uint8_t scaled_value;
    if (param->type == CONTINUOUS_100) {
        scaled_value = (filtered * 100) / 1023;
    } else if (param->type == CONTINUOUS_50) {
        scaled_value = (filtered * 50) / 1023;
    } else {
        scaled_value = filtered > 512 ? 1 : 0;
    }
    
    // Check if value has changed enough to send
    if (abs(scaled_value - param->value) > 1) {
        param->prev_value = param->value;
        param->value = scaled_value;
        
        // Send both SysEx and CC if enabled
        if (midi.sysex_active) {
            send_sysex(param);
        }
        if (midi.cc_active) {
            // Map parameter value to CC range (0-127)
            uint8_t cc_value = (scaled_value * 127) / param->max_value;
            send_cc_message(param->cc_number, cc_value);
        }
        
        display.needs_update = true;
    }
}

int main() {
    // Initialize system
    stdio_init_all();
    init_hardware();
    init_lcd();
    
    // Initialize value filters
    for (int i = 0; i < NUM_PARAMETERS; i++) {
        pot_filters[i].current = 0;
        pot_filters[i].alpha = 0.3f;  // Adjust filter coefficient as needed
    }
    
    // Main loop
    while (1) {
        read_controls();
        process_buttons();
        handle_leds();
        
        if (display.needs_update) {
            update_display();
            display.needs_update = false;
        }
        
        sleep_us(100);  // Small delay for system stability
    }
}
