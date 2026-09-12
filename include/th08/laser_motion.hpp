#pragma once
#include "geometry.hpp"
#include "timing.hpp"

namespace th08::laser {
enum class Phase { starting, active, despawning };
struct State {
    geometry::Vec2 position;
    float angle = 0, start_offset = 0, end_offset = 0, length = 0, width = 0, speed = 0;
    std::int32_t timer = 0, start_time = 0, duration = 0, despawn_duration = 0;
    float subframe = 0;
    std::int32_t hitbox_start_time = 0, hitbox_end_delay = 0;
    Phase phase = Phase::active;
    bool in_use = true, fade_alpha = false;
};
struct CollisionCall {
    geometry::Laser geometry;
    bool allow_graze;
};
enum class Status { advanced, inactive, invalid };
struct Result {
    Status status = Status::invalid;
    std::array<CollisionCall, 3> calls{};
    unsigned count = 0;
};
// Collision/lifetime projection of BulletManager::OnUpdate's laser loop.
// Ordered calls matter: a transition frame can check starting, active and
// despawning hitboxes. Calls remain observable even if that frame retires the laser.
// Rendering, ANM execution, cancellation and external laser mutations stay upstream.
// Failure leaves state unchanged. Freeze must suppress this call externally.
inline Result advance(State &state, float multiplier = 1) {
    Result result;
    if (!state.in_use) {
        result.status = Status::inactive;
        return result;
    }
    const auto &s = state;
    if (!geometry::finite(s.position) || !std::isfinite(s.angle) || std::abs(s.angle) > 16 ||
        !std::isfinite(s.start_offset) || !std::isfinite(s.end_offset) ||
        !std::isfinite(s.length) || !std::isfinite(s.width) || !std::isfinite(s.speed) ||
        !std::isfinite(multiplier) || multiplier <= 0 || s.length < 0 || s.width < 0 ||
        s.timer < 0 || s.timer >= 1000000 || !std::isfinite(s.subframe) || s.subframe < 0 ||
        s.subframe >= 1 || s.start_time < 0 || s.start_time > 1000000 || s.duration < 0 ||
        s.duration > 1000000 || s.despawn_duration < 0 || s.despawn_duration > 1000000 ||
        s.hitbox_start_time < 0 || s.hitbox_end_delay < 0 || unsigned(s.phase) > 2 ||
        (s.phase == Phase::starting && s.start_time == 0) ||
        (s.fade_alpha && s.start_time == 0 && s.despawn_duration > 0))
        return result;
    auto next = state;
    next.end_offset += multiplier * next.speed;
    if (next.end_offset - next.start_offset > next.length)
        next.start_offset = next.end_offset - next.length;
    if (next.start_offset < 0)
        next.start_offset = 0;
    const float distance = next.end_offset - next.start_offset;
    if (!std::isfinite(distance) || distance < 0)
        return result;
    geometry::Laser box{{{distance / 2.0f + next.start_offset + next.position.x, next.position.y},
                         {next.start_offset <= 0 ? distance : distance * 0.7f, next.width / 2.0f}},
                        next.position,
                        next.angle};
    try {
        geometry::validate(geometry::Hazard::laser(box));
    } catch (const std::invalid_argument &) {
        return result;
    }
    auto emit = [&](bool graze) { result.calls[result.count++] = {box, graze}; };
    bool tail = true;
    if (next.phase == Phase::starting) {
        if (!next.fade_alpha) {
            const auto ramp = std::min(next.start_time, 30);
            const float current_width =
                next.start_time - ramp < next.timer
                    ? (float(next.timer) + next.subframe) * next.width / float(next.start_time)
                    : 1.2f;
            // This is x, not y, in the pinned source. Do not "fix" the axis.
            box.box.size.x = current_width / 2.0f;
        }
        if (next.timer >= next.hitbox_start_time)
            emit(false);
        if (next.timer >= next.start_time) {
            next.timer = 0;
            next.subframe = 0;
            next.phase = Phase::active;
        }
    }
    if (next.phase == Phase::active) {
        emit(next.timer % 20 == 0);
        if (next.timer >= next.duration) {
            next.timer = 0;
            next.subframe = 0;
            next.phase = Phase::despawning;
            if (next.despawn_duration == 0) {
                next.in_use = false;
                tail = false;
            }
        }
    }
    if (next.phase == Phase::despawning && tail) {
        if (!next.fade_alpha && next.despawn_duration > 0) {
            const float current_width = next.width - (float(next.timer) + next.subframe) *
                                                         next.width / float(next.despawn_duration);
            box.box.size.x = current_width / 2.0f;
        }
        if (next.timer < next.hitbox_end_delay)
            emit(false);
        if (next.timer >= next.despawn_duration) {
            next.in_use = false;
            tail = false;
        }
    }
    // Preserve signed sizes from source ramp arithmetic, including terminal
    // roundoff. Clamping to zero changes inclusive-contact behavior.
    for (unsigned i = 0; i < result.count; ++i)
        if (!std::isfinite(result.calls[i].geometry.box.size.x)) {
            result.count = 0;
            return result;
        }
    if (tail) {
        if (next.start_offset >= 640)
            next.in_use = false;
        timing::tick(next.timer, next.subframe, multiplier);
    }
    state = next;
    result.status = next.in_use ? Status::advanced : Status::inactive;
    return result;
}
} // namespace th08::laser
