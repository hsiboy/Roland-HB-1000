#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/hardware.h"
#include "hardware/display.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "midi/midi.h"
#include "parameters/parameters.h"
#include "ui/interface.h"

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

    // Initialize MIDI
    if (!midi::MIDI::init()) {
        printf("MIDI initialization failed\n");
        hardware::Display::show_message("Error:", "MIDI Init Failed");
        return -1;
    }

    // Initialize UI
    if (!ui::Interface::init()) {
        printf("UI initialization failed\n");
        return -1;
    }

    printf("System initialized and ready\n");

    // Main loop
    while (true) {
        // Update hardware state
        hardware::ADC::read_all();  // Read all potentiometers
        hardware::GPIO::update();    // Update button states and LEDs

        // Update UI
        ui::Interface::update();     // Handle UI logic

        // Process MIDI
        midi::MIDI::process_incoming();

        // Small delay to prevent overwhelming the system
        sleep_us(100);
    }

    return 0;
}
