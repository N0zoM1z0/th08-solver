#pragma once

#include "native_policy.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>

namespace th08::policy {
struct UpcomingBulletWarning {
    unsigned update;
    float x, y, full_width, full_height;
    int enemy_index, opcode;
};

struct UpcomingBulletStats {
    std::uint64_t decisions = 0, observed_spawns = 0, warnings = 0;
    std::uint64_t suppressed_spawns = 0, unsupported_spawns = 0;
    std::uint64_t candidate_paths = 0, candidate_updates = 0, candidate_checks = 0;
    std::uint64_t candidate_overlaps = 0, constrained_decisions = 0, allowed_candidates = 0;
};

// Convert only source-decoded, action-independent ECL bullet observations.
// An unsupported entry makes an enabled spell profile stop at the boundary.
template <class Spawns>
bool collect_upcoming_bullet_warnings(const Spawns &spawns,
                                      std::vector<UpcomingBulletWarning> &warnings,
                                      UpcomingBulletStats &stats) {
    warnings.clear();
    for (const auto &spawn : spawns) {
        ++stats.observed_spawns;
        if (!spawn.supported) {
            ++stats.unsupported_spawns;
            return false;
        }
        if (spawn.suppressed) {
            ++stats.suppressed_spawns;
            continue;
        }
        for (unsigned offset = 0; offset < spawn.linear_updates; ++offset) {
            warnings.push_back({spawn.update + offset, spawn.x + float(offset) * spawn.vx,
                                spawn.y + float(offset) * spawn.vy, spawn.full_width,
                                spawn.full_height, spawn.enemy_index, spawn.opcode});
            ++stats.warnings;
        }
    }
    std::sort(warnings.begin(), warnings.end(),
              [](const auto &left, const auto &right) { return left.update < right.update; });
    return true;
}

// Return one bit per dy-major direction. The already-latched direction owns
// update one; the candidate owns update two onward, matching native input lag.
inline std::uint16_t
upcoming_bullet_candidate_mask(float player_x, float player_y, float half_x, float half_y,
                               float axis, float diagonal, std::uint16_t latched_input,
                               unsigned horizon, const std::vector<UpcomingBulletWarning> &warnings,
                               UpcomingBulletStats &stats) {
    ++stats.decisions;
    if (warnings.empty() || !horizon)
        return 0x1ff;

    struct Score {
        unsigned overlaps = 0;
        float minimum_clearance = std::numeric_limits<float>::infinity();
    };
    const auto pending = detail::direction(latched_input);
    auto score = [&](detail::Direction candidate) {
        ++stats.candidate_paths;
        float x = player_x, y = player_y;
        unsigned simulated_update = 0;
        Score result;
        for (const auto &warning : warnings) {
            if (warning.update > horizon)
                break;
            while (simulated_update < warning.update) {
                ++simulated_update;
                detail::advance(x, y, simulated_update == 1 ? pending : candidate, axis, diagonal);
                ++stats.candidate_updates;
            }
            const float clearance =
                detail::box_clearance(x, y, half_x, half_y, warning.x, warning.y,
                                      warning.full_width, warning.full_height);
            result.minimum_clearance = std::min(result.minimum_clearance, clearance);
            ++stats.candidate_checks;
            if (clearance <= 0.f) {
                ++result.overlaps;
                ++stats.candidate_overlaps;
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
