#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <chrono>
#include "sysex.h"
#include "../parameters/parameters.h"
#include "../hardware/value_smoother.h"

namespace pg1000 {
namespace midi {

// MIDI Constants
static constexpr uint8_t MIDI_CHANNEL = 1;     // Default channel
static constexpr uint32_t MIDI_BAUD = 31250;   // MIDI baud rate
static constexpr uint8_t UART_TX = 0;          // UART TX pin
static constexpr uint8_t UART_RX = 1;          // UART RX pin
static constexpr size_t MAX_SYSEX_SIZE = 256;  // Maximum SysEx message size
static constexpr uint8_t MAX_PARAMETERS = 128;  // Maximum number of parameters
static constexpr uint32_t MIN_UPDATE_INTERVAL = 10000;  // Minimum time between parameter updates (10ms)

// MIDI Message Types
enum class MessageType : uint8_t {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    SYSTEM_EXCLUSIVE = 0xF0,
    TIMING_CLOCK = 0xF8,
    START = 0xFA,
    CONTINUE = 0xFB,
    STOP = 0xFC,
    ACTIVE_SENSING = 0xFE,
    SYSTEM_RESET = 0xFF
};

// Roland Constants
static constexpr uint8_t ROLAND_ID = 0x41;
static constexpr uint8_t D50_ID = 0x14;
static constexpr uint8_t DT1_COMMAND = 0x12;
static constexpr uint8_t RQ1_COMMAND = 0x11;

// MIDI Error Codes
enum class MidiError {
    OK = 0,
    INVALID_CHANNEL,
    INVALID_PARAMETER,
    INVALID_VALUE,
    BUFFER_OVERFLOW,
    CHECKSUM_ERROR,
    MALFORMED_MESSAGE
};

class MIDI {
public:
    // Initialize MIDI
    static bool init();

    // MIDI message sending
    static MidiError send_cc(uint8_t cc, uint8_t value);
    static MidiError send_sysex(const Parameter* param);
    static MidiError send_program_change(uint8_t program);
    static MidiError request_parameter(const Parameter* param);
    static MidiError request_all_parameters();

    // MIDI message receiving
    static void process_incoming();
    
    // Configuration
    static void enable_sysex(bool enable) { sysex_enabled = enable; }
    static void enable_cc(bool enable) { cc_enabled = enable; }
    static void set_update_interval(uint32_t interval_us) { min_update_interval = interval_us; }

    // MIDI channel access
    static void set_midi_channel(uint8_t channel) { 
        if (channel >= 1 && channel <= 16) midi_channel = channel;
    }
    static uint8_t get_midi_channel() { return midi_channel; }

    // Error handling
    static const char* get_error_string(MidiError error);

private:
    static uint8_t midi_channel;
    static bool sysex_enabled;
    static bool cc_enabled;
    static std::vector<uint8_t> sysex_buffer;
    static bool in_sysex;
    static uint32_t min_update_interval;
    static std::array<hardware::ValueSmoother<4>, MAX_PARAMETERS> parameter_smoothers;
    static std::array<std::chrono::steady_clock::time_point, MAX_PARAMETERS> last_update_time;
    
    // Helper functions
    static MidiError send_bytes(const uint8_t* data, size_t length);
    static void handle_sysex();
    static uint8_t calculate_checksum(const uint8_t* data, size_t length);
    static bool verify_checksum(const std::vector<uint8_t>& message);
    static bool should_update_parameter(uint8_t parameter_index);
};

} // namespace midi
} // namespace pg1000
