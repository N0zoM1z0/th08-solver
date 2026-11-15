#pragma once

#include <cmath>
#include <cstdint>
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

} // namespace th08::policy
