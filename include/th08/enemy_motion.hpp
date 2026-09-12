#pragma once
#include "kinematics.hpp"
#include <cmath>
#include <cstdint>
#include <limits>

namespace th08::enemy {
struct Vec3 {
    float x = 0, y = 0, z = 0;
};
struct Clock {
    std::int32_t previous = -999;
    float fraction = 0;
    std::int32_t current = 0;
    void set(std::int32_t value) {
        *this = {-999, 0, value};
    }
};
enum class Mode : std::uint8_t { none, polar, interpolated, orbit };
// The source's unnamed value 7 follows the same default branch as linear.
enum class Easing : std::uint8_t {
    linear,
    in_quadratic,
    in_cubic,
    in_quartic,
    out_quadratic,
    out_cubic,
    out_quartic,
    unnamed_linear
};
enum class Status { advanced, invalid_state, requires_parent };
struct Bounds {
    Vec3 lower, upper;
};
// A projection, not an enemy lifetime. The owning world resolves pause, death,
// alignment and callbacks before calling these phases. Copying owns all state.
struct State {
    Vec3 position, position_offset, world_position, velocity;
    Vec3 previous_position, last_displacement;
    Vec3 interpolation_origin, interpolation_delta;
    float angle = 0, angular_velocity = 0, speed = 0, acceleration = 0;
    float orbit_angle = 0, orbit_angular_velocity = 0, orbit_radius = 0, radial_velocity = 0;
    Clock timer;
    std::int32_t duration = 0;
    Mode mode = Mode::none;
    Easing easing = Easing::linear;
    bool mirror_x = false, clamp = false, skip_integration = false;
    bool inherit_parent_position = false;
    Bounds bounds;
};
namespace detail {
inline bool finite(Vec3 v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
inline Vec3 add(Vec3 a, Vec3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline Vec3 subtract(Vec3 a, Vec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Vec3 scale(Vec3 v, float factor) {
    return {v.x * factor, v.y * factor, v.z * factor};
}
inline bool valid(const State &s) {
    return finite(s.position) && finite(s.position_offset) && finite(s.world_position) &&
           finite(s.velocity) && finite(s.previous_position) && finite(s.last_displacement) &&
           finite(s.interpolation_origin) && finite(s.interpolation_delta) &&
           std::isfinite(s.angle) && std::isfinite(s.angular_velocity) && std::isfinite(s.speed) &&
           std::isfinite(s.acceleration) && std::isfinite(s.orbit_angle) &&
           std::isfinite(s.orbit_angular_velocity) && std::isfinite(s.orbit_radius) &&
           std::isfinite(s.radial_velocity) && std::isfinite(s.timer.fraction) &&
           s.timer.fraction >= 0 && s.timer.fraction < 1 && static_cast<unsigned>(s.mode) <= 3 &&
           static_cast<unsigned>(s.easing) <= 7 &&
           (!s.clamp || (finite(s.bounds.lower) && finite(s.bounds.upper)));
}
inline void clamp(State &s) {
    if (!s.clamp)
        return;
    // Preserve ordered if/else even for source-provided inverted bounds.
    if (s.position.x < s.bounds.lower.x)
        s.position.x = s.bounds.lower.x;
    else if (s.position.x > s.bounds.upper.x)
        s.position.x = s.bounds.upper.x;
    if (s.position.y < s.bounds.lower.y)
        s.position.y = s.bounds.lower.y;
    else if (s.position.y > s.bounds.upper.y)
        s.position.y = s.bounds.upper.y;
}
inline bool decrement(Clock &clock, float multiplier, bool force_extra) {
    if (clock.current < std::numeric_limits<std::int32_t>::min() + 2)
        return false;
    if (force_extra) {
        --clock.current;
        clock.fraction = 0;
        clock.previous = -999;
    }
    if (multiplier > .99f) {
        --clock.current;
    } else {
        clock.previous = clock.current;
        clock.fraction -= multiplier;
        if (clock.fraction < 0) {
            --clock.current;
            clock.fraction += 1;
        }
    }
    return true;
}
inline float ease(float progress, Easing easing) {
    switch (easing) {
    case Easing::in_quadratic:
        return progress * progress;
    case Easing::in_cubic:
        return progress * progress * progress;
    case Easing::in_quartic:
        return progress * progress * progress * progress;
    case Easing::out_quadratic:
        progress = 1.0f - progress;
        progress *= progress;
        return 1.0f - progress;
    case Easing::out_cubic:
        progress = 1.0f - progress;
        progress = progress * progress * progress;
        return 1.0f - progress;
    case Easing::out_quartic:
        progress = 1.0f - progress;
        progress = progress * progress * progress * progress;
        return 1.0f - progress;
    default:
        return progress;
    }
}
} // namespace detail

// ECL redispatch publication retains z; the later manager phase explicitly zeros it.
inline Status refresh_world(State &state) {
    if (!detail::valid(state))
        return Status::invalid_state;
    const Vec3 world = detail::add(state.position, state.position_offset);
    if (!detail::finite(world))
        return Status::invalid_state;
    state.world_position = world;
    return Status::advanced;
}
inline Status set_position(State &state, float x, float y) {
    if (!detail::valid(state) || !std::isfinite(x) || !std::isfinite(y))
        return Status::invalid_state;
    state.position = {x, y, 0};
    detail::clamp(state);
    return Status::advanced;
}
// Resolved scalar helpers: operand evaluation and shared RNG belong to the world.
// Zero duration is rejected: UpdateMovement would divide by zero. Negative source
// durations remain representable and terminate on their first movement update.
inline Status configure_relative(State &state, std::int32_t duration, std::int32_t easing,
                                 float target_x, float target_y) {
    if (!detail::valid(state) || duration == 0 || !std::isfinite(target_x) ||
        !std::isfinite(target_y))
        return Status::invalid_state;
    State next = state;
    next.interpolation_delta = detail::subtract({target_x, target_y, 0}, next.world_position);
    next.interpolation_origin = next.position;
    next.timer.set(next.duration = duration);
    next.easing = static_cast<Easing>(static_cast<std::uint32_t>(easing) & 7U);
    next.mode = Mode::interpolated;
    next.velocity = {};
    if (next.mirror_x)
        next.interpolation_delta.x = -next.interpolation_delta.x;
    if (!detail::valid(next))
        return Status::invalid_state;
    state = next;
    return Status::advanced;
}
inline Status configure_polar(State &state, std::int32_t duration, std::int32_t easing, float angle,
                              float speed) {
    if (!detail::valid(state) || duration <= 0 || !std::isfinite(angle) || !std::isfinite(speed))
        return Status::invalid_state;
    State next = state;
    angle = kinematics::normalize_angle(angle);
    next.interpolation_delta = {std::cos(angle) * speed * duration,
                                std::sin(angle) * speed * duration, 0};
    next.interpolation_origin = next.world_position;
    next.timer.set(next.duration = duration);
    next.easing = static_cast<Easing>(static_cast<std::uint32_t>(easing) & 7U);
    next.mode = Mode::interpolated;
    if (next.mirror_x)
        next.interpolation_delta.x = -next.interpolation_delta.x;
    if (!detail::valid(next))
        return Status::invalid_state;
    state = next;
    return Status::advanced;
}

// Run after all ECL contexts, before UpdateShotAndAnm. Do NOT combine this with
// displacement: shot position/ANM decisions observe that intervening phase.
// All failures are atomic. No allocation, global state, RNG or player defaults.
inline Status update_velocity(State &state, float multiplier, bool force_extra_timer_step) {
    if (!detail::valid(state) || !std::isfinite(multiplier) || multiplier <= 0)
        return Status::invalid_state;
    State next = state;
    switch (next.mode) {
    case Mode::none:
        break;
    case Mode::orbit: {
        next.orbit_angle =
            kinematics::normalize_angle(next.orbit_angle, multiplier * next.orbit_angular_velocity);
        next.orbit_radius = multiplier * next.radial_velocity + next.orbit_radius;
        next.velocity.x = std::cos(next.orbit_angle) * next.orbit_radius +
                          next.interpolation_origin.x - next.position.x;
        next.velocity.y = std::sin(next.orbit_angle) * next.orbit_radius +
                          next.interpolation_origin.y - next.position.y;
        // Orbit leaves velocity.z unchanged, unlike polar movement.
        next.angle = kinematics::point_angle(next.velocity.y, next.velocity.x);
        if (next.duration > 0) {
            if (!detail::decrement(next.timer, multiplier, force_extra_timer_step))
                return Status::invalid_state;
            if (next.timer.current <= 0)
                next.mode = Mode::none;
        }
        break;
    }
    case Mode::polar:
        next.angle = kinematics::normalize_angle(next.angle, multiplier * next.angular_velocity);
        next.speed = multiplier * next.acceleration + next.speed;
        next.velocity = {std::cos(next.angle) * next.speed, std::sin(next.angle) * next.speed, 0};
        if (next.duration > 0) {
            if (!detail::decrement(next.timer, multiplier, force_extra_timer_step))
                return Status::invalid_state;
            if (next.timer.current <= 0)
                next.mode = Mode::none;
        }
        break;
    case Mode::interpolated: {
        if (next.duration == 0 ||
            !detail::decrement(next.timer, multiplier, force_extra_timer_step))
            return Status::invalid_state;
        float progress = 1.0f - (float(next.timer.current) + next.timer.fraction) / next.duration;
        if (progress < 0)
            progress = 0;
        progress = detail::ease(progress, next.easing);
        next.velocity =
            detail::subtract(detail::add(next.interpolation_origin,
                                         detail::scale(next.interpolation_delta, progress)),
                             next.position);
        if (next.mirror_x)
            next.velocity.x = -next.velocity.x;
        next.angle = kinematics::point_angle(next.velocity.y, next.velocity.x);
        if (next.timer.current <= 0) {
            next.mode = Mode::none;
            next.position = detail::add(next.interpolation_origin, next.interpolation_delta);
            next.velocity = {};
        }
        break;
    }
    }
    if (!detail::valid(next))
        return Status::invalid_state;
    state = next;
    return Status::advanced;
}

// Run only after the intervening shot/ANM phase. parent_position is the parent's
// LOCAL position, not its world position. Null means no parent exists, matching
// the source's nullable parent pointer; an unresolved existing parent must stop
// the owning world before this call (or pass parent_resolved=false).
inline Status integrate_position(State &state, float multiplier,
                                 const Vec3 *parent_position = nullptr,
                                 bool parent_resolved = true) {
    if (!detail::valid(state) || !std::isfinite(multiplier) || multiplier <= 0)
        return Status::invalid_state;
    if (!state.skip_integration && state.inherit_parent_position && !parent_resolved)
        return Status::requires_parent;
    if (parent_position && !detail::finite(*parent_position))
        return Status::invalid_state;
    State next = state;
    if (!next.skip_integration) {
        detail::clamp(next);
        next.last_displacement = detail::subtract(next.position, next.previous_position);
        next.previous_position = next.position;
        if (next.mirror_x)
            next.position.x -= multiplier * next.velocity.x;
        else
            next.position.x += multiplier * next.velocity.x;
        next.position.y += multiplier * next.velocity.y;
        next.position.z += multiplier * next.velocity.z;
        detail::clamp(next);
        if (next.inherit_parent_position && parent_position)
            next.position_offset = *parent_position;
    }
    next.world_position = detail::add(next.position, next.position_offset);
    next.world_position.z = 0;
    if (!detail::valid(next))
        return Status::invalid_state;
    state = next;
    return Status::advanced;
}
} // namespace th08::enemy
