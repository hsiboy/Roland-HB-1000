#pragma once

#include <cstdint>
#include <vector>
#include "pico/stdlib.h"
#include "../parameters/parameters.h"

namespace pg1000 {
namespace midi {

// SysEx Message Types
enum class SysExCommand : uint8_t {
    RQ1 = 0x11,  // Request data (one way)
    DT1 = 0x12   // Data set (two way)
};

// SysEx Constants
struct SysExConst {
    static constexpr uint8_t STATUS = 0xF0;        // Start of SysEx
    static constexpr uint8_t EOX = 0xF7;           // End of SysEx
    static constexpr uint8_t ROLAND_ID = 0x41;     // Roland manufacturer ID
    static constexpr uint8_t D50_ID = 0x14;        // D-50 model ID
    static constexpr uint16_t FULL_REQUEST_SIZE = 421; // Size for full parameter request
};

// SysEx Address Structure
struct SysExAddress {
    uint8_t msb;    // Most significant byte
    uint8_t mid;    // Middle byte
    uint8_t lsb;    // Least significant byte

    // Constructor for easy initialization
    constexpr SysExAddress(uint8_t m = 0, uint8_t i = 0, uint8_t l = 0) 
        : msb(m), mid(i), lsb(l) {}
};

class SysEx {
public:
    // Create SysEx messages
    static std::vector<uint8_t> create_parameter_request();
    static std::vector<uint8_t> create_parameter_set(const Parameter* param, uint8_t value);
    static std::vector<uint8_t> create_patch_write();
    static std::vector<uint8_t> create_bulk_request();

    // Message parsing
    static bool parse_message(const std::vector<uint8_t>& data);
    static bool is_valid_message(const std::vector<uint8_t>& data);
    
    // Address helpers
    static SysExAddress get_parameter_address(const Parameter* param);
    static uint8_t calculate_checksum(const std::vector<uint8_t>& data);

    // Set MIDI channel for device ID
    static void set_midi_channel(uint8_t channel) { midi_channel = channel; }

private:
    static uint8_t midi_channel;  // Current MIDI channel (1-16)

    // Base addresses for different sections
    static constexpr SysExAddress UPPER_PARTIAL_1{0x00, 0x00, 0x00};  // 0-53
    static constexpr SysExAddress UPPER_PARTIAL_2{0x00, 0x00, 0x40};  // 64-117
    static constexpr SysExAddress UPPER_COMMON{0x00, 0x01, 0x00};     // 128-175
    static constexpr SysExAddress LOWER_PARTIAL_1{0x00, 0x01, 0x40};  // 192-245
    static constexpr SysExAddress LOWER_PARTIAL_2{0x00, 0x02, 0x00};  // 256-309
    static constexpr SysExAddress LOWER_COMMON{0x00, 0x02, 0x40};     // 320-367
    static constexpr SysExAddress PATCH{0x00, 0x03, 0x00};            // 384-420
    static constexpr SysExAddress PATCH_WRITE{0x00, 0x20, 0x00};      // Patch write address

    // Internal helper functions
    static uint8_t get_device_id() { return static_cast<uint8_t>(midi_channel - 1); }
    static void add_checksum(std::vector<uint8_t>& msg);
    static void add_address(std::vector<uint8_t>& msg, const SysExAddress& addr);
};

} // namespace midi
} // namespace pg1000
