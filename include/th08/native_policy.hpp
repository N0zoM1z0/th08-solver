#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace th08::policy {
struct HazardReactiveOptions {
    unsigned bullet_horizon = 12;
    unsigned laser_horizon = 120;
    bool vector_acceleration = true;
    // dy-major bits for the nine constant directions; callers must retain at
    // least one candidate when composing an independently derived constraint.
    std::uint16_t candidate_mask = 0x1ff;
    // A spell adapter may extrapolate the most recently observed native pooled
    // laser translation/rotation as one rigid motion. The default makes no
    // future-motion claim.
    bool rigid_laser_motion = false;
};
struct HazardReactiveStats {
    std::uint64_t decisions = 0, candidates = 0;
    std::uint64_t bullet_checks = 0, vector_acceleration_checks = 0;
    std::uint64_t unsupported_transform_checks = 0, laser_paths = 0, laser_paths_pruned = 0;
    std::uint64_t rigid_laser_paths = 0, laser_checks = 0, predicted_overlaps = 0;
};
// Optional diagnostics copy the exact scalar ranking inputs without retaining
// observed hazards or changing proposal evaluation. Masked entries still expose
// their action so a trace can distinguish disabled from unevaluated directions.
struct HazardCandidateEvaluation {
    bool enabled = false;
    std::uint16_t action = 4;
    unsigned first_overlap = 0;
    float minimum_clearance = 0;
    double danger = 0, center_distance = 0;
};
struct HazardReactiveDecision {
    std::array<HazardCandidateEvaluation, 9> candidates{};
    std::uint16_t selected_action = 4;
};

