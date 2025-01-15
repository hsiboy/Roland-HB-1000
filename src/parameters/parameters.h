#pragma once

#include <cstdint>

namespace pg1000 {

enum class ParamGroup {
    UPPER_PARTIAL_1,
    UPPER_PARTIAL_2,
    UPPER_COMMON,
    LOWER_PARTIAL_1,
    LOWER_PARTIAL_2,
    LOWER_COMMON,
    PATCH
};

enum class ParamType {
    WG,        // Wave Generator parameters
    TVF,       // Time Variant Filter parameters
    TVA,       // Time Variant Amplifier parameters
    COMMON,    // Common parameters
    PATCH      // Patch parameters
};

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

} // namespace pg1000
