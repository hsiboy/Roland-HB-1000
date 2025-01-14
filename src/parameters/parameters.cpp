#include "parameters.h"
#include <array>
#include <cstddef>

namespace pg1000 {

// D50 Parameter Definitions
static const std::array<Parameter, 34> PARAMETERS = {{
    // COMMON parameters
    {0x0319, 0, 0, ParamType::CONTINUOUS_100, 100, 0, ParamGroup::COMMON, "DCO-1 Fine Tune", false, 3},
    {0x0318, 0, 0, ParamType::CONTINUOUS_100, 100, 1, ParamGroup::COMMON, "DCO-1 Coarse", false, 9},
    {0x0321, 0, 0, ParamType::CONTINUOUS_100, 100, 2, ParamGroup::COMMON, "DCO-2 Fine Tune", false, 14},
    {0x031C, 0, 0, ParamType::CONTINUOUS_100, 100, 3, ParamGroup::COMMON, "DCO-2 Coarse", false, 15},
    {0x0323, 0, 0, ParamType::CONTINUOUS_100, 100, 4, ParamGroup::COMMON, "DCO Balance", false, 20},
    {0x0324, 0, 0, ParamType::CONTINUOUS_100, 100, 5, ParamGroup::COMMON, "DCO Mix", false, 21},
    {0x012F, 0, 0, ParamType::CONTINUOUS_100, 100, 6, ParamGroup::COMMON, "Chorus Rate", false, 22},
    {0x0116, 0, 0, ParamType::CONTINUOUS_100, 100, 7, ParamGroup::COMMON, "Chorus Depth", false, 23},
    {0x0117, 0, 0, ParamType::CONTINUOUS_100, 100, 8, ParamGroup::COMMON, "Chorus Balance", false, 24},
    {0x0118, 0, 0, ParamType::CONTINUOUS_100, 100, 9, ParamGroup::COMMON, "Output Level", false, 25},

    // TVF parameters
    {0x011A, 0, 0, ParamType::CONTINUOUS_100, 100, 10, ParamGroup::COMMON, "TVF Cutoff", false, 26},
    {0x011B, 0, 0, ParamType::CONTINUOUS_100, 100, 11, ParamGroup::COMMON, "TVF Resonance", false, 27},
    {0x011E, 0, 0, ParamType::CONTINUOUS_100, 100, 12, ParamGroup::COMMON, "TVF Key Follow", false, 28},
    {0x011F, 0, 0, ParamType::CONTINUOUS_100, 100, 13, ParamGroup::COMMON, "TVF Env Depth", false, 29},

    // Time parameters (0-50 range)
    {0x010D, 0, 0, ParamType::CONTINUOUS_50, 50, 14, ParamGroup::COMMON, "T1", false, 30},
    {0x010E, 0, 0, ParamType::CONTINUOUS_50, 50, 15, ParamGroup::COMMON, "T2", false, 31},
    {0x010F, 0, 0, ParamType::CONTINUOUS_50, 50, 16, ParamGroup::COMMON, "T3", false, 85},
    {0x0110, 0, 0, ParamType::CONTINUOUS_50, 50, 17, ParamGroup::COMMON, "T4", false, 86},
    
    // Add more parameters...
}};

// Parameter state storage
static std::array<ParameterState, PARAMETERS.size()> parameter_states;

int get_parameter_count() {
    return PARAMETERS.size();
}

const Parameter* get_parameter(int index) {
    if (index >= 0 && static_cast<size_t>(index) < PARAMETERS.size()) {
        return &PARAMETERS[index];
    }
    return nullptr;
}

const Parameter* get_parameter_by_id(uint16_t id) {
    for (const auto& param : PARAMETERS) {
        if (param.id == id) {
            return &param;
        }
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
