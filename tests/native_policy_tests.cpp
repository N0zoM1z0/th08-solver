#include "test_support.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <th08/direct_laser_policy.hpp>
#include <th08/imminent_laser_policy.hpp>
#include <th08/native_policy.hpp>
#include <th08/spell_policy.hpp>
#include <th08/upcoming_bullet_policy.hpp>
#include <vector>

namespace {
struct Bullet {
    float x, y, vx, vy, full_width, full_height;
    int state;
    std::uint32_t active_transforms;
    float vector_acceleration_x, vector_acceleration_y;
    int vector_acceleration_timer, vector_acceleration_duration;
    int wait_linear_updates;
};
struct Laser {
    float origin_x, origin_y, angle;
    float start_offset, end_offset, start_length, width, speed;
    int start_time, hitbox_start_time, duration, despawn_duration, hitbox_end_delay;
    int timer, slot;
    std::uint16_t flags;
    std::uint8_t state;
    float origin_delta_x, origin_delta_y, angle_delta;
    bool motion_observed;
};
struct EclContext {
    int enemy_index, time, next_time, next_opcode;
    int secondary_time, pending_subroutine, active_interpolations, per_frame_ex;
    std::uint16_t operand_flags;
    std::uint32_t enemy_flags;
    bool difficulty_enabled, has_raw_int0, has_parent;
    int raw_int0;
    float position_x, position_y, position_offset_x, position_offset_y;
    float velocity_x, velocity_y, rotation, rotation_velocity;
    float variable0, variable1;
};
struct ImminentSpawn {
    bool supported, suppressed;
    int enemy_index, opcode;
    float origin_x, origin_y, angle;
    float start_offset, end_offset, start_length, width, speed;
    int start_time, hitbox_start_time, duration, despawn_duration, hitbox_end_delay;
    std::uint16_t flags;
};
struct UpcomingSpawn {
    bool supported, suppressed;
    int enemy_index, opcode;
    unsigned update, linear_updates;
    float x, y, vx, vy, full_width, full_height;
};
Laser horizontal_laser() {
    return {0, 380, 0, 0, 400, 400, 2, 0, 0, 0, 100, 0, 0, 0, 7, 1, 1, 0, 0, 0, false};
}
} // namespace

