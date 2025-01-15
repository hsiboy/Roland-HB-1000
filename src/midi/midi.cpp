#include "midi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

namespace pg1000 {
namespace midi {

// Static member initialization
uint8_t MIDI::midi_channel = MIDI_CHANNEL;
bool MIDI::sysex_enabled = true;
bool MIDI::cc_enabled = true;
std::vector<uint8_t> MIDI::sysex_buffer;
bool MIDI::in_sysex = false;

static void on_uart_rx() {
    while (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        MIDI::process_incoming();
    }
}

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

void MIDI::send_cc(uint8_t cc, uint8_t value) {
    if (!cc_enabled) return;

    uint8_t status = static_cast<uint8_t>(MessageType::CONTROL_CHANGE) | (midi_channel - 1);
    uint8_t data[] = {
        status,
        static_cast<uint8_t>(cc & 0x7F),
        static_cast<uint8_t>(value & 0x7F)
    };
    
    send_bytes(data, sizeof(data));
}

void MIDI::send_sysex(const Parameter* param) {
    if (!sysex_enabled || !param) return;

    SysExAddress addr = SysEx::get_parameter_address(param);
    uint8_t sysex[] = {
        static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE),
        ROLAND_ID,
        0x10,          // Device ID
        D50_ID,
        0x12,          // Command ID (DT1)
        addr.msb,      // Address MSB
        addr.mid,      // Address middle byte
        addr.lsb,      // Address LSB
        param->value,  // Parameter value
        0x00,
        0xF7
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

void MIDI::process_incoming() {
    while (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        
        // Handle SysEx
        if (byte == static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE)) {
            sysex_buffer.clear();
            sysex_buffer.push_back(byte);
            in_sysex = true;
            continue;
        }
        
        // Process SysEx data
        if (in_sysex) {
            sysex_buffer.push_back(byte);
            
            // Check for end of SysEx
            if (byte == 0xF7) {
                in_sysex = false;
                handle_sysex();
            }
            
            // Check for buffer overflow
            if (sysex_buffer.size() >= MAX_SYSEX_SIZE) {
                in_sysex = false;
                sysex_buffer.clear();
            }
            continue;
        }
    }
}

void MIDI::send_bytes(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uart_putc(uart0, data[i]);
    }
}

void MIDI::handle_sysex() {
    if (sysex_buffer.size() < 11) return;  // Minimum size for D50 message
    
    // Verify Roland message
    if (sysex_buffer[1] != ROLAND_ID || 
        sysex_buffer[3] != D50_ID) return;
        
    // Process message based on command
    switch (sysex_buffer[4]) {
        case 0x11:  // RQ1 - Data request
            break;
            
        case 0x12:  // DT1 - Data set
            // TODO: Update parameter based on address
            break;
    }
}

} // namespace midi
} // namespace pg1000
