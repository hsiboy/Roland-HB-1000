#pragma once

#include "pico/stdlib.h"
#include "pico/platform.h"

namespace pg1000 {
namespace hardware {

template<size_t WINDOW_SIZE = 8>
class ValueSmoother {
public:
    ValueSmoother() : index(0), sum(0) {
        buffer.fill(0);
    }

    // Add a new value and get the smoothed result
    uint16_t update(uint16_t new_value) {
        // Subtract the oldest value and add the new one
        sum -= buffer[index];
        sum += new_value;
        buffer[index] = new_value;

        // Update circular buffer index
        index = (index + 1) % WINDOW_SIZE;

        // Return averaged value
        return sum / WINDOW_SIZE;
    }

    // Apply hysteresis to prevent small value fluctuations
    uint16_t apply_hysteresis(uint16_t current, uint16_t previous, uint16_t threshold = 4) {
        if (current > previous && (current - previous) < threshold) {
            return previous;
        }
        if (previous > current && (previous - current) < threshold) {
            return previous;
        }
        return current;
    }

    // Reset the smoother
    void reset() {
        index = 0;
        sum = 0;
        buffer.fill(0);
    }

private:
    std::array<uint16_t, WINDOW_SIZE> buffer;
    size_t index;
    uint32_t sum;  // Using 32-bit to prevent overflow during summation
};

} // namespace hardware
} // namespace pg1000
