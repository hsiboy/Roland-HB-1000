#pragma once

#include <cstdint>
#include <vector>
#include "sysex.h"
#include "../parameters/parameters.h"


namespace pg1000 {
namespace midi {

// MIDI Constants
static constexpr uint8_t MIDI_CHANNEL = 1;    // Default channel
static constexpr uint32_t MIDI_BAUD = 31250;  // MIDI baud rate
static constexpr uint8_t UART_TX = 0;         // UART TX pin
static constexpr uint8_t UART_RX = 1;         // UART RX pin
static constexpr size_t MAX_SYSEX_SIZE = 64;  // Maximum SysEx message size

// MIDI Message Types
enum class MessageType : uint8_t {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    SYSTEM_EXCLUSIVE = 0xF0
};

// Roland Constants
static constexpr uint8_t ROLAND_ID = 0x41;
static constexpr uint8_t D50_ID = 0x14;


class MIDI {
public:
    // Initialize MIDI
    static bool init();

    // MIDI message sending
    static void send_cc(uint8_t cc, uint8_t value);
    static void send_sysex(const Parameter* param);
    static void send_program_change(uint8_t program);

    // MIDI message receiving
    static void process_incoming();
    
    // Configuration
    static void set_midi_channel(uint8_t channel) { midi_channel = channel; }
    static void enable_sysex(bool enable) { sysex_enabled = enable; }
    static void enable_cc(bool enable) { cc_enabled = enable; }

private:
    static uint8_t midi_channel;
    static bool sysex_enabled;
    static bool cc_enabled;
    static std::vector<uint8_t> sysex_buffer;
    static bool in_sysex;
    
    // Helper functions
    static void send_bytes(const uint8_t* data, size_t length);
    static void handle_sysex();
};

} // namespace midi
} // namespace pg1000
