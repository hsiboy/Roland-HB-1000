#pragma once

#include <cstdint>
#include <vector>
#include "../parameters/parameters.h"

namespace pg1000 {
namespace midi {

// D50 SysEx format constants
struct D50SysEx {
    static constexpr uint8_t START_BYTE = 0xF0;
    static constexpr uint8_t END_BYTE = 0xF7;
    static constexpr uint8_t ROLAND_ID = 0x41;
    static constexpr uint8_t D50_ID = 0x14;
    static constexpr uint8_t FORMAT_TYPE = 0x12;  // DT1 format
    
    // Message lengths
    static constexpr uint8_t HEADER_LENGTH = 5;   // F0 + ID + Device + Command
    static constexpr uint8_t MESSAGE_LENGTH = 11; // Total message length
    
    // Command types
    static constexpr uint8_t CMD_DT1 = 0x12;     // Data set 1
    static constexpr uint8_t CMD_RQ1 = 0x11;     // Data request 1
};

class SysEx {
public:
    // Message creation
    static std::vector<uint8_t> create_parameter_message(const Parameter* param);
    static std::vector<uint8_t> create_request_message(uint16_t parameter_id);
    static std::vector<uint8_t> create_bulk_request_message();
    
    // Message parsing
    static bool parse_message(const std::vector<uint8_t>& data);
    static bool is_valid_message(const std::vector<uint8_t>& data);
    
    // Utility functions
    static uint16_t get_parameter_id(const std::vector<uint8_t>& data);
    static uint8_t get_parameter_value(const std::vector<uint8_t>& data);
    static bool is_bulk_data(const std::vector<uint8_t>& data);

private:
    // Message validation
    static bool verify_header(const std::vector<uint8_t>& data);
    static bool verify_length(const std::vector<uint8_t>& data);
    static uint8_t calculate_checksum(const std::vector<uint8_t>& data);
    
    // Helpers for message construction
    static void add_header(std::vector<uint8_t>& msg);
    static void add_address(std::vector<uint8_t>& msg, uint16_t address);
    static void add_checksum(std::vector<uint8_t>& msg);
};

} // namespace midi
} // namespace pg1000
