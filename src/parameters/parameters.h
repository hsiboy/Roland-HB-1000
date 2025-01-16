#pragma once

#include <cstdint>
#include <array>

namespace pg1000 {


enum class ParamType {
   CONTINUOUS_100,  // 0-100 range (standard params)
   CONTINUOUS_50,   // 0-50 range (time params)
   KEYFOLLOW,      // Keyfollow rates (-1, -1/2, -4/1, etc)
   ENUM,           // Fixed value choices (waveforms etc)
   BIPOLAR_50,     // -50 to +50 range
   BIPOLAR_24,     // -24 to +24 (key shift)
   BIPOLAR_12,     // -12 to +12 (EQ gain)
   BIPOLAR_7       // -7 to +7 (bias levels)
};

enum class ParamGroup {
   UPPER_PARTIAL_1,  // Base addr: 00-00-00
   UPPER_PARTIAL_2,  // Base addr: 00-00-40  
   UPPER_COMMON,     // Base addr: 00-01-00
   LOWER_PARTIAL_1,  // Base addr: 00-01-40
   LOWER_PARTIAL_2,  // Base addr: 00-02-00
   LOWER_COMMON,     // Base addr: 00-02-40
   PATCH,           // Base addr: 00-03-00
   COMMON           // Shared settings
};


// Parameter definition

struct Parameter {
    const char* name;           // Parameter name
    ParamGroup group;          // Which section this belongs to
    ParamType type;           // Parameter type
    uint8_t id;              // Unique parameter identifier
    union {
        uint8_t partial_offset; // Offset for partial parameters (WG, TVF, TVA)
        uint8_t common_offset;  // Offset for common parameters
        uint8_t patch_offset;   // Offset for patch parameters
    };
    uint8_t value;            // Current value
    uint8_t prev_value;       // Previous value
    int8_t min_value;        // Minimum allowed value
    int8_t max_value;        // Maximum allowed value
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
