#pragma once

#include <cstdint>
#include <vector>
#include "../parameters/parameters.h"

namespace pg1000 {
namespace midi {

// MIDI Constants
constexpr uint8_t MIDI_CHANNEL = 1;  // Default MIDI channel
constexpr uint8_t ROLAND_ID = 0x41;  // Roland manufacturer ID
constexpr uint8_t D50_ID = 0x14;     // D50 device ID

// MIDI Message Types
enum class MessageType {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    POLY_PRESSURE = 0xA0,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    CHANNEL_PRESSURE = 0xD0,
    PITCH_BEND = 0xE0,
    SYSTEM_EXCLUSIVE = 0xF0
};

// MIDI Interface Class
class MIDI {
public:
    // Initialize MIDI system
    static bool init();

    // Send MIDI messages
    static void send_cc(uint8_t cc, uint8_t value);
    static void send_sysex(const Parameter* param);
    static void send_program_change(uint8_t program);

    // Handle incoming MIDI
    static void process_incoming();
    static void set_midi_channel(uint8_t channel);

    // Enable/disable different message types
    static void enable_sysex(bool enable) { sysex_enabled = enable; }
    static void enable_cc(bool enable) { cc_enabled = enable; }

private:
    static constexpr uint8_t UART_TX = 0;
    static constexpr uint8_t UART_RX = 1;
    static constexpr uint32_t MIDI_BAUD = 31250;

    // Message assembly buffer
    static constexpr size_t MAX_SYSEX_SIZE = 64;
    static std::vector<uint8_t> sysex_buffer;
    static bool in_sysex;
    
    // Configuration
    static uint8_t midi_channel;
    static bool sysex_enabled;
    static bool cc_enabled;

    // Internal functions
    static void handle_sysex();
    static void send_bytes(const uint8_t* data, size_t length);
};

} // namespace midi
} // namespace pg1000
