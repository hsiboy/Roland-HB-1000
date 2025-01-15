#include "sysex.h"
#include <algorithm>

namespace pg1000 {
namespace midi {

// Static member initialization
uint8_t SysEx::midi_channel = 1;

std::vector<uint8_t> SysEx::create_parameter_request() {
    std::vector<uint8_t> msg;
    msg.reserve(13);  // Fixed size for RQ1 message

    // Message header
    msg.push_back(SysExConst::STATUS);
    msg.push_back(SysExConst::ROLAND_ID);
    msg.push_back(get_device_id());
    msg.push_back(SysExConst::D50_ID);
    msg.push_back(static_cast<uint8_t>(SysExCommand::RQ1));

    // Add starting address [00-00-00]
    add_address(msg, UPPER_PARTIAL_1);

    // Add size [00-03-25] (421 bytes)
    msg.push_back(0x00);
    msg.push_back(0x03);
    msg.push_back(0x25);

    // Add checksum and EOX
    add_checksum(msg);
    msg.push_back(SysExConst::EOX);

    return msg;
}

std::vector<uint8_t> SysEx::create_parameter_set(const Parameter* param, uint8_t value) {
    if (!param) return std::vector<uint8_t>();

    std::vector<uint8_t> msg;
    msg.reserve(11);  // Fixed size for DT1 message

    // Message header
    msg.push_back(SysExConst::STATUS);
    msg.push_back(SysExConst::ROLAND_ID);
    msg.push_back(get_device_id());
    msg.push_back(SysExConst::D50_ID);
    msg.push_back(static_cast<uint8_t>(SysExCommand::DT1));

    // Add parameter address
    add_address(msg, get_parameter_address(param));

    // Add parameter value
    msg.push_back(value & 0x7F);  // Ensure 7-bit value

    // Add checksum and EOX
    add_checksum(msg);
    msg.push_back(SysExConst::EOX);

    return msg;
}

std::vector<uint8_t> SysEx::create_patch_write() {
    std::vector<uint8_t> msg;
    msg.reserve(13);

    // Message header
    msg.push_back(SysExConst::STATUS);
    msg.push_back(SysExConst::ROLAND_ID);
    msg.push_back(get_device_id());
    msg.push_back(SysExConst::D50_ID);
    msg.push_back(static_cast<uint8_t>(SysExCommand::DT1));

    // Add patch write address [00-20-00]
    add_address(msg, PATCH_WRITE);

    // Add two zero bytes
    msg.push_back(0x00);
    msg.push_back(0x00);

    // Add checksum and EOX
    add_checksum(msg);
    msg.push_back(SysExConst::EOX);

    return msg;
}

std::vector<uint8_t> SysEx::create_bulk_request() {
    // Similar to parameter request but with different size
    return create_parameter_request();
}

bool SysEx::parse_message(const std::vector<uint8_t>& data) {
    if (!is_valid_message(data)) return false;

    // Get command type
    SysExCommand cmd = static_cast<SysExCommand>(data[4]);

    switch (cmd) {
        case SysExCommand::DT1: {
            // Extract address and value
            SysExAddress addr{data[5], data[6], data[7]};
            uint8_t value = data[8];
            
            // TODO: Update parameter based on address
            return true;
        }
        
        case SysExCommand::RQ1:
            // We don't handle incoming requests
            return false;
            
        default:
            return false;
    }
}

bool SysEx::is_valid_message(const std::vector<uint8_t>& data) {
    // Check minimum length and SysEx markers
    if (data.size() < 10 || 
        data[0] != SysExConst::STATUS || 
        data.back() != SysExConst::EOX) return false;

    // Verify Roland message
    if (data[1] != SysExConst::ROLAND_ID ||
        data[3] != SysExConst::D50_ID) return false;

    // Verify checksum
    return calculate_checksum(data) == data[data.size() - 2];
}

SysExAddress SysEx::get_parameter_address(const Parameter* param) {
    if (!param) return SysExAddress();

    // TODO: Calculate actual address based on parameter type and offset
    // This needs to be implemented based on the full parameter mapping
    return SysExAddress();
}

uint8_t SysEx::calculate_checksum(const std::vector<uint8_t>& data) {
    if (data.size() < 6) return 0;  // Not enough data for checksum

    uint8_t sum = 0;
    // Sum all bytes between Command-ID and EOX (excluding both)
    for (size_t i = 5; i < data.size() - 2; i++) {
        sum += data[i];
    }
    
    // Return Roland checksum (LSB of inverted sum)
    return (128 - (sum & 0x7F)) & 0x7F;
}

void SysEx::add_checksum(std::vector<uint8_t>& msg) {
    msg.push_back(calculate_checksum(msg));
}

void SysEx::add_address(std::vector<uint8_t>& msg, const SysExAddress& addr) {
    msg.push_back(addr.msb);
    msg.push_back(addr.mid);
    msg.push_back(addr.lsb);
}

} // namespace midi
} // namespace pg1000
