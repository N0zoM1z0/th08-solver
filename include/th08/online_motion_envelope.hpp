#pragma once
#include <algorithm>

namespace th08::online::detail {
// Conditional kinematic hypothesis: retain the observed-motion path and every
// possible single switch to an owned alternative velocity. Repeated additions
// preserve the modeled float update order; x/y extrema may belong to different
// switch times, so their rectangle deliberately overapproximates that path set.
// This does not predict whether/when native state will switch, or other transforms.
struct MotionEnvelope {
    float min_x, min_y, max_x, max_y;
    MotionEnvelope(float x, float y) : min_x(x), min_y(y), max_x(x), max_y(y) {}
    void advance(float observed_x, float observed_y, float alternate_vx, float alternate_vy) {
        min_x = std::min(observed_x, min_x + alternate_vx);
        min_y = std::min(observed_y, min_y + alternate_vy);
        max_x = std::max(observed_x, max_x + alternate_vx);
        max_y = std::max(observed_y, max_y + alternate_vy);
    }
};
} // namespace th08::online::detail
