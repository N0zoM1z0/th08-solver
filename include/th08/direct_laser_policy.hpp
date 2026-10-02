#pragma once

#include "native_policy.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace th08::policy {
struct DirectLaserWarning {
    unsigned first_update, last_update;
    float center_x, center_y, full_width, full_height;
    float origin_x, origin_y, angle;
    int enemy_index, ex_index;
};

struct DirectLaserStats {
    std::uint64_t decisions = 0, contexts = 0, upcoming_calls = 0;
    std::uint64_t usable_warnings = 0, variable_selectors = 0, dynamic_geometry = 0;
    std::uint64_t candidate_overlaps = 0, constrained_decisions = 0, allowed_candidates = 0;
};

namespace detail {
inline float direct_laser_height(int ex_index) {
    // g_EclExInsn indices 9, 11 and 25 own the three source hitbox widths.
    return ex_index == 9 ? 160.f : ex_index == 11 ? 240.f : ex_index == 25 ? 288.f : 0.f;
}

template <class Context>
bool make_direct_laser_warning(const Context &context, int ex_index, unsigned first_update,
                               unsigned last_update, DirectLaserWarning &warning,
                               DirectLaserStats &stats) {
    constexpr std::uint32_t movement_mode_mask = 3u << 12;
    const float height = direct_laser_height(ex_index);
    if (!height)
        return false;
    ++stats.upcoming_calls;
    const bool stable =
        context.secondary_time == 0 && context.pending_subroutine < 0 &&
        context.active_interpolations == 0 && !context.has_parent &&
        (context.enemy_flags & movement_mode_mask) == 0 && std::abs(context.velocity_x) <= 1e-6f &&
        std::abs(context.velocity_y) <= 1e-6f && std::abs(context.rotation_velocity) <= 1e-6f;
    if (!stable) {
        ++stats.dynamic_geometry;
        return false;
    }
    // Narrow/wide callbacks use worldPosition; medium uses position. Under the
    // currently observed motion state, the next RunEcl world position is the
    // current position plus offset. The adapter is refreshed every update; it
    // does not claim that an uninspected future instruction preserves geometry.
    // The callbacks then subtract context variables 0/1.
    const bool use_world_position = ex_index != 11;
    const float origin_x = context.position_x +
                           (use_world_position ? context.position_offset_x : 0.f) -
                           context.variable0;
    const float origin_y = context.position_y +
                           (use_world_position ? context.position_offset_y : 0.f) -
                           context.variable1;
    warning = {first_update, last_update, origin_x + 295.f, origin_y,         590.f,
               height,       origin_x,    origin_y,         context.rotation, context.enemy_index,
               ex_index};
    ++stats.usable_warnings;
    return true;
}

inline float direct_laser_clearance(float x, float y, float half_x, float half_y,
                                    const DirectLaserWarning &warning) {
    const float sine = std::sin(-warning.angle), cosine = std::cos(-warning.angle);
    const float relative_x = x - warning.origin_x, relative_y = y - warning.origin_y;
    const float local_x = cosine * relative_x - sine * relative_y + warning.origin_x;
    const float local_y = sine * relative_x + cosine * relative_y + warning.origin_y;
    return box_clearance(local_x, local_y, half_x, half_y, warning.center_x, warning.center_y,
                         warning.full_width, warning.full_height);
}
} // namespace detail

// Only the current instruction is inspected. This deliberately does not scan
// across jumps/calls or resolve variable operands into one assumed future.
template <class Contexts>
void collect_direct_laser_warnings(const Contexts &contexts, unsigned horizon,
                                   std::vector<DirectLaserWarning> &warnings,
                                   DirectLaserStats &stats) {
    constexpr int call_ex = 136, set_repeating_ex = 137;
    warnings.clear();
    for (const auto &context : contexts) {
        ++stats.contexts;
        if (context.per_frame_ex >= 0) {
            unsigned last_update = horizon;
            // A constant -1 opcode 137 disables the installed callback before
            // the per-frame tail on that update.
            if (context.next_opcode == set_repeating_ex && context.difficulty_enabled &&
                context.has_raw_int0 && !(context.operand_flags & 1) && context.raw_int0 < 0 &&
                context.next_time >= context.time && context.secondary_time == 0) {
                const auto disable_update = unsigned(context.next_time - context.time) + 1;
                last_update = std::min(last_update, disable_update - 1);
            }
            DirectLaserWarning warning{};
            if (last_update >= 1 &&
                detail::make_direct_laser_warning(context, context.per_frame_ex, 1, last_update,
                                                  warning, stats))
                warnings.push_back(warning);
        }
        if ((context.next_opcode != call_ex && context.next_opcode != set_repeating_ex) ||
            !context.difficulty_enabled)
            continue;
        if (!context.has_raw_int0 || (context.operand_flags & 1)) {
            ++stats.variable_selectors;
            continue;
        }
        if (context.next_time < context.time)
            continue;
        const auto updates_until = unsigned(context.next_time - context.time) + 1;
        if (updates_until > horizon)
            continue;
        DirectLaserWarning warning{};
        const unsigned last_update =
            context.next_opcode == set_repeating_ex ? horizon : updates_until;
        if (detail::make_direct_laser_warning(context, context.raw_int0, updates_until, last_update,
                                              warning, stats))
            warnings.push_back(warning);
    }
}

// Return one bit per dy-major direction in [-1, 1]^2. The generic hazard
// scorer can then rank bullets and pooled lasers without selecting a path that
// re-enters a source-derived direct hitbox interval.
inline std::uint16_t direct_laser_candidate_mask(float player_x, float player_y, float half_x,
                                                 float half_y, float axis, float diagonal,
                                                 std::uint16_t latched_input,
                                                 const std::vector<DirectLaserWarning> &warnings,
                                                 DirectLaserStats &stats) {
    ++stats.decisions;
    if (warnings.empty())
        return 0x1ff;

    struct Score {
        unsigned overlaps = 0;
        float minimum_clearance = std::numeric_limits<float>::infinity();
    };
    auto score = [&](detail::Direction candidate) {
        Score result;
        const auto pending = detail::direction(latched_input);
        for (const auto &warning : warnings) {
            float x = player_x, y = player_y;
            for (unsigned step = 1; step <= warning.last_update; ++step) {
                detail::advance(x, y, step == 1 ? pending : candidate, axis, diagonal);
                if (step < warning.first_update)
                    continue;
                const float clearance =
                    detail::direct_laser_clearance(x, y, half_x, half_y, warning);
                result.minimum_clearance = std::min(result.minimum_clearance, clearance);
                result.overlaps += clearance <= 0;
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
            stats.candidate_overlaps += scores[index].overlaps;
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
