#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace th08::hazard_probe {

// Exact arguments of a native laser collision call. The source slot distinguishes
// pooled lasers from non-pooled calls; -1 does not identify a particular ECL actor.
struct Laser {
    float center_x = 0, center_y = 0, full_width = 0, full_height = 0;
    float origin_x = 0, origin_y = 0, angle = 0;
    int pooled_slot = -1;
    bool graze_enabled = false;
};

inline std::uint32_t float_bits(float value) {
    if (!std::isfinite(value))
        throw std::runtime_error("non-finite native laser geometry");
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

using LaserKey = std::array<std::uint32_t, 9>;

inline LaserKey laser_key(const Laser &laser) {
    return {float_bits(laser.center_x),   float_bits(laser.center_y),
            float_bits(laser.full_width), float_bits(laser.full_height),
            float_bits(laser.origin_x),   float_bits(laser.origin_y),
            float_bits(laser.angle),      static_cast<std::uint32_t>(laser.pooled_slot),
            laser.graze_enabled ? 1U : 0U};
}

inline std::vector<LaserKey> canonical_lasers(const std::vector<Laser> &lasers) {
    std::vector<LaserKey> result;
    result.reserve(lasers.size());
    for (const auto &laser : lasers)
        result.push_back(laser_key(laser));
    std::sort(result.begin(), result.end());
    return result;
}

struct Frame {
    std::uint64_t update = 0;
    std::vector<Laser> lasers;
};

// Compare a complete requested window, not a surviving prefix. Order of native
// collision calls is irrelevant here, but duplicate calls and float bits remain
// significant. An absent window must never count as agreement.
inline bool same_window(const std::vector<Frame> &left, const std::vector<Frame> &right,
                        std::uint64_t event_update, unsigned span) {
    if (!span || left.size() != span || right.size() != span)
        return false;
    for (unsigned i = 0; i < span; ++i) {
        if (left[i].update != event_update + i || right[i].update != event_update + i ||
            canonical_lasers(left[i].lasers) != canonical_lasers(right[i].lasers))
            return false;
    }
    return true;
}

} // namespace th08::hazard_probe
