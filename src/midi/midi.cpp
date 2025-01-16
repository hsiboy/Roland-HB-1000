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
uint32_t MIDI::min_update_interval = MIN_UPDATE_INTERVAL;
std::array<hardware::ValueSmoother<4>, MAX_PARAMETERS> MIDI::parameter_smoothers;
std::array<std::chrono::steady_clock::time_point, MAX_PARAMETERS> MIDI::last_update_time;

const char* MIDI::get_error_string(MidiError error) {
    switch (error) {
        case MidiError::OK: return "OK";
        case MidiError::INVALID_CHANNEL: return "Invalid MIDI channel";
        case MidiError::INVALID_PARAMETER: return "Invalid parameter";
        case MidiError::INVALID_VALUE: return "Invalid value";
        case MidiError::BUFFER_OVERFLOW: return "Buffer overflow";
        case MidiError::CHECKSUM_ERROR: return "Checksum error";
        case MidiError::MALFORMED_MESSAGE: return "Malformed message";
        default: return "Unknown error";
    }
}

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

    // Initialize parameter update timestamps
    auto now = std::chrono::steady_clock::now();
    for (auto& time : last_update_time) {
        time = now;
    }
    
    return true;
}

MidiError MIDI::send_cc(uint8_t cc, uint8_t value) {
    if (!cc_enabled) return MidiError::OK;
    if (cc >= MAX_PARAMETERS) return MidiError::INVALID_PARAMETER;
    if (value > 127) return MidiError::INVALID_VALUE;

    // Apply value smoothing
    uint16_t smoothed_value = parameter_smoothers[cc].update(value);
    
    // Check if we should send an update
    if (!should_update_parameter(cc)) {
        return MidiError::OK;
    }

    uint8_t status = static_cast<uint8_t>(MessageType::CONTROL_CHANGE) | (midi_channel - 1);
    uint8_t data[] = {
        status,
        static_cast<uint8_t>(cc & 0x7F),
        static_cast<uint8_t>(smoothed_value & 0x7F)
    };
    
    return send_bytes(data, sizeof(data));
}

MidiError MIDI::send_sysex(const Parameter* param) {
    if (!sysex_enabled || !param) return MidiError::INVALID_PARAMETER;

    // Get parameter address
    SysExAddress addr = SysEx::get_parameter_address(param);
    
    // Apply value smoothing
    uint16_t smoothed_value = parameter_smoothers[param->id].update(param->value);
    
    // Check if we should send an update
    if (!should_update_parameter(param->id)) {
        return MidiError::OK;
    }

    // Prepare SysEx message
    std::vector<uint8_t> sysex = {
        static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE),
        ROLAND_ID,
        static_cast<uint8_t>(midi_channel - 1),  // Device ID
        D50_ID,
        DT1_COMMAND,
        addr.msb,      // Address MSB
        addr.mid,      // Address middle byte
        addr.lsb,      // Address LSB
        static_cast<uint8_t>(smoothed_value & 0x7F)  // Parameter value
    };
    
    // Calculate checksum
    sysex.push_back(calculate_checksum(sysex.data() + 5, 4));  // Start from address bytes
    sysex.push_back(0xF7);  // End of SysEx
    
    return send_bytes(sysex.data(), sysex.size());
}

MidiError MIDI::send_program_change(uint8_t program) {
    if (program > 127) return MidiError::INVALID_VALUE;

    uint8_t status = static_cast<uint8_t>(MessageType::PROGRAM_CHANGE) | (midi_channel - 1);
    uint8_t data[] = {
        status,
        static_cast<uint8_t>(program & 0x7F)
    };
    
    return send_bytes(data, sizeof(data));
}

