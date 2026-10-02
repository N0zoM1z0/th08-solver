#include "test_support.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <th08/native_policy.hpp>
#include <th08/spell_policy.hpp>
#include <vector>

namespace {
struct Bullet {
    float x, y, vx, vy, full_width, full_height;
    int state;
    std::uint32_t active_transforms;
    float vector_acceleration_x, vector_acceleration_y;
    int vector_acceleration_timer, vector_acceleration_duration;
};
struct Laser {
    float origin_x, origin_y, angle;
    float start_offset, end_offset, start_length, width, speed;
    int start_time, hitbox_start_time, duration, despawn_duration, hitbox_end_delay;
    int timer, slot;
    std::uint16_t flags;
    std::uint8_t state;
};
Laser horizontal_laser() {
    return {0, 380, 0, 0, 400, 400, 2, 0, 0, 0, 100, 0, 0, 0, 7, 1, 1};
}
} // namespace

int main() {
    using th08::policy::hazard_reactive;
    using th08::policy::HazardReactiveOptions;
    using th08::policy::HazardReactiveStats;
    using th08::test::check;

    const std::vector<Laser> no_lasers;
    const std::vector<Bullet> center_bullet{{194, 380, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}};
    HazardReactiveStats delayed_stats;
    const auto delayed = hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 132,
                                         center_bullet, no_lasers, delayed_stats, {3, 0});
    check(delayed == 68, "policy ignored the already-latched rightward movement");
    check(delayed_stats.decisions == 1 && delayed_stats.candidates == 9 &&
              delayed_stats.bullet_checks == 18,
          "bullet proposal costs were not recorded exactly");

    const std::vector<Bullet> accelerating_bullet{{186, 380, 0, 0, 1, 1, 1, 0x10, 2, 0, 0, 2}};
    HazardReactiveStats acceleration_stats;
    const auto accelerated =
        hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 4, accelerating_bullet, no_lasers,
                        acceleration_stats, {3, 0});
    check(accelerated == 68, "active vector acceleration was projected as constant velocity");
    check(acceleration_stats.vector_acceleration_checks == 18 &&
              acceleration_stats.unsupported_transform_checks == 0,
          "vector-acceleration coverage was not reported exactly");
    HazardReactiveStats linear_stats;
    const auto linear =
        hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 4, accelerating_bullet, no_lasers,
                        linear_stats, {3, 0, false});
    check(linear != accelerated && linear_stats.vector_acceleration_checks == 0 &&
              linear_stats.unsupported_transform_checks == 18,
          "vector-acceleration ablation did not retain the explicit unknown model");
    check(!th08::policy::native_spell_policy(199).hazards.vector_acceleration &&
              th08::policy::native_spell_policy(193).hazards.vector_acceleration,
          "spell portfolio lost its isolated ID199 model selection");

    const std::vector<Bullet> no_bullets;
    const std::vector<Laser> laser{horizontal_laser()};
    HazardReactiveStats laser_stats;
    const auto dodge = hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 4, no_bullets,
                                       laser, laser_stats, HazardReactiveOptions{0, 3});
    check(dodge == 20, "horizontal laser did not select the deterministic upward escape");
    check(laser_stats.laser_paths == 9 && laser_stats.laser_paths_pruned > 0 &&
              laser_stats.laser_checks > 0,
          "laser broad phase or accounting was bypassed");

    auto forecast = th08::policy::detail::forecast(horizontal_laser());
    forecast.state = 0;
    forecast.timer = 0;
    forecast.start_time = 2;
    forecast.hitbox_start_time = 1;
    auto phase = th08::policy::detail::advance(forecast);
    check(!phase.lethal && forecast.timer == 1 && forecast.state == 0,
          "starting laser became lethal one update early");
    phase = th08::policy::detail::advance(forecast);
    check(phase.lethal && forecast.timer == 2 && forecast.state == 0,
          "starting laser missed its hitbox activation update");
    phase = th08::policy::detail::advance(forecast);
    check(phase.lethal && forecast.timer == 1 && forecast.state == 1,
          "laser transition did not preserve source fallthrough and timer order");

    auto terminal = th08::policy::detail::forecast(horizontal_laser());
    terminal.timer = terminal.duration;
    phase = th08::policy::detail::advance(terminal);
    check(phase.present && phase.lethal && !terminal.present,
          "final active laser collision was removed one update early");
    phase = th08::policy::detail::advance(terminal);
    check(!phase.present, "removed laser produced another forecast phase");

    auto fallthrough = th08::policy::detail::forecast(horizontal_laser());
    fallthrough.timer = fallthrough.duration;
    fallthrough.despawn_duration = 10;
    fallthrough.hitbox_end_delay = 0;
    phase = th08::policy::detail::advance(fallthrough);
    check(phase.lethal && fallthrough.present && fallthrough.state == 2 && fallthrough.timer == 1,
          "active-to-despawn fallthrough discarded the active collision");

    const auto straight = th08::policy::detail::forecast(horizontal_laser());
    check(th08::policy::detail::laser_path_may_overlap(192, 380, 192, 420, .825f, straight),
          "laser broad phase rejected a crossing path");
    check(!th08::policy::detail::laser_path_may_overlap(192, 400, 300, 420, .825f, straight),
          "laser broad phase retained a separated path");
    auto vertical_view = horizontal_laser();
    vertical_view.origin_x = 192;
    vertical_view.origin_y = 0;
    vertical_view.angle = 1.57079632679489661923f;
    const auto vertical = th08::policy::detail::forecast(vertical_view);
    check(th08::policy::detail::laser_path_may_overlap(192, 300, 192, 400, .825f, vertical) &&
              !th08::policy::detail::laser_path_may_overlap(220, 300, 240, 400, .825f, vertical),
          "rotated laser broad phase used the wrong coordinate axis");

    std::cout << "{\"input_latch\":\"covered\",\"vector_acceleration\":\"covered\","
                 "\"laser_lifecycle\":\"covered\",\"broad_phase\":\"covered\","
                 "\"spell_portfolio\":\"covered\"}\n";
}