namespace detail {
struct Direction {
    int x, y;
};
inline Direction direction(std::uint16_t input) {
    return {input & 64 ? -1 : input & 128 ? 1 : 0, input & 16 ? -1 : input & 32 ? 1 : 0};
}
inline void advance(float &x, float &y, Direction direction, float axis, float diagonal) {
    const float speed = direction.x && direction.y ? diagonal : axis;
    x = std::clamp(x + direction.x * speed, 8.f, 376.f);
    y = std::clamp(y + direction.y * speed, 16.f, 432.f);
}
inline float box_clearance(float x, float y, float half_x, float half_y, float center_x,
                           float center_y, float full_width, float full_height) {
    const float gap_x = std::abs(x - center_x) - half_x - std::abs(full_width) / 2;
    const float gap_y = std::abs(y - center_y) - half_y - std::abs(full_height) / 2;
    return std::max(gap_x, gap_y);
}
enum class BulletProjectionKind { linear, vector_acceleration, unsupported };
struct BulletProjection {
    float x, y;
    BulletProjectionKind kind;
};
// BulletManager updates active vector acceleration before adding velocity to
// position. This closed form mirrors that order for an already-active opcode.
// Other active transforms remain soft ranking evidence rather than collision proof.
template <class Bullet>
BulletProjection project_bullet(const Bullet &bullet, unsigned step,
                                bool enable_vector_acceleration = true) {
    constexpr std::uint32_t vector_acceleration = 0x10;
    if (enable_vector_acceleration && bullet.active_transforms == vector_acceleration) {
        const int remaining_frames =
            std::max(0, bullet.vector_acceleration_duration - bullet.vector_acceleration_timer);
        const unsigned accelerated = std::min(step, unsigned(remaining_frames));
        const float accelerated_updates = float(accelerated);
        const float coefficient =
            accelerated_updates * float(step) - accelerated_updates * (accelerated_updates - 1) / 2;
        return {bullet.x + step * bullet.vx + coefficient * bullet.vector_acceleration_x,
                bullet.y + step * bullet.vy + coefficient * bullet.vector_acceleration_y,
                BulletProjectionKind::vector_acceleration};
    }
    return {bullet.x + step * bullet.vx, bullet.y + step * bullet.vy,
            bullet.active_transforms ? BulletProjectionKind::unsupported
                                     : BulletProjectionKind::linear};
}
inline std::uint16_t input(Direction direction) {
    return std::uint16_t(4 |
                         (direction.x < 0   ? 64
                          : direction.x > 0 ? 128
                                            : 0) |
                         (direction.y < 0   ? 16
                          : direction.y > 0 ? 32
                                            : 0));
}
struct LaserForecast {
    float origin_x, origin_y, angle;
    float rotation_sine, rotation_cosine;
    float start_offset, end_offset, start_length, width, speed;
    int start_time, hitbox_start_time, duration, despawn_duration, hitbox_end_delay;
    int timer, state;
    std::uint16_t flags;
    bool present = true;
    float motion_center_x = 0, motion_center_y = 0, angle_delta = 0;
    bool rigid_motion = false;
};
struct LaserPhase {
    float center_x = 0, center_y = 0, full_width = 0, full_height = 0;
    // present describes this update's geometry. The forecast may become absent
    // for the next update after native collision has already run.
    bool lethal = false, present = true;
};
template <class Laser> LaserForecast forecast(const Laser &laser, bool rigid_motion = false) {
    LaserForecast result{laser.origin_x,
                         laser.origin_y,
                         laser.angle,
                         std::sin(-laser.angle),
                         std::cos(-laser.angle),
                         laser.start_offset,
                         laser.end_offset,
                         laser.start_length,
                         laser.width,
                         laser.speed,
                         laser.start_time,
                         laser.hitbox_start_time,
                         laser.duration,
                         laser.despawn_duration,
                         laser.hitbox_end_delay,
                         laser.timer,
                         laser.state,
                         laser.flags,
                         true};
    if (!rigid_motion || !laser.motion_observed || std::abs(laser.angle_delta) <= 1e-6f)
        return result;
    const float sine = std::sin(laser.angle_delta), cosine = std::cos(laser.angle_delta);
    const float previous_x = laser.origin_x - laser.origin_delta_x;
    const float previous_y = laser.origin_y - laser.origin_delta_y;
    // Solve p1 - center = R(delta) * (p0 - center). This owns only the
    // proposal geometry; native updates still decide whether motion repeats.
    const float rhs_x = laser.origin_x - (cosine * previous_x - sine * previous_y);
    const float rhs_y = laser.origin_y - (sine * previous_x + cosine * previous_y);
    const float diagonal = 1 - cosine;
    const float determinant = diagonal * diagonal + sine * sine;
    if (determinant <= 1e-12f)
        return result;
    result.motion_center_x = (diagonal * rhs_x - sine * rhs_y) / determinant;
    result.motion_center_y = (sine * rhs_x + diagonal * rhs_y) / determinant;
    result.angle_delta = laser.angle_delta;
    result.rigid_motion = std::isfinite(result.motion_center_x) &&
                          std::isfinite(result.motion_center_y) &&
                          std::isfinite(result.angle_delta);
    return result;
}
// Mirrors the existing-laser geometry and lifecycle order in BulletManager::OnUpdate.
// It deliberately cannot predict future spawns or player-aimed creation.
inline LaserPhase advance(LaserForecast &laser) {
    if (!laser.present) {
        LaserPhase absent;
        absent.present = false;
        return absent;
    }
    if (laser.rigid_motion) {
        // ECL changes pooled-laser position/angle before BulletManager advances
        // lifecycle and submits collision geometry in the native calc chain.
        const float sine = std::sin(laser.angle_delta), cosine = std::cos(laser.angle_delta);
        const float relative_x = laser.origin_x - laser.motion_center_x;
        const float relative_y = laser.origin_y - laser.motion_center_y;
        laser.origin_x = laser.motion_center_x + cosine * relative_x - sine * relative_y;
        laser.origin_y = laser.motion_center_y + sine * relative_x + cosine * relative_y;
        laser.angle += laser.angle_delta;
        laser.rotation_sine = std::sin(-laser.angle);
        laser.rotation_cosine = std::cos(-laser.angle);
    }
    laser.end_offset += laser.speed;
    if (laser.end_offset - laser.start_offset > laser.start_length)
        laser.start_offset = laser.end_offset - laser.start_length;
    laser.start_offset = std::max(0.f, laser.start_offset);

    LaserPhase phase;
    phase.center_x =
        (laser.end_offset - laser.start_offset) / 2 + laser.start_offset + laser.origin_x;
    phase.center_y = laser.origin_y;
    phase.full_width = laser.start_offset <= 0 ? laser.end_offset - laser.start_offset
                                               : (laser.end_offset - laser.start_offset) * .7f;
    phase.full_height = laser.width / 2;

    switch (laser.state) {
    case 0: { // LASER_STATE_STARTING
        if (!(laser.flags & 1)) {
            const int ramp = std::min(laser.start_time, 30);
            const float current_width = laser.start_time - ramp < laser.timer
                                            ? float(laser.timer) * laser.width / laser.start_time
                                            : 1.2f;
            phase.full_width = current_width / 2;
        }
        phase.lethal = laser.timer >= laser.hitbox_start_time;
        if (laser.timer < laser.start_time)
            break;
        laser.timer = 0;
        laser.state = 1;
        [[fallthrough]];
    }
    case 1: // LASER_STATE_ACTIVE
        phase.lethal = true;
        if (laser.timer < laser.duration)
            break;
        laser.timer = 0;
        laser.state = 2;
        if (!laser.despawn_duration) {
            laser.present = false;
            return phase;
        }
        [[fallthrough]];
    case 2: // LASER_STATE_DESPAWNING
    {
        const bool active_collision_already_ran = phase.lethal;
        if (!(laser.flags & 1) && laser.despawn_duration > 0) {
            const float current_width =
                laser.width - float(laser.timer) * laser.width / laser.despawn_duration;
            const float despawn_width = current_width / 2;
            if (!active_collision_already_ran ||
                std::abs(despawn_width) > std::abs(phase.full_width))
                phase.full_width = despawn_width;
        }
        phase.lethal = active_collision_already_ran || laser.timer < laser.hitbox_end_delay;
        if (laser.timer >= laser.despawn_duration) {
            laser.present = false;
            return phase;
        }
        break;
    }
    default:
        laser.present = false;
        phase.present = false;
        return phase;
    }
    if (laser.start_offset >= 640) {
        laser.present = false;
        return phase;
    }
    ++laser.timer;
    return phase;
}
inline float laser_clearance(float x, float y, float half_x, float half_y,
                             const LaserForecast &laser, const LaserPhase &phase) {
    const float relative_x = x - laser.origin_x, relative_y = y - laser.origin_y;
    const float local_x =
        laser.rotation_cosine * relative_x - laser.rotation_sine * relative_y + laser.origin_x;
    const float local_y =
        laser.rotation_sine * relative_x + laser.rotation_cosine * relative_y + laser.origin_y;
    return box_clearance(local_x, local_y, half_x, half_y, phase.center_x, phase.center_y,
                         phase.full_width, phase.full_height);
}
// A focused player following one constant candidate stays inside the world-space
// rectangle between its first and last predicted positions. Projecting that
// rectangle onto the laser's perpendicular axis gives a conservative broad phase:
// a rejected path cannot overlap any phase forecast at the current angle.
inline bool laser_path_may_overlap(float first_x, float first_y, float last_x, float last_y,
                                   float player_half_y, const LaserForecast &laser) {
    const float min_x = std::min(first_x, last_x), max_x = std::max(first_x, last_x);
    const float min_y = std::min(first_y, last_y), max_y = std::max(first_y, last_y);
    const float constant = laser.origin_y - laser.rotation_sine * laser.origin_x -
                           laser.rotation_cosine * laser.origin_y;
    const float projected_min =
        constant + laser.rotation_sine * (laser.rotation_sine >= 0 ? min_x : max_x) +
        laser.rotation_cosine * (laser.rotation_cosine >= 0 ? min_y : max_y);
    const float projected_max =
        constant + laser.rotation_sine * (laser.rotation_sine >= 0 ? max_x : min_x) +
        laser.rotation_cosine * (laser.rotation_cosine >= 0 ? max_y : min_y);
    const float collision_half_height = std::abs(laser.width) / 4 + player_half_y;
    if (!std::isfinite(projected_min) || !std::isfinite(projected_max) ||
        !std::isfinite(collision_half_height))
        return true;
    return projected_max >= laser.origin_y - collision_half_height &&
           projected_min <= laser.origin_y + collision_half_height;
}
struct CandidateScore {
    unsigned first_overlap;
    float minimum_clearance;
    double danger, center_distance;
    std::uint16_t action;
};
inline bool better(const CandidateScore &left, const CandidateScore &right) {
    if (left.first_overlap != right.first_overlap)
        return left.first_overlap > right.first_overlap;
    if (left.minimum_clearance != right.minimum_clearance)
        return left.minimum_clearance > right.minimum_clearance;
    if (left.danger != right.danger)
        return left.danger < right.danger;
    if (left.center_distance != right.center_distance)
        return left.center_distance < right.center_distance;
    return left.action < right.action;
}
} // namespace detail

