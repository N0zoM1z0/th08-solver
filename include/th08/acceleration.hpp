#pragma once
#include "bullet_motion.hpp"

namespace th08::bullet {
enum class AccelerationMode { deceleration, vector, polar };
struct Acceleration {
    AccelerationMode mode = AccelerationMode::deceleration;
    bool active = false;
    std::int32_t timer = 0, duration = 0;
    float subframe = 0;
    // Vector acceleration stores the already-installed delta, including the
    // installation frame's multiplier. Polar acceleration uses the scalar deltas.
    float vector_x = 0, vector_y = 0, speed_delta = 0, angle_delta = 0;
};

inline Status install_vector_acceleration(Acceleration &output, const Flight &flight,
                                          float magnitude, float angle, std::int32_t duration,
                                          float multiplier) {
    if (!std::isfinite(magnitude) || !std::isfinite(angle) || !std::isfinite(flight.angle) ||
        !std::isfinite(multiplier) || multiplier <= 0)
        return Status::invalid;
    const float resolved_angle = angle > -990.0f ? angle : flight.angle;
    const float scaled = multiplier * magnitude;
    Acceleration state;
    state.mode = AccelerationMode::vector;
    state.active = true;
    state.duration = duration;
    state.vector_x = std::cos(resolved_angle) * scaled;
    state.vector_y = std::sin(resolved_angle) * scaled;
    if (!std::isfinite(state.vector_x) || !std::isfinite(state.vector_y))
        return Status::invalid;
    output = state;
    return Status::advanced;
}

// One installed acceleration state, before displacement/collision. The world must
// invoke concurrent effects in source order: deceleration, vector, polar, turns.
// No transform-table scheduling, pool, sounds, cancellation or external state here.
// Active effects tick even on their expiry frame. Failure changes neither input.
inline Status advance_acceleration(Flight &flight, Acceleration &state, float multiplier = 1) {
    if (!state.active)
        return Status::advanced;
    if (unsigned(state.mode) > 2 || !std::isfinite(multiplier) || multiplier <= 0 ||
        state.timer < 0 || state.timer == std::numeric_limits<std::int32_t>::max() ||
        !std::isfinite(state.subframe) || state.subframe < 0 || state.subframe >= 1 ||
        !std::isfinite(flight.angle) || !std::isfinite(flight.speed) ||
        !std::isfinite(flight.velocity_x) || !std::isfinite(flight.velocity_y) ||
        !std::isfinite(state.vector_x) || !std::isfinite(state.vector_y) ||
        !std::isfinite(state.speed_delta) || !std::isfinite(state.angle_delta))
        return Status::invalid;
    auto next = flight;
    bool active = state.active;
    switch (state.mode) {
    case AccelerationMode::deceleration:
        if (state.timer <= 16) {
            const float magnitude = 5.0f - ((float(state.timer) + state.subframe) * 5.0f) / 16.0f;
            const float scaled = (magnitude + next.speed) * multiplier;
            next.velocity_x = std::cos(next.angle) * scaled;
            next.velocity_y = std::sin(next.angle) * scaled;
        } else {
            active = false;
        }
        break;
    case AccelerationMode::vector:
        if (state.timer >= state.duration) {
            active = false;
        } else {
            next.velocity_x += state.vector_x * multiplier;
            next.velocity_y += state.vector_y * multiplier;
            if (std::abs(next.velocity_x) > 0.0001f || std::abs(next.velocity_y) > 0.0001f)
                next.angle = kinematics::point_angle(next.velocity_y, next.velocity_x);
            // Source vector acceleration does not recompute the scalar speed.
        }
        break;
    case AccelerationMode::polar:
        if (state.timer >= state.duration) {
            active = false;
        } else {
            next.angle = kinematics::normalize_angle(next.angle, multiplier * state.angle_delta);
            next.speed += multiplier * state.speed_delta;
            const float scaled = multiplier * next.speed;
            next.velocity_x = std::cos(next.angle) * scaled;
            next.velocity_y = std::sin(next.angle) * scaled;
        }
        break;
    }
    if (!std::isfinite(next.angle) || !std::isfinite(next.speed) ||
        !std::isfinite(next.velocity_x) || !std::isfinite(next.velocity_y))
        return Status::invalid;
    flight = next;
    state.active = active;
    timing::tick(state.timer, state.subframe, multiplier);
    return Status::advanced;
}
} // namespace th08::bullet