int main() {
    using th08::policy::hazard_reactive;
    using th08::policy::HazardReactiveOptions;
    using th08::policy::HazardReactiveStats;
    using th08::test::check;

    const std::vector<Laser> no_lasers;
    const std::vector<Bullet> no_bullets;
    const std::vector<Bullet> center_bullet{{194, 380, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0}};
    HazardReactiveStats delayed_stats;
    th08::policy::HazardReactiveDecision delayed_decision;
    const auto delayed =
        hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 132, center_bullet, no_lasers,
                        delayed_stats, {3, 0}, &delayed_decision);
    check(delayed == 68, "policy ignored the already-latched rightward movement");
    check(delayed_stats.decisions == 1 && delayed_stats.candidates == 9 &&
              delayed_stats.bullet_projections == 18 && delayed_stats.bullet_checks == 18,
          "bullet proposal costs were not recorded exactly");
    check(delayed_decision.selected_action == delayed && delayed_decision.candidates[3].enabled &&
              delayed_decision.candidates[3].action == 68 &&
              delayed_decision.candidates[3].continuation_action == 68 &&
              delayed_decision.candidates[3].first_overlap == 4,
          "hazard decision diagnostics did not preserve the selected candidate scores");
    HazardReactiveStats maneuver_stats;
    th08::policy::HazardReactiveDecision maneuver_decision;
    const auto maneuver =
        hazard_reactive(192, 380, .825f, .825f, 2, 1.414213538f, 4, center_bullet,
                        std::vector<Laser>{horizontal_laser()}, maneuver_stats,
                        HazardReactiveOptions{3, 3, true, 0x1ff, false, 1}, &maneuver_decision);
    check(maneuver_decision.selected_action == maneuver && maneuver_stats.candidates == 81 &&
              maneuver_stats.bullet_projections == 2 && maneuver_stats.bullet_checks == 162 &&
              maneuver_stats.laser_forecast_updates == 3 && maneuver_stats.laser_paths == 81,
          "two-leg ranking did not share immutable hazard forecasts or report its full cost");
    const std::vector<Bullet> crossing_bullet{{190, 370, -2, 1, 3, 3, 1, 0, 0, 0, 0, 0, 0}};
    HazardReactiveStats turn_stats;
    th08::policy::HazardReactiveDecision turn_decision;
    const auto turn = hazard_reactive(
        192, 380, .825f, .825f, 2, 1.414213538f, 4, crossing_bullet, no_lasers, turn_stats,
        HazardReactiveOptions{6, 0, true, 0x1ff, false, 1}, &turn_decision);
    check(turn == 36 && turn_decision.candidates[7].continuation_action == 132,
          "two-leg ranking collapsed the selected down-then-right path to one direction");

    const std::vector<Bullet> accelerating_bullet{{186, 380, 0, 0, 1, 1, 1, 0x10, 2, 0, 0, 2, 0}};
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
    const Bullet waiting_bullet{192, 404, 0, 4, 24, 24, 1, 0x20000, 0, 0, 0, 0, 4};
    check(th08::policy::detail::project_bullet(waiting_bullet, 4, true, true).kind ==
                  th08::policy::detail::BulletProjectionKind::wait_linear &&
              th08::policy::detail::project_bullet(waiting_bullet, 5, true, true).kind ==
                  th08::policy::detail::BulletProjectionKind::unsupported,
          "WAIT projection crossed its source-owned linear-motion bound");
    HazardReactiveStats wait_stats;
    const auto wait_escape = hazard_reactive(
        197.65686f, 432, .825f, .825f, 2, 1.414213538f, 164, std::vector<Bullet>{waiting_bullet},
        no_lasers, wait_stats, HazardReactiveOptions{12, 0, true, 0x1ff, false, 0, true});
    check(wait_escape == 132 && wait_stats.wait_linear_checks == 27 &&
              wait_stats.unsupported_transform_checks == 72,
          "bounded WAIT projection lost the measured pure-right escape");
    check(!th08::policy::native_spell_policy(199).hazards.vector_acceleration &&
              th08::policy::native_spell_policy(193).hazards.vector_acceleration,
          "spell portfolio lost its isolated ID199 model selection");
    check(th08::policy::native_spell_policy(201).hazards.bullet_horizon == 32 &&
              th08::policy::native_spell_policy(201).hazards.wait_linear_projection &&
              th08::policy::native_spell_policy(201).upcoming_ecl_bullets,
          "spell portfolio lost the isolated ID201 WAIT projection");
    const auto id202 = th08::policy::native_spell_policy(202);
    check(id202.hazards.bullet_horizon == 12 && id202.hazards.wait_linear_projection &&
              !id202.upcoming_ecl_bullets && !id202.hazards.first_leg_updates,
          "ID202 crossed its observed-WAIT-only profile boundary");
    // Native update 3264: this already-born random child has six proven linear
    // updates left. No future random angle is needed to reject staying still.
    const Bullet id202_child{
        187.367584f, 429.627258f, 2.10032201f, -.317858994f, 4, 4, 1, 0x20000, 0, 0, 0, 0, 6};
    HazardReactiveStats id202_stats;
    th08::policy::HazardReactiveDecision id202_decision;
    hazard_reactive(192.568558f, 426.34314f, .825f, .825f, 2, 1.414213538f, 4,
                    std::vector<Bullet>{id202_child}, no_lasers, id202_stats, id202.hazards,
                    &id202_decision);
    check(id202_decision.candidates[4].first_overlap == 2 &&
              id202_decision.candidates[1].first_overlap > 2 &&
              th08::policy::detail::project_bullet(id202_child, 7, true, true).kind ==
                  th08::policy::detail::BulletProjectionKind::unsupported,
          "ID202 lost its source-bounded stationary collision and upward escape");
    check(th08::policy::native_spell_policy(85).hazards.rigid_laser_motion &&
              th08::policy::native_spell_policy(198).hazards.rigid_laser_motion,
          "spell portfolio lost an isolated pooled-laser motion model");

    const std::vector<ImminentSpawn> imminent_spawns{{true, false, 3, 114, 7.35971069f, 448,
                                                      -1.56775308f, 0, 0, 2400, 16, 30, 1, 1, 40,
                                                      40, 20, 6}};
    std::vector<th08::policy::ImminentLaserWarning> imminent_warnings;
    th08::policy::ImminentLaserStats imminent_stats;
    check(th08::policy::collect_imminent_laser_warnings(imminent_spawns, imminent_warnings,
                                                        imminent_stats),
          "literal imminent pooled-laser spawn was rejected");
    const auto imminent_mask = th08::policy::imminent_laser_candidate_mask(
        8.82842636f, 426.34314f, .825f, .825f, 2, 1.414213538f, 20, 120, imminent_warnings,
        imminent_stats);
    check(imminent_mask == (1u << 5) && imminent_stats.due_spawns == 1 &&
              imminent_stats.warnings == 1 && imminent_stats.constrained_decisions == 1,
          "imminent pooled-laser warning lost the only native-observed rightward escape");
    auto unsupported_spawn = imminent_spawns.front();
    unsupported_spawn.supported = false;
    check(!th08::policy::collect_imminent_laser_warnings(
              std::vector<ImminentSpawn>{unsupported_spawn}, imminent_warnings, imminent_stats) &&
              imminent_stats.unsupported_spawns == 1,
          "unsupported imminent pooled-laser geometry did not stop warning collection");
    check(th08::policy::native_spell_policy(93).imminent_pooled_lasers,
          "spell portfolio lost the isolated ID93 spawn-warning adapter");

    // Enemy-order observations need not be chronological. The harmless update-2
    // spawn follows the lethal update-12 spawn to exercise sorting and path reuse.
    const std::vector<UpcomingSpawn> upcoming_spawns{
        {true, false, 22, 97, 12, 1, 200, 424, 0, 4, 100, 24},
        {true, false, 23, 99, 2, 1, -1000, -1000, 0, 0, 1, 1}};
    std::vector<th08::policy::UpcomingBulletWarning> upcoming_warnings;
    th08::policy::UpcomingBulletStats upcoming_stats;
    check(th08::policy::collect_upcoming_bullet_warnings(upcoming_spawns, upcoming_warnings,
                                                         upcoming_stats),
          "source-bounded ECL bullet spawn was rejected");
    const auto upcoming_mask = th08::policy::upcoming_bullet_candidate_mask(
        200, 432, .825f, .825f, 2, 1.414213538f, 4, 12, upcoming_warnings, upcoming_stats);
    check(upcoming_mask == (1u << 1) && upcoming_stats.observed_spawns == 2 &&
              upcoming_stats.warnings == 2 && upcoming_stats.candidate_checks == 18 &&
              upcoming_stats.candidate_updates == 108 && upcoming_stats.candidate_overlaps == 8 &&
              upcoming_stats.constrained_decisions == 1,
          "upcoming ECL bullet warning lost its only pure-up escape");
    auto unknown_bullet_spawn = upcoming_spawns.front();
    unknown_bullet_spawn.supported = false;
    th08::policy::UpcomingBulletStats unsupported_bullet_stats;
    check(!th08::policy::collect_upcoming_bullet_warnings(
              std::vector<UpcomingSpawn>{unknown_bullet_spawn}, upcoming_warnings,
              unsupported_bullet_stats) &&
              unsupported_bullet_stats.unsupported_spawns == 1,
          "unsupported ECL bullet future did not stop warning collection");

    EclContext direct_context{2,   110, 120, 137,  0,    -1,    0,
                              -1,  0,   0,   true, true, false, 9,
                              262, 384, 0,   0,    0,    0,     1.57079632679489661923f,
                              0,   0,   295};
    std::vector<th08::policy::DirectLaserWarning> warnings;
    th08::policy::DirectLaserStats direct_stats;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{direct_context}, 120,
                                                warnings, direct_stats);
    check(warnings.size() == 1 && warnings[0].first_update == 11 &&
              warnings[0].last_update == 120 && warnings[0].full_height == 160,
          "constant ECL opcode 137 warning was not decoded at its native boundary");
    auto disabling_context = direct_context;
    disabling_context.per_frame_ex = 9;
    disabling_context.raw_int0 = -1;
    th08::policy::DirectLaserStats disable_stats;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{disabling_context}, 120,
                                                warnings, disable_stats);
    check(warnings.size() == 1 && warnings[0].first_update == 1 && warnings[0].last_update == 10,
          "repeating ECL laser warning crossed its source disable update");
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{direct_context}, 120,
                                                warnings, direct_stats);
    const auto direct_mask = th08::policy::direct_laser_candidate_mask(
        192, 414, .825f, .825f, 2, 1.414213538f, 36, warnings, direct_stats);
    check(direct_mask == 73 && direct_stats.candidate_overlaps > 0 &&
              direct_stats.constrained_decisions == 1 && direct_stats.allowed_candidates == 3,
          "direct ECL laser warning did not retain exactly its three safe escapes");
    HazardReactiveStats constrained_stats;
    const auto constrained =
        hazard_reactive(192, 414, .825f, .825f, 2, 1.414213538f, 36, no_bullets, no_lasers,
                        constrained_stats, HazardReactiveOptions{0, 0, true, direct_mask});
    check((constrained == 84 || constrained == 68 || constrained == 100) &&
              constrained_stats.candidates == 3,
          "generic hazard ranking ignored an independently derived candidate mask");
    auto variable_context = direct_context;
    variable_context.operand_flags = 1;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{variable_context}, 120,
                                                warnings, direct_stats);
    check(warnings.empty() && direct_stats.variable_selectors == 1,
          "variable ECL selector was silently treated as a constant future");
    auto moving_context = direct_context;
    moving_context.velocity_x = 1;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{moving_context}, 120,
                                                warnings, direct_stats);
    check(warnings.empty() && direct_stats.dynamic_geometry == 1,
          "moving direct-laser geometry was projected without an owned motion model");
    check(th08::policy::native_spell_policy(89).direct_ecl_lasers,
          "spell portfolio lost the isolated ID89 ECL warning adapter");

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

    auto rotating_view = horizontal_laser();
    rotating_view.origin_x = 1;
    rotating_view.origin_y = 0;
    rotating_view.origin_delta_x = 1;
    rotating_view.origin_delta_y = 1;
    rotating_view.angle_delta = 1.57079632679489661923f;
    rotating_view.motion_observed = true;
    auto rotating = th08::policy::detail::forecast(rotating_view, true);
    th08::policy::detail::advance(rotating);
    check(std::abs(rotating.origin_x) < 1e-5f && std::abs(rotating.origin_y - 1) < 1e-5f &&
              std::abs(rotating.angle - rotating_view.angle_delta) < 1e-6f,
          "rigid pooled-laser motion did not preserve its observed rotation center");
    auto pivoting_view = horizontal_laser();
    pivoting_view.origin_x = 192;
    pivoting_view.origin_y = 128;
    pivoting_view.angle_delta = .0078539816f;
    pivoting_view.motion_observed = true;
    auto pivoting = th08::policy::detail::forecast(pivoting_view, true);
    th08::policy::detail::advance(pivoting);
    check(std::abs(pivoting.origin_x - pivoting_view.origin_x) < 1e-5f &&
              std::abs(pivoting.origin_y - pivoting_view.origin_y) < 1e-5f &&
              std::abs(pivoting.angle - pivoting_view.angle_delta) < 1e-6f,
          "fixed-origin pooled-laser rotation did not preserve its native pivot");
    HazardReactiveStats rotating_stats;
    hazard_reactive(4, 4, .825f, .825f, 2, 1.414213538f, 4, no_bullets,
                    std::vector<Laser>{rotating_view}, rotating_stats,
                    HazardReactiveOptions{0, 2, true, 0x1ff, true});
    check(rotating_stats.rigid_laser_paths == 9 && rotating_stats.laser_paths_pruned == 0,
          "moving pooled lasers were not measured or bypassed the rotating forecast");

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
                 "\"wait_projection\":\"covered\",\"upcoming_ecl_bullet\":\"covered\","
                 "\"direct_ecl_laser\":\"covered\",\"spell_portfolio\":\"covered\"}\n";
}
