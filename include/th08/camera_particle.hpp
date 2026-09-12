#pragma once
#include "rng.hpp"
#include <cmath>
#include <cstdint>

namespace th08::effect::camera_particle {
struct Vec3 {
    float x, y, z;
};
struct Color {
    std::uint8_t r, g, b, a;
};
// These fields are shared with ANM/render consumers, not discarded visual writes.
struct Animation {
    Vec3 position_offset, position_initial, position_final, rotation_initial;
    Color primary, secondary;
    std::uint32_t flags;
};
// Supply the actual post-allocation, post-template-time-zero snapshot. This
// callback does not initialize a pool slot or fabricate inherited ANM fields.
struct State {
    Vec3 position, inherited_velocity, velocity, acceleration, particle_position;
    Animation animation;
    std::int8_t draw_group;
};
struct Camera {
    Vec3 position, look_at_offset, forward;
};
struct Bosses {
    std::uint8_t occupied_slots; // HasBoss examines all eight slots.
    bool primary_damageable;
    Vec3 primary_position; // Slot zero LOCAL position, not world position.
};
enum class Status { initialized, alive, culled, missing_context, invalid_state };
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
inline Vec3 scale(Vec3 a, float value) {
    return {a.x * value, a.y * value, a.z * value};
}
inline float dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Color tint(Color primary, Color stage) {
    auto channel = [](std::uint8_t a, std::uint8_t b) {
        return std::uint8_t((std::uint32_t(a) * b) >> 8);
    };
    return {channel(primary.r, stage.r), channel(primary.g, stage.g), channel(primary.b, stage.b),
            channel(primary.a, stage.a)};
}
} // namespace detail

// Effect table index 51 selects ANM script 73. This is its initializer callback,
// AFTER SetAndExecuteScriptIdx and the caller's color/velocity setup. Eight float
// RNG calls consume sixteen U16 draws. The frame multiplier is installed here,
// not applied again by update. Missing/invalid inputs leave State and RNG intact.
inline Status initialize(State &state, const Camera *camera, float multiplier, random::Rng *rng) {
    if (!camera || !rng)
        return Status::missing_context;
    if (!detail::finite(camera->position) || !detail::finite(camera->look_at_offset) ||
        !detail::finite(state.inherited_velocity) || !std::isfinite(multiplier))
        return Status::invalid_state;
    auto next = state;
    auto stream = *rng;
    const Vec3 offset{-camera->look_at_offset.x, -camera->look_at_offset.y,
                      -camera->look_at_offset.z};
    next.particle_position = detail::add(camera->look_at_offset, camera->position);
    next.particle_position.x += stream.range_signed_float(60.0f) + offset.x / 2.0f;
    next.particle_position.y += stream.range_signed_float(100.0f) - 50.0f + offset.y / 2.0f;
    next.particle_position.z += stream.range_float(100.0f) - 100.0f + offset.z / 2.0f;
    next.velocity.x = stream.range_signed_float(.001f) + next.inherited_velocity.x;
    next.velocity.y = stream.range_signed_float(.03f) + next.inherited_velocity.y;
    next.velocity.z = -stream.range_float(.1f) - .3f + next.inherited_velocity.z;
    next.acceleration.x = stream.range_signed_float(.0001f);
    next.acceleration.y = stream.range_signed_float(.0001f);
    next.acceleration.z = -.0003f;
    next.velocity = detail::scale(next.velocity, multiplier);
    next.acceleration = detail::scale(next.acceleration, multiplier);
    if (!detail::finite(next.particle_position) || !detail::finite(next.velocity) ||
        !detail::finite(next.acceleration))
        return Status::invalid_state;
    next.draw_group = 1;
    next.animation.position_offset.x = -9999.0f;
    next.animation.position_initial.x = 0;
    next.animation.position_final = {0, 0, 0};
    next.animation.rotation_initial = {0, 0, 0};
    state = next;
    *rng = stream;
    return Status::initialized;
}

// Callback phase only: no ANM advance, effect timer, pool retirement or freeze
// scheduling. Neither player position nor ANM visibility is read by this callback.
// Camera normalization follows the pinned modern D3DX profile (length > 1e-8).
// Arithmetic operands and intermediate results must remain finite.
// A cull is a successful callback result: motion before the test stays committed,
// while boss/tint/ANM-flag writes have not run. Unknown boss/tint inputs are needed
// only after that test. Other failures roll back the complete call.
inline Status update(State &state, const Camera *camera, const Bosses *bosses,
                     const Color *stage_tint) {
    if (!camera)
        return Status::missing_context;
    if (!detail::finite(camera->position) || !detail::finite(camera->forward) ||
        !detail::finite(state.velocity) || !detail::finite(state.acceleration) ||
        !detail::finite(state.particle_position))
        return Status::invalid_state;
    auto next = state;
    next.velocity = detail::add(next.velocity, next.acceleration);
    next.particle_position = detail::add(next.particle_position, next.velocity);
    next.position = next.particle_position;
    Vec3 delta = detail::subtract(next.position, camera->position);
    const float length = std::sqrt(detail::dot(delta, delta));
    if (!detail::finite(next.velocity) || !detail::finite(next.position) || !std::isfinite(length))
        return Status::invalid_state;
    if (length > 1.0e-8f)
        delta = {delta.x / length, delta.y / length, delta.z / length};
    else
        delta = {0, 0, 0};
    const float alignment = detail::dot(camera->forward, delta);
    if (!std::isfinite(alignment))
        return Status::invalid_state;
    if (alignment < .94f) {
        state = next;
        return Status::culled;
    }
    if (!bosses)
        return Status::missing_context;
    if (bosses->occupied_slots != 0) {
        // The source scans every slot, then dereferences slot zero. Reject that
        // undefined configuration instead of treating another boss as slot zero.
        if (!(bosses->occupied_slots & 1U))
            return Status::invalid_state;
        if (bosses->primary_damageable) {
            if (!detail::finite(bosses->primary_position) ||
                !std::isfinite(next.animation.position_offset.x))
                return Status::invalid_state;
            if (next.animation.position_offset.x <= -9999.0f)
                next.animation.position_offset = bosses->primary_position;
            else {
                if (!detail::finite(next.animation.position_offset))
                    return Status::invalid_state;
                next.animation.position_offset =
                    detail::add(detail::scale(detail::subtract(bosses->primary_position,
                                                               next.animation.position_offset),
                                              .1f),
                                next.animation.position_offset);
                if (!detail::finite(next.animation.position_offset))
                    return Status::invalid_state;
            }
        }
    }
    if (!stage_tint)
        return Status::missing_context;
    next.animation.flags |= 0x20000U;
    next.animation.secondary = detail::tint(next.animation.primary, *stage_tint);
    state = next;
    return Status::alive;
}
} // namespace th08::effect::camera_particle
