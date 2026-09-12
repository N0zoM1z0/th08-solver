#pragma once
#include "kinematics.hpp"
#include "timing.hpp"
#include <algorithm>
#include <array>
#include <limits>

namespace th08::bullet {
enum class Status { advanced, inactive, invalid, requires_target, unsupported };
struct Flight {
    float x, y, velocity_x, velocity_y, angle, speed;
};
enum class TurnMode { relative, absolute, aimed };
struct DirectionChange {
    TurnMode mode = TurnMode::relative;
    bool active = false;
    std::int32_t interval = 0, repeats = 0, completed = 0, timer = 0;
    float angle = 0, speed = 0;
    float subframe = 0;
};
// One active direction-change transform. A target is required only on its firing
// frame; callers must resolve it from the candidate-dependent current player state.
// Failure leaves both arguments unchanged. This kernel has no allocation or RNG.
inline Status advance_direction(Flight &flight, DirectionChange &turn, float multiplier,
                                float angle_to_player = std::numeric_limits<float>::quiet_NaN()) {
    if (!turn.active)
        return Status::advanced;
    if (!std::isfinite(multiplier) || multiplier <= 0 || turn.timer < 0 ||
        turn.timer == std::numeric_limits<std::int32_t>::max() || turn.completed < 0 ||
        turn.completed == std::numeric_limits<std::int32_t>::max() ||
        !std::isfinite(flight.speed) || !std::isfinite(flight.angle) ||
        !std::isfinite(turn.speed) || !std::isfinite(turn.angle) || unsigned(turn.mode) > 2 ||
        !std::isfinite(turn.subframe) || turn.subframe < 0 || turn.subframe >= 1)
        return Status::invalid;
    float angle = flight.angle, speed = flight.speed, magnitude;
    const bool fire = turn.timer >= turn.interval;
    if (fire) {
        if (turn.mode == TurnMode::aimed && !std::isfinite(angle_to_player))
            return Status::requires_target;
        angle = turn.mode == TurnMode::relative ? angle + turn.angle
                : turn.mode == TurnMode::absolute
                    ? turn.angle
                    : kinematics::normalize_angle(angle_to_player, turn.angle);
        speed = turn.speed;
        magnitude = speed;
    } else {
        magnitude = speed - ((float(turn.timer) + turn.subframe) * speed) / float(turn.interval);
    }
    const float scaled = magnitude * multiplier;
    const float vx = std::cos(angle) * scaled, vy = std::sin(angle) * scaled;
    if (!std::isfinite(vx) || !std::isfinite(vy) || !std::isfinite(angle))
        return Status::invalid;
    flight.angle = angle;
    flight.speed = speed;
    flight.velocity_x = vx;
    flight.velocity_y = vy;
    if (fire) {
        ++turn.completed;
        if (turn.completed >= turn.repeats)
            turn.active = false;
        turn.timer = 0;
        turn.subframe = 0;
    }
    timing::tick(turn.timer, turn.subframe, multiplier);
    return Status::advanced;
}

enum class Phase { unused, spawning_fast, spawning_normal, spawning_slow, fired };
struct Particle {
    Flight flight{};
    DirectionChange turn{};
    Phase phase = Phase::unused;
    std::uint32_t spawn_calls_remaining = 0;
    std::int32_t cull_delay = 0, offscreen_frames = 0;
    float sprite_width = 0, sprite_height = 0;
};
struct InitialState {
    kinematics::Launch launch;
    float x, y, sprite_width, sprite_height;
    Phase phase;
    std::uint32_t spawn_calls;
    std::int32_t cull_delay;
    DirectionChange turn;
};
// The caller supplies a certified animation lifetime and already-installed turn
// state. This does not approximate missing transform programs or sprite scripts.
inline Status initialize(const InitialState &input, Particle &output) {
    if (!std::isfinite(input.x) || !std::isfinite(input.y) ||
        !std::isfinite(input.launch.velocity_x) || !std::isfinite(input.launch.velocity_y) ||
        !std::isfinite(input.launch.angle) || !std::isfinite(input.launch.speed) ||
        !std::isfinite(input.sprite_width) || !std::isfinite(input.sprite_height) ||
        input.sprite_width < 0 || input.sprite_height < 0 || input.cull_delay < 0 ||
        unsigned(input.phase) < 1 || unsigned(input.phase) > 4 ||
        (input.phase != Phase::fired && (input.spawn_calls == 0 || input.spawn_calls > 32767)))
        return Status::invalid;
    Particle result;
    result.flight = {input.x,
                     input.y,
                     input.launch.velocity_x,
                     input.launch.velocity_y,
                     input.launch.angle,
                     input.launch.speed};
    result.phase = input.phase;
    result.spawn_calls_remaining = input.spawn_calls;
    result.cull_delay = input.cull_delay;
    result.sprite_width = input.sprite_width;
    result.sprite_height = input.sprite_height;
    result.turn = input.turn;
    if (result.phase != Phase::fired) {
        result.flight.x -= result.flight.velocity_x * 4.0f;
        result.flight.y -= result.flight.velocity_y * 4.0f;
    }
    if (!std::isfinite(result.flight.x) || !std::isfinite(result.flight.y))
        return Status::invalid;
    output = result;
    return Status::advanced;
}
inline bool within_playfield(const Particle &particle) {
    const auto &p = particle.flight;
    return !(particle.sprite_width / 2.0f + p.x < 0 || p.x - particle.sprite_width / 2.0f > 384 ||
             particle.sprite_height / 2.0f + p.y < 0 || p.y - particle.sprite_height / 2.0f > 448);
}
// Pre-collision lifecycle for a certified, non-cancelled particle with at most one
// direction transform and no later transform records. Unit-rate animation clock.
// Deathbomb freeze must suppress this entire call externally. scripted_freeze only
// suppresses the fired displacement; spawning displacement still occurs in source.
// No player collision, graze, cancellation, ANM rendering, or pool allocation here.
inline Status advance(Particle &particle, float multiplier = 1, bool scripted_freeze = false,
                      float angle_to_player = std::numeric_limits<float>::quiet_NaN()) {
    if (particle.phase == Phase::unused)
        return Status::inactive;
    if (!std::isfinite(multiplier) || multiplier <= 0 || unsigned(particle.phase) > 4 ||
        particle.cull_delay < 0 || particle.offscreen_frames < 0 ||
        particle.offscreen_frames > 128 || !std::isfinite(particle.sprite_width) ||
        !std::isfinite(particle.sprite_height) || particle.sprite_width < 0 ||
        particle.sprite_height < 0 || !std::isfinite(particle.flight.x) ||
        !std::isfinite(particle.flight.y) || !std::isfinite(particle.flight.velocity_x) ||
        !std::isfinite(particle.flight.velocity_y))
        return Status::invalid;
    if (particle.phase != Phase::fired && multiplier != 1)
        return Status::unsupported; // Timing certificate uses a unit-rate ZunTimer.
    auto next = particle;
    if (next.phase != Phase::fired) {
        if (next.spawn_calls_remaining == 0)
            return Status::invalid;
        const float divisor = next.phase == Phase::spawning_fast     ? 2.0f
                              : next.phase == Phase::spawning_normal ? 2.5f
                                                                     : 3.0f;
        // Float3::operator/ multiplies by one rounded reciprocal.
        const float inverse = 1.0f / divisor;
        next.flight.x += next.flight.velocity_x * inverse;
        next.flight.y += next.flight.velocity_y * inverse;
        if (--next.spawn_calls_remaining != 0) {
            if (!std::isfinite(next.flight.x) || !std::isfinite(next.flight.y))
                return Status::invalid;
            particle = next;
            return Status::advanced;
        }
        next.phase = Phase::fired;
    }
    const auto status = advance_direction(next.flight, next.turn, multiplier, angle_to_player);
    if (status != Status::advanced)
        return status;
    if (next.cull_delay != 0)
        --next.cull_delay;
    if (!scripted_freeze) {
        next.flight.x += next.flight.velocity_x;
        next.flight.y += next.flight.velocity_y;
    }
    if (!std::isfinite(next.flight.x) || !std::isfinite(next.flight.y))
        return Status::invalid;
    if (next.cull_delay == 0) {
        if (within_playfield(next)) {
            next.offscreen_frames = 0;
        } else if (next.turn.active) {
            if (++next.offscreen_frames >= 128)
                next.phase = Phase::unused;
        } else if (next.offscreen_frames == 0) {
            next.phase = Phase::unused;
        } else {
            --next.offscreen_frames;
        }
    }
    particle = next;
    return next.phase == Phase::unused ? Status::inactive : Status::advanced;
}
} // namespace th08::bullet