MidiError MIDI::request_parameter(const Parameter* param) {
    if (!param) return MidiError::INVALID_PARAMETER;

    SysExAddress addr = SysEx::get_parameter_address(param);
    std::vector<uint8_t> sysex = {
        static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE),
        ROLAND_ID,
        static_cast<uint8_t>(midi_channel - 1),
        D50_ID,
        RQ1_COMMAND,
        addr.msb,
        addr.mid,
        addr.lsb,
        0x00,  // Size MSB
        0x00,  // Size LSB
        0x01   // Size (1 byte)
    };
    
    sysex.push_back(calculate_checksum(sysex.data() + 5, 6));
    sysex.push_back(0xF7);
    
    return send_bytes(sysex.data(), sysex.size());
}

MidiError MIDI::request_all_parameters() {
    std::vector<uint8_t> sysex = {
        static_cast<uint8_t>(MessageType::SYSTEM_EXCLUSIVE),
        ROLAND_ID,
        static_cast<uint8_t>(midi_channel - 1),
        D50_ID,
        RQ1_COMMAND,
        0x00, 0x00, 0x00,  // Start address
        0x00, 0x03, 0x25   // Size (421 bytes)
    };
    
    sysex.push_back(calculate_checksum(sysex.data() + 5, 6));
    sysex.push_back(0xF7);
    
    return send_bytes(sysex.data(), sysex.size());
}

void MIDI::process_incoming() {
    while (uart_is_readable(uart0)) {
        uint8_t byte = uart_getc(uart0);
        
        // Handle real-time messages immediately
        if (byte >= static_cast<uint8_t>(MessageType::TIMING_CLOCK)) {
            handle_realtime_message(static_cast<MessageType>(byte));
            continue;
        }
        
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
                if (verify_checksum(sysex_buffer)) {
                    handle_sysex();
                }
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

MidiError MIDI::send_bytes(const uint8_t* data, size_t length) {
    if (!data) return MidiError::INVALID_PARAMETER;
    
    for (size_t i = 0; i < length; i++) {
        uart_putc(uart0, data[i]);
    }
    
    return MidiError::OK;
}

void MIDI::handle_sysex() {
    if (sysex_buffer.size() < 11) return;  // Minimum size for D50 message
    
    // Verify Roland message
    if (sysex_buffer[1] != ROLAND_ID || 
        sysex_buffer[3] != D50_ID) return;
        
    // Process message based on command
    switch (sysex_buffer[4]) {
        case RQ1_COMMAND:  // RQ1 - Data request
            // Handle parameter request response
            break;
            
        case DT1_COMMAND:  // DT1 - Data set
            if (sysex_buffer.size() < 13) return;  // Need at least address + value + checksum
            
            // Extract address and value
            SysExAddress addr = {
                sysex_buffer[5],
                sysex_buffer[6],
                sysex_buffer[7]
            };
            
            uint8_t value = sysex_buffer[8];
            
            // Update parameter based on address
            // TODO: Implement parameter update logic
            break;
    }
}

uint8_t MIDI::calculate_checksum(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum = (sum + data[i]) & 0x7F;
    }
    return (128 - sum) & 0x7F;
}

bool MIDI::verify_checksum(const std::vector<uint8_t>& message) {
    if (message.size() < 3) return false;  // Need at least command + checksum + EOX
    
    // Calculate checksum from command ID to last data byte
    size_t data_end = message.size() - 2;  // Exclude checksum and EOX
    uint8_t sum = 0;
    
    for (size_t i = 5; i < data_end; i++) {  // Start from command ID
        sum = (sum + message[i]) & 0x7F;
    }
    
    sum = (sum + message[data_end]) & 0x7F;  // Add checksum
    return sum == 0;  // Valid checksum should result in 0
}

bool MIDI::should_update_parameter(uint8_t parameter_index) {
    if (parameter_index >= MAX_PARAMETERS) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        now - last_update_time[parameter_index]).count();
        
    if (elapsed >= min_update_interval) {
        last_update_time[parameter_index] = now;
        return true;
    }
    
    return false;
}


} // namespace midi
} // namespace pg1000
