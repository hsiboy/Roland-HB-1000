#pragma once

#include <cstdint>
#include <array>

namespace pg1000 {

enum class ParamType {
    CONTINUOUS_100,  // 0-100 range parameters
    CONTINUOUS_50,   // 0-50 range parameters
    SWITCH,         // On/Off or small range switches
    KEYFOLLOW,      // Keyfollow parameters
    ENUM           // Enumerated values (like waveforms)
};

enum class ParamGroup {
    COMMON,         // Common parameters
    UPPER_PARTIAL_1,
    UPPER_PARTIAL_2,
    UPPER_COMMON,
    LOWER_PARTIAL_1,
    LOWER_PARTIAL_2,
    LOWER_COMMON,
    PATCH
};

// Parameter definition
struct Parameter {
    const char* name;           // Parameter name
    ParamGroup group;          // Which section this belongs to
    ParamType type;           // Parameter type
    union {
        uint8_t partial_offset; // Offset for partial parameters (WG, TVF, TVA)
        uint8_t common_offset;  // Offset for common parameters
        uint8_t patch_offset;   // Offset for patch parameters
    };
    uint8_t value;            // Current value
    uint8_t prev_value;       // Previous value
    uint8_t min_value;        // Minimum allowed value
    uint8_t max_value;        // Maximum allowed value
    uint8_t pot_number;       // Which potentiometer controls this
    bool active;             // Is this parameter currently active?
};

// Parameter value filtering state
struct ParameterState {
    float current_value;
    float alpha;  // Filter coefficient
};

// Function declarations
int get_parameter_count();
const Parameter* get_parameter(int index);
const Parameter* get_parameter_by_pot(uint8_t pot_number);
void update_parameter_value(const Parameter* param, uint8_t new_value);
float get_filtered_value(const Parameter* param);

} // namespace pg1000
