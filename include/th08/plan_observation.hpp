#pragma once

#include "modern/headless/session.hpp"
#include <cstdint>
#include <cstring>

namespace th08::plan {

inline bool same_float(float a, float b) {
    std::uint32_t x, y;
    static_assert(sizeof(x) == sizeof(a));
    std::memcpy(&x, &a, sizeof(x));
    std::memcpy(&y, &b, sizeof(y));
    return x == y;
}

// Fork verification promises agreement with the entire public native
// observation, including RNG consumption and lifecycle/feedback state. This is
// an execution check; it is not an independent oracle for the game's physics.
inline bool same_observation(const headless::State &a, const headless::State &b) {
    return a.frame == b.frame && same_float(a.x, b.x) && same_float(a.y, b.y) &&
           same_float(a.deaths, b.deaths) && a.stage == b.stage &&
           a.player_state == b.player_state && a.bullets == b.bullets && a.enemies == b.enemies &&
           a.spell == b.spell && a.spell_active == b.spell_active && a.retry_menu == b.retry_menu &&
           a.stage_complete == b.stage_complete && a.rng_seed == b.rng_seed &&
           a.rng_draws == b.rng_draws && a.score == b.score && a.graze == b.graze &&
           a.gauge == b.gauge && same_float(a.lives, b.lives) &&
           same_float(a.hurt_half_x, b.hurt_half_x) && same_float(a.hurt_half_y, b.hurt_half_y) &&
           a.latched_input == b.latched_input && a.sampled_input == b.sampled_input &&
           a.youkai == b.youkai && a.calculation_updates == b.calculation_updates &&
           a.final_route_selection == b.final_route_selection &&
           a.selected_final_route == b.selected_final_route;
}

} // namespace th08::plan
