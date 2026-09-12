#pragma once
#include <cstdint>

namespace th08::timing {
// Supervisor::TickTimer advances at most one integer frame. Rates above 0.99
// bypass fractional accumulation rather than adding their numeric value.
// Preconditions: finite positive multiplier, fraction in [0,1), non-overflowing frame.
inline void tick(std::int32_t &frame, float &fraction, float multiplier) {
    if (multiplier <= 0.99f) {
        fraction += multiplier;
        if (fraction >= 1) {
            ++frame;
            fraction -= 1;
        }
    } else {
        ++frame;
    }
}
} // namespace th08::timing
