#include "parameters.h"
#include <array>
#include <cstddef>

namespace pg1000 {

static const std::array<Parameter, 56> PARAMETERS = {{

// format of parameters is as follows:
// {
//        "WG Pitch Coarse",           // name
//        ParamGroup::UPPER_PARTIAL_1, // group
//        ParamType::CONTINUOUS_100,   // type
//        {.partial_offset = 0},       // offset
//        0,                           // current value
//        0,                           // previous value
//        0,                           // min value
//        72,                          // max value (C1-C7)
//        0,                           // pot number
//        false                        // active
//    },

    // Wave Generator (WG) Parameters - Upper Partial 1
    {"WG Pitch Coarse", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {0}, 0, 0, 0, 72, 0, true},
    {"WG Pitch Fine", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {1}, 0, 0, 0, 100, 1, true},
    {"WG Pitch Keyfollow", ParamGroup::UPPER_PARTIAL_1, ParamType::KEYFOLLOW, {2}, 0, 0, 0, 16, 2, true},
    {"WG Mod LFO Mode", ParamGroup::UPPER_PARTIAL_1, ParamType::ENUM, {3}, 0, 0, 0, 3, 3, true},
    {"WG Mod P-ENV Mode", ParamGroup::UPPER_PARTIAL_1, ParamType::ENUM, {4}, 0, 0, 0, 2, 4, true},
    {"WG Mod Bender Mode", ParamGroup::UPPER_PARTIAL_1, ParamType::ENUM, {5}, 0, 0, 0, 2, 5, true},
    {"WG Waveform", ParamGroup::UPPER_PARTIAL_1, ParamType::ENUM, {6}, 0, 0, 0, 1, 6, true},
    {"WG PCM Wave No.", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {7}, 0, 0, 0, 99, 7, true},
    {"WG Pulse Width", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {8}, 0, 0, 0, 100, 8, true},
    {"WG PW Velocity Range", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {9}, 0, 0, 0, 14, 9, true},

    // Time Variant Filter (TVF) Parameters
    {"TVF Cutoff Freq", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {13}, 0, 0, 0, 100, 10, true},
    {"TVF Resonance", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {14}, 0, 0, 0, 30, 11, true},
    {"TVF Keyfollow", ParamGroup::UPPER_PARTIAL_1, ParamType::KEYFOLLOW, {15}, 0, 0, 0, 14, 12, true},
    {"TVF Bias Point/Dir", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {16}, 0, 0, 0, 127, 13, true},
    {"TVF Bias Level", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {17}, 0, 0, -7, 7, 14, true},
    {"TVF ENV Depth", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {18}, 0, 0, 0, 100, 15, true},

    // Time Variant Amplifier (TVA) Parameters
    {"TVA Level", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {35}, 0, 0, 0, 100, 16, true},
    {"TVA Velocity Range", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {36}, 0, 0, -50, 50, 17, true},
    {"TVA Bias Point Dir", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {37}, 0, 0, 0, 127, 18, true},
    {"TVA Bias Level", ParamGroup::UPPER_PARTIAL_1, ParamType::CONTINUOUS_100, {38}, 0, 0, -12, 0, 19, true},

    // Common Parameters
    {"Structure", ParamGroup::COMMON, ParamType::ENUM, {10}, 0, 0, 0, 6, 20, true},
    {"P-ENV Velocity Range", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {11}, 0, 0, 0, 2, 21, true},
    {"P-ENV Time Keyfollow", ParamGroup::COMMON, ParamType::KEYFOLLOW, {12}, 0, 0, 0, 4, 22, true},
    {"P-ENV Time 1", ParamGroup::COMMON, ParamType::CONTINUOUS_50, {13}, 0, 0, 0, 50, 23, true},
    {"P-ENV Time 2", ParamGroup::COMMON, ParamType::CONTINUOUS_50, {14}, 0, 0, 0, 50, 24, true},
    {"P-ENV Time 3", ParamGroup::COMMON, ParamType::CONTINUOUS_50, {15}, 0, 0, 0, 50, 25, true},
    {"P-ENV Time 4", ParamGroup::COMMON, ParamType::CONTINUOUS_50, {16}, 0, 0, 0, 50, 26, true},

    // LFO Parameters
    {"LFO-1 Waveform", ParamGroup::COMMON, ParamType::ENUM, {25}, 0, 0, 0, 3, 27, true},
    {"LFO-1 Rate", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {26}, 0, 0, 0, 100, 28, true},
    {"LFO-1 Delay Time", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {27}, 0, 0, 0, 100, 29, true},
    {"LFO-1 Sync", ParamGroup::COMMON, ParamType::ENUM, {28}, 0, 0, 0, 2, 30, true},

    // EQ Parameters
    {"Low EQ Freq", ParamGroup::COMMON, ParamType::ENUM, {37}, 0, 0, 0, 15, 31, true},
    {"Low EQ Gain", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {38}, 0, 0, -12, 12, 32, true},
    {"High EQ Freq", ParamGroup::COMMON, ParamType::ENUM, {39}, 0, 0, 0, 21, 33, true},
    {"High EQ Q", ParamGroup::COMMON, ParamType::ENUM, {40}, 0, 0, 0, 8, 34, true},
    {"High EQ Gain", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {41}, 0, 0, -12, 12, 35, true},

    // Chorus Parameters
    {"Chorus Type", ParamGroup::COMMON, ParamType::ENUM, {42}, 0, 0, 1, 8, 36, true},
    {"Chorus Rate", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {43}, 0, 0, 0, 100, 37, true},
    {"Chorus Depth", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {44}, 0, 0, 0, 100, 38, true},
    {"Chorus Balance", ParamGroup::COMMON, ParamType::CONTINUOUS_100, {45}, 0, 0, 0, 100, 39, true},

    // Patch Parameters
    {"Portamento Mode", ParamGroup::PATCH, ParamType::ENUM, {20}, 0, 0, 0, 2, 40, true},
    {"Hold Mode", ParamGroup::PATCH, ParamType::ENUM, {21}, 0, 0, 0, 2, 41, true},
    {"Upper Key Shift", ParamGroup::PATCH, ParamType::CONTINUOUS_100, {22}, 0, 0, -24, 24, 42, true},
    {"Lower Key Shift", ParamGroup::PATCH, ParamType::CONTINUOUS_100, {23}, 0, 0, -24, 24, 43, true},
    {"Upper Fine Tune", ParamGroup::PATCH, ParamType::CONTINUOUS_100, {24}, 0, 0, -50, 50, 44, true},
    {"Lower Fine Tune", ParamGroup::PATCH, ParamType::CONTINUOUS_100, {25}, 0, 0, -50, 50, 45, true}
}};

// Parameter state storage
static std::array<ParameterState, PARAMETERS.size()> parameter_states;

int get_parameter_count() {
    return PARAMETERS.size();
}

const Parameter* get_parameter(int index) {
    if (index >= 0 && index < static_cast<int>(PARAMETERS.size())) {
        return &PARAMETERS[index];
    }
    return nullptr;
}

const Parameter* get_parameter_by_pot(uint8_t pot_number) {
    for (const auto& param : PARAMETERS) {
        if (param.pot_number == pot_number) {
            return &param;
        }
    }
    return nullptr;
}

void update_parameter_value(const Parameter* param, uint8_t new_value) {
    if (!param) return;
    
    // Find parameter index
    for (size_t i = 0; i < PARAMETERS.size(); i++) {
        if (&PARAMETERS[i] == param) {
            // Apply exponential filter
            auto& state = parameter_states[i];
            state.current_value = state.current_value + 
                state.alpha * (static_cast<float>(new_value) - state.current_value);
            
            // Update parameter value
            const_cast<Parameter*>(param)->prev_value = param->value;
            const_cast<Parameter*>(param)->value = static_cast<uint8_t>(state.current_value);
            break;
        }
    }
}

float get_filtered_value(const Parameter* param) {
    if (!param) return 0.0f;
    
    // Find parameter index
    for (size_t i = 0; i < PARAMETERS.size(); i++) {
        if (&PARAMETERS[i] == param) {
            return parameter_states[i].current_value;
        }
    }
    return 0.0f;
}

} // namespace pg1000
