#pragma once

#include <cstdint>

namespace pg1000 {

// Parameter Groups
enum class ParamGroup {
    COMMON,
    UPPER,
    LOWER,
    TOTAL_GROUPS
};

// Parameter Types
enum class ParamType {
    CONTINUOUS_100,  // 0-100 range
    CONTINUOUS_50,   // 0-50 range
    SWITCH          // Binary or small range switches
};

// Parameter definition
struct Parameter {
    uint16_t id;          // SysEx parameter ID
    uint8_t value;        // Current value
    uint8_t prev_value;   // Previous value
    ParamType type;       // Parameter type
    uint8_t max_value;    // Maximum value
    uint8_t pot_number;   // Physical control number
    ParamGroup group;     // Which parameter group
    const char* name;     // Parameter name
    bool active;          // Is this parameter currently active?
    uint8_t cc_number;    // Optional CC mapping
};

// Parameter state tracking
struct ParameterState {
    float current_value;  // Filtered current value
    float alpha;          // Filter coefficient
};

// Get number of defined parameters
int get_parameter_count();

// Get parameter by index
const Parameter* get_parameter(int index);

// Get parameter by SysEx ID
const Parameter* get_parameter_by_id(uint16_t id);

// Get parameter by potentiometer number
const Parameter* get_parameter_by_pot(uint8_t pot_number);

// Update parameter value
void update_parameter_value(const Parameter* param, uint8_t new_value);

// Get filtered value for parameter
float get_filtered_value(const Parameter* param);

} // namespace pg1000
