#include "midi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

namespace pg1000 {
namespace midi {

// Static member initialization
std::vector<uint8_t> MIDI::sysex_buffer;
bool MIDI::in_sysex = false;
uint8_t MIDI::midi_channel = MIDI_CHANNEL;
bool MIDI::sysex_enabled = true;
bool MIDI::cc_enabled = true;

// Forward declare the UART interrupt handler
static void on_uart_rx();

bool MIDI::init() {
    // Initialize UART for MIDI
    uart_init(uart0, MIDI_BAUD);
    gpio_set_function(UART_TX, GPIO_FUNC_UART);
    gpio_set_function(UART_RX, GPIO_FUNC_UART);

    // Setup UART interrupt
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);

    // Initialize SysEx buffer
    sysex_buffer.reserve(MAX_SYSEX_SIZE);
    
    return true;
}

// UART interrupt handler implementation
static void on_uart_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        MIDI::process_incoming();
    }
}

void MIDI::send_cc(uint8_t cc, uint8_t value) {
    if (!cc_enabled) return;

    uint8_t status = static_cast<uint8_t>(MessageType::CONTROL_CHANGE) | (midi_channel - 1);
    uint8_t data[] = {
        status,
        static_cast<uint8_t>(cc & 0x7F),        // CC number (0-127)
        static_cast<uint8_t>(value & 0x7F)      // Value (0-127)
    };
    
    send_bytes(data, sizeof(data));
}

void MIDI::send_sysex(const Parameter* param) {
    if (!sysex_enabled || !param) return;

    uint8_t sysex[] = {
        static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE),
        ROLAND_ID,      // Roland ID
        0x10,          // Device ID
        D50_ID,        // D50 ID
        0x12,          // Command ID (DT1)
        0x00,          // Address MSB
        static_cast<uint8_t>((param->id >> 8) & 0xFF),  // Parameter ID MSB
        static_cast<uint8_t>(param->id & 0xFF),         // Parameter ID LSB
        param->value,   // Parameter value
        0x00,          // Not used
        0xF7           // End of SysEx
    };
    
    send_bytes(sysex, sizeof(sysex));
}

void MIDI::send_program_change(uint8_t program) {
    uint8_t status = static_cast<uint8_t>(MessageType::PROGRAM_CHANGE) | (midi_channel - 1);
    uint8_t data[] = {
        status,
        static_cast<uint8_t>(program & 0x7F)
    };
    
    send_bytes(data, sizeof(data));
}

// [Rest of the implementation remains the same]

} // namespace midi
} // namespace pg1000
