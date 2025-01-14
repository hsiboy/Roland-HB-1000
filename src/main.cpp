#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/hardware.h"
#include "hardware/display.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "midi/midi.h"
#include "parameters/parameters.h"

using namespace pg1000;

int main() {
    // Initialize stdio (for debug output)
    stdio_init_all();
    printf("\nPG-1000 Controller Starting...\n");

    // Initialize all hardware
    auto init_result = hardware::Hardware::init_all();
    if (init_result.status != hardware::HardwareStatus::OK) {
        printf("Hardware initialization failed: %d\n", static_cast<int>(init_result.status));
        hardware::Hardware::print_status();
        return -1;
    }

    printf("Hardware initialized successfully\n");
    hardware::Hardware::print_status();

    // Initialize display with welcome message
    hardware::Display::init();
    hardware::Display::show_message("PG-1000", "Initializing...");

    // Initialize MIDI
    if (!midi::MIDI::init()) {
        printf("MIDI initialization failed\n");
        hardware::Display::show_message("Error:", "MIDI Init Failed");
        return -1;
    }

    printf("MIDI initialized\n");

    // Main loop variables
    uint32_t last_display_update = 0;
    uint32_t last_led_update = 0;
    bool needs_display_update = true;

    // Main loop
    while (true) {
        uint32_t now = time_us_32();

        // Read all inputs
        hardware::GPIO::update();  // Update buttons and LEDs
        hardware::ADC::read_all(); // Read all potentiometers

        // Process buttons
        for (uint8_t i = 0; i < hardware::GPIO::NUM_BUTTONS; i++) {
            if (hardware::GPIO::get_button_pressed(i)) {
                // Handle button press
                printf("Button %d pressed\n", i);
                needs_display_update = true;
            }
        }

        // Update display every 50ms if needed
        if (needs_display_update && (now - last_display_update > 50000)) {
            const Parameter* current_param = get_parameter(0);  // Get currently selected parameter
            if (current_param) {
                hardware::Display::show_parameter(
                    current_param->name, 
                    current_param->value,
                    current_param->max_value
                );
            }
            last_display_update = now;
            needs_display_update = false;
        }

        // Update LEDs every 100ms
        if (now - last_led_update > 100000) {
            // Update LED states based on current mode
            hardware::GPIO::update();
            last_led_update = now;
        }

        // Process incoming MIDI
        midi::MIDI::process_incoming();

        // Small delay to prevent overwhelming the system
        sleep_us(100);
    }

    return 0;
}
