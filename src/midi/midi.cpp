#include "midi.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <algorithm>

namespace pg1000 {
namespace midi {

// Static member initialization
std::vector<uint8_t> MIDI::sysex_buffer;
bool MIDI::in_sysex = false;
uint8_t MIDI::midi_channel = MIDI_CHANNEL;
bool MIDI::sysex_enabled = true;
bool MIDI::cc_enabled = true;

// UART interrupt handler for MIDI input
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
        cc & 0x7F,        // CC number (0-127)
        value & 0x7F      // Value (0-127)
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
        program & 0x7F
    };
    
    send_bytes(data, sizeof(data));
}

void MIDI::process_incoming() {
    while (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        
        // Check for System Exclusive messages
        if (byte == static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE)) {
            sysex_buffer.clear();
            sysex_buffer.push_back(byte);
            in_sysex = true;
            continue;
        }
        
        // Handle SysEx data
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
        
        // Handle other MIDI messages
        // Status byte
        if (byte & 0x80) {
            MessageType type = static_cast<MessageType>(byte & 0xF0);
            uint8_t channel = (byte & 0x0F) + 1;
            
            // Only process messages for our channel
            if (channel != midi_channel) continue;
            
            // Handle different message types
            switch (type) {
                case MessageType::CONTROL_CHANGE:
                    // CC messages will be handled in the next two bytes
                    break;
                    
                case MessageType::PROGRAM_CHANGE:
                    // Program change messages have one data byte
                    break;
                    
                default:
                    // Ignore other message types
                    break;
            }
        }
    }
}

void MIDI::set_midi_channel(uint8_t channel) {
    midi_channel = std::clamp(channel, uint8_t(1), uint8_t(16));
}

void MIDI::handle_sysex() {
    // Check if this is a D50 message
    if (sysex_buffer.size() < 11) return;  // Minimum size for D50 parameter message
    if (sysex_buffer[1] != ROLAND_ID) return;  // Not Roland
    if (sysex_buffer[3] != D50_ID) return;    // Not D50

    // Extract parameter data
    uint16_t param_id = (sysex_buffer[6] << 8) | sysex_buffer[7];
    uint8_t value = sysex_buffer[8];

    // Find and update parameter
    auto* param = get_parameter_by_id(param_id);
    if (param) {
        update_parameter_value(param, value);
    }
}

void MIDI::send_bytes(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uart_putc(uart0, data[i]);
    }
}

} // namespace midi
} // namespace pg1000
