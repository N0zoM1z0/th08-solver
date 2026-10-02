#pragma once

#include "native_policy.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace th08::policy {
struct ImminentLaserWarning {
    detail::LaserForecast laser;
    int enemy_index;
};

struct ImminentLaserStats {
    std::uint64_t decisions = 0, due_spawns = 0, warnings = 0;
    std::uint64_t suppressed_spawns = 0, unsupported_spawns = 0;
    std::uint64_t candidate_paths = 0, forecast_updates = 0, candidate_overlaps = 0;
    std::uint64_t constrained_decisions = 0, allowed_candidates = 0;
};

// Convert only source-decoded, action-independent opcode-114 observations.
// Returning false makes an enabled spell adapter stop on unsupported geometry.
template <class Spawns>
bool collect_imminent_laser_warnings(const Spawns &spawns,
                                     std::vector<ImminentLaserWarning> &warnings,
                                     ImminentLaserStats &stats) {
    warnings.clear();
    for (const auto &spawn : spawns) {
        ++stats.due_spawns;
        if (!spawn.supported) {
            ++stats.unsupported_spawns;
            return false;
        }
        if (spawn.suppressed) {
            ++stats.suppressed_spawns;
            continue;
        }
        detail::LaserForecast laser{};
        laser.origin_x = spawn.origin_x;
        laser.origin_y = spawn.origin_y;
        laser.angle = spawn.angle;
        laser.rotation_sine = std::sin(-spawn.angle);
        laser.rotation_cosine = std::cos(-spawn.angle);
        laser.start_offset = spawn.start_offset;
        laser.end_offset = spawn.end_offset;
        laser.start_length = spawn.start_length;
        laser.width = spawn.width;
        laser.speed = spawn.speed;
        laser.start_time = spawn.start_time;
        laser.hitbox_start_time = spawn.hitbox_start_time;
        laser.duration = spawn.duration;
        laser.despawn_duration = spawn.despawn_duration;
        laser.hitbox_end_delay = spawn.hitbox_end_delay;
        laser.timer = 0;
        laser.state = spawn.start_time == 0 ? 1 : 0;
        laser.flags = spawn.flags;
        laser.present = true;
        warnings.push_back({laser, spawn.enemy_index});
        ++stats.warnings;
    }
    return true;
}

// Rank the same nine constant directions as the generic hazard policy. The
// first update uses already-latched movement; the proposed direction begins on
// the second update, matching native input latency and same-update laser spawn.
inline std::uint16_t
imminent_laser_candidate_mask(float player_x, float player_y, float half_x, float half_y,
                              float axis, float diagonal, std::uint16_t latched_input,
                              unsigned horizon, const std::vector<ImminentLaserWarning> &warnings,
                              ImminentLaserStats &stats) {
    ++stats.decisions;
    if (warnings.empty() || horizon == 0)
        return 0x1ff;

    struct Score {
        unsigned overlaps = 0;
        float minimum_clearance = std::numeric_limits<float>::infinity();
    };
    const auto pending = detail::direction(latched_input);
    auto score = [&](detail::Direction candidate) {
        Score result;
        for (const auto &warning : warnings) {
            ++stats.candidate_paths;
            auto laser = warning.laser;
            float x = player_x, y = player_y;
            for (unsigned step = 1; step <= horizon && laser.present; ++step) {
                detail::advance(x, y, step == 1 ? pending : candidate, axis, diagonal);
                const auto phase = detail::advance(laser);
                ++stats.forecast_updates;
                if (!phase.present || !phase.lethal)
                    continue;
                const float clearance = detail::laser_clearance(x, y, half_x, half_y, laser, phase);
                result.minimum_clearance = std::min(result.minimum_clearance, clearance);
                if (clearance <= 0) {
                    ++result.overlaps;
                    ++stats.candidate_overlaps;
                }
            }
        }
        return result;
    };

    std::array<Score, 9> scores;
    unsigned best_overlaps = std::numeric_limits<unsigned>::max();
    float best_clearance = -std::numeric_limits<float>::infinity();
    unsigned index = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            scores[index] = score({dx, dy});
            if (scores[index].overlaps < best_overlaps) {
                best_overlaps = scores[index].overlaps;
                best_clearance = scores[index].minimum_clearance;
            } else if (scores[index].overlaps == best_overlaps) {
                best_clearance = std::max(best_clearance, scores[index].minimum_clearance);
            }
            ++index;
        }

    std::uint16_t mask = 0;
    for (index = 0; index < scores.size(); ++index)
        if (scores[index].overlaps == best_overlaps &&
            (!best_overlaps || scores[index].minimum_clearance == best_clearance)) {
            mask |= std::uint16_t(1u << index);
            ++stats.allowed_candidates;
        }
    if (mask != 0x1ff)
        ++stats.constrained_decisions;
    return mask;
}
} // namespace th08::policy
