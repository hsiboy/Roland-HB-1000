#include "sysex.h"
#include <algorithm>

namespace pg1000 {
namespace midi {

std::vector<uint8_t> SysEx::create_parameter_message(const Parameter* param) {
    if (!param) return std::vector<uint8_t>();
    
    std::vector<uint8_t> msg;
    msg.reserve(D50SysEx::MESSAGE_LENGTH);
    
    // Add basic header
    add_header(msg);
    
    // Add command type (DT1)
    msg.push_back(D50SysEx::CMD_DT1);
    
    // Add parameter address
    add_address(msg, param->id);
    
    // Add parameter value
    msg.push_back(param->value);
    
    // Add checksum
    add_checksum(msg);
    
    // Add end marker
    msg.push_back(D50SysEx::END_BYTE);
    
    return msg;
}

std::vector<uint8_t> SysEx::create_request_message(uint16_t parameter_id) {
    std::vector<uint8_t> msg;
    msg.reserve(D50SysEx::MESSAGE_LENGTH);
    
    // Add basic header
    add_header(msg);
    
    // Add command type (RQ1)
    msg.push_back(D50SysEx::CMD_RQ1);
    
    // Add parameter address
    add_address(msg, parameter_id);
    
    // Add size (1 byte for single parameter)
    msg.push_back(0x00);
    msg.push_back(0x00);
    msg.push_back(0x01);
    
    // Add checksum
    add_checksum(msg);
    
    // Add end marker
    msg.push_back(D50SysEx::END_BYTE);
    
    return msg;
}

std::vector<uint8_t> SysEx::create_bulk_request_message() {
    std::vector<uint8_t> msg;
    msg.reserve(D50SysEx::MESSAGE_LENGTH);
    
    // Add basic header
    add_header(msg);
    
    // Add command type (RQ1)
    msg.push_back(D50SysEx::CMD_RQ1);
    
    // Add starting address (0x0000)
    msg.push_back(0x00);
    msg.push_back(0x00);
    msg.push_back(0x00);
    
    // Add size (all parameters)
    msg.push_back(0x00);
    msg.push_back(0x40);  // Request all parameters
    msg.push_back(0x00);
    
    // Add checksum
    add_checksum(msg);
    
    // Add end marker
    msg.push_back(D50SysEx::END_BYTE);
    
    return msg;
}

bool SysEx::parse_message(const std::vector<uint8_t>& data) {
    if (!is_valid_message(data)) return false;
    
    // Message is valid - extract data based on message type
    uint8_t command = data[4];
    
    switch (command) {
        case D50SysEx::CMD_DT1:
            // Parameter data received
            return true;
            
        default:
            // Unsupported message type
            return false;
    }
}

bool SysEx::is_valid_message(const std::vector<uint8_t>& data) {
    return verify_header(data) && 
           verify_length(data) && 
           (data.back() == D50SysEx::END_BYTE) &&
           (calculate_checksum(data) == data[data.size() - 2]);
}

uint16_t SysEx::get_parameter_id(const std::vector<uint8_t>& data) {
    if (!is_valid_message(data)) return 0;
    
    return (data[6] << 8) | data[7];
}

uint8_t SysEx::get_parameter_value(const std::vector<uint8_t>& data) {
    if (!is_valid_message(data)) return 0;
    
    return data[8];
}

bool SysEx::is_bulk_data(const std::vector<uint8_t>& data) {
    if (!is_valid_message(data)) return false;
    
    // Check if this is a bulk data message
    return (data[4] == D50SysEx::CMD_DT1) && 
           (data[5] == 0x00) && 
           (data[6] == 0x00) && 
           (data[7] == 0x00);
}

bool SysEx::verify_header(const std::vector<uint8_t>& data) {
    return data.size() >= D50SysEx::HEADER_LENGTH &&
           data[0] == D50SysEx::START_BYTE &&
           data[1] == D50SysEx::ROLAND_ID &&
           data[3] == D50SysEx::D50_ID;
}

bool SysEx::verify_length(const std::vector<uint8_t>& data) {
    // Check minimum length
    if (data.size() < D50SysEx::MESSAGE_LENGTH) return false;
    
    // For bulk messages, verify the length matches the data size
    if (is_bulk_data(data)) {
        size_t expected_length = data[6] * 128 + data[7] + D50SysEx::HEADER_LENGTH + 2;  // +2 for checksum and end marker
        return data.size() == expected_length;
    }
    
    // For parameter messages, verify standard length
    return data.size() == D50SysEx::MESSAGE_LENGTH;
}

uint8_t SysEx::calculate_checksum(const std::vector<uint8_t>& data) {
    uint8_t sum = 0;
    
    // Sum all bytes between header and checksum
    for (size_t i = D50SysEx::HEADER_LENGTH; i < data.size() - 2; i++) {
        sum += data[i];
    }
    
    // Return Roland checksum (LSB of inverted sum)
    return (128 - (sum & 0x7F)) & 0x7F;
}

void SysEx::add_header(std::vector<uint8_t>& msg) {
    msg.push_back(D50SysEx::START_BYTE);
    msg.push_back(D50SysEx::ROLAND_ID);
    msg.push_back(0x10);  // Device ID
    msg.push_back(D50SysEx::D50_ID);
}

void SysEx::add_address(std::vector<uint8_t>& msg, uint16_t address) {
    msg.push_back(0x00);  // Address MSB
    msg.push_back((address >> 8) & 0x7F);
    msg.push_back(address & 0x7F);
}

void SysEx::add_checksum(std::vector<uint8_t>& msg) {
    uint8_t sum = 0;
    
    // Calculate checksum starting after header
    for (size_t i = D50SysEx::HEADER_LENGTH; i < msg.size(); i++) {
        sum += msg[i];
    }
    
    // Add Roland checksum
    msg.push_back((128 - (sum & 0x7F)) & 0x7F);
}

} // namespace midi
} // namespace pg1000