// Candidate-independent projections rank proposals only. The native update remains
// the acceptance oracle for spawns, transforms, aiming, damage, RNG and lifecycle.
template <class Bullets, class Lasers>
std::uint16_t hazard_reactive(float player_x, float player_y, float half_x, float half_y,
                              float axis, float diagonal, std::uint16_t latched_input,
                              const Bullets &bullets, const Lasers &lasers,
                              HazardReactiveStats &stats, HazardReactiveOptions options = {},
                              HazardReactiveDecision *decision = nullptr) {
    ++stats.decisions;
    if (decision)
        *decision = {};
    const unsigned horizon = std::max(options.bullet_horizon, options.laser_horizon);
    detail::CandidateScore best{0, -std::numeric_limits<float>::infinity(),
                                std::numeric_limits<double>::infinity(),
                                std::numeric_limits<double>::infinity(), 4};
    const auto pending = detail::direction(latched_input);
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            const unsigned candidate_index = unsigned((dy + 1) * 3 + dx + 1);
            const detail::Direction candidate{dx, dy};
            const auto action = detail::input(candidate);
            if (decision)
                decision->candidates[candidate_index].action = action;
            if (!(options.candidate_mask & (1u << candidate_index)))
                continue;
            ++stats.candidates;
            float x = player_x, y = player_y;
            detail::advance(x, y, pending, axis, diagonal);
            detail::CandidateScore score{horizon + 1, std::numeric_limits<float>::infinity(), 0, 0,
                                         action};

            for (unsigned step = 2; step <= options.bullet_horizon; ++step) {
                detail::advance(x, y, candidate, axis, diagonal);
                for (const auto &bullet : bullets) {
                    if (bullet.state == 5 || bullet.state == 6)
                        continue;
                    ++stats.bullet_checks;
                    const auto projected =
                        detail::project_bullet(bullet, step, options.vector_acceleration);
                    if (projected.kind == detail::BulletProjectionKind::vector_acceleration)
                        ++stats.vector_acceleration_checks;
                    else if (projected.kind == detail::BulletProjectionKind::unsupported)
                        ++stats.unsupported_transform_checks;
                    const float clearance =
                        detail::box_clearance(x, y, half_x, half_y, projected.x, projected.y,
                                              bullet.full_width, bullet.full_height);
                    score.minimum_clearance = std::min(score.minimum_clearance, clearance);
                    const double positive = std::max(0.f, clearance);
                    score.danger += 1 / ((positive + 1) * (positive + 1) * step);
                    if (clearance <= 0 && bullet.state == 1 &&
                        projected.kind != detail::BulletProjectionKind::unsupported) {
                        score.first_overlap = std::min(score.first_overlap, step);
                        ++stats.predicted_overlaps;
                    }
                }
            }

            x = player_x;
            y = player_y;
            detail::advance(x, y, pending, axis, diagonal);
            float laser_first_x = x, laser_first_y = y;
            if (options.laser_horizon >= 2)
                detail::advance(laser_first_x, laser_first_y, candidate, axis, diagonal);
            const float candidate_speed = candidate.x && candidate.y ? diagonal : axis;
            const float laser_steps = options.laser_horizon > 1 ? options.laser_horizon - 1 : 0;
            const float laser_last_x =
                std::clamp(x + candidate.x * candidate_speed * laser_steps, 8.f, 376.f);
            const float laser_last_y =
                std::clamp(y + candidate.y * candidate_speed * laser_steps, 16.f, 432.f);
            for (const auto &view : lasers) {
                auto laser = detail::forecast(view, options.rigid_laser_motion);
                ++stats.laser_paths;
                if (laser.rigid_motion)
                    ++stats.rigid_laser_paths;
                if (options.laser_horizon < 2 ||
                    (!laser.rigid_motion &&
                     !detail::laser_path_may_overlap(laser_first_x, laser_first_y, laser_last_x,
                                                     laser_last_y, half_y, laser))) {
                    ++stats.laser_paths_pruned;
                    continue;
                }
                float lx = x, ly = y;
                for (unsigned step = 1; step <= options.laser_horizon && laser.present; ++step) {
                    const auto phase = detail::advance(laser);
                    if (step >= 2)
                        detail::advance(lx, ly, candidate, axis, diagonal);
                    if (step < 2 || !phase.present || !phase.lethal)
                        continue;
                    ++stats.laser_checks;
                    const float clearance =
                        detail::laser_clearance(lx, ly, half_x, half_y, laser, phase);
                    score.minimum_clearance = std::min(score.minimum_clearance, clearance);
                    const double positive = std::max(0.f, clearance);
                    score.danger += 1 / ((positive + 1) * (positive + 1) * step);
                    if (clearance <= 0) {
                        score.first_overlap = std::min(score.first_overlap, step);
                        ++stats.predicted_overlaps;
                    }
                }
            }
            if (!std::isfinite(score.minimum_clearance))
                score.minimum_clearance = 1e6f;
            float next_x = player_x, next_y = player_y;
            detail::advance(next_x, next_y, pending, axis, diagonal);
            detail::advance(next_x, next_y, candidate, axis, diagonal);
            const double center_x = next_x - 192, center_y = next_y - 380;
            score.center_distance = center_x * center_x + center_y * center_y;
            if (decision)
                decision->candidates[candidate_index] = {true,
                                                         score.action,
                                                         score.first_overlap,
                                                         score.minimum_clearance,
                                                         score.danger,
                                                         score.center_distance};
            if (detail::better(score, best))
                best = score;
        }
    if (decision)
        decision->selected_action = best.action;
    return best.action;
}
} // namespace th08::policy
