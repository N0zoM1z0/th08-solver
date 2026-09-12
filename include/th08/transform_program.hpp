#pragma once
#include "acceleration.hpp"

namespace th08::bullet::transform {
enum Kind : std::uint32_t {
    none = 0,
    decelerate = 1,
    vector = 0x10,
    polar = 0x20,
    relative = 0x40,
    aimed = 0x80,
    absolute = 0x100,
    bounce_all = 0x400,
    bounce_except_bottom = 0x800,
    cull_delay = 0x2000,
    wait = 0x20000,
    despawn = 0x40000,
    sound = 0x80000,
    wrap_x = 0x400000,
    wrap_y = 0x800000
};
inline constexpr std::uint32_t motion_flags = decelerate | vector | polar | relative | aimed |
                                              absolute | bounce_all | bounce_except_bottom | wait |
                                              wrap_x | wrap_y;
struct Record {
    float float0 = 0, float1 = 0;
    std::int32_t int0 = 0, int1 = 0;
    std::uint32_t kind = none;
    std::int32_t allow_while_active = 0;
};
struct Program {
    // Caller-owned immutable records; multiple particles may share this program.
    std::array<Record, 18> records{};
};
struct State {
    Flight flight{};
    std::array<Acceleration, 3> acceleration{
        {{AccelerationMode::deceleration}, {AccelerationMode::vector}, {AccelerationMode::polar}}};
    // All three direction flags deliberately share one state, as in the source.
    DirectionChange turn{};
    std::uint32_t enabled_flags = 0, active_flags = 0, pc = 0;
    std::int32_t offscreen_cull_delay = 0, transform_sound = -1, wait_timer = 0;
    float wait_subframe = 0;
    std::int32_t wrap_timer = 0, bounce_count = 0, bounce_limit = 0;
    float wrap_subframe = 0, bounce_speed = 0;
    // Bounce depends on resource-derived sprite dimensions, not collision size.
    float sprite_width = std::numeric_limits<float>::quiet_NaN();
    float sprite_height = std::numeric_limits<float>::quiet_NaN();
    bool despawning = false;
};
struct SoundEvent {
    std::int32_t id;
    bool positioned;
    float x;
};
struct Result {
    Status status = Status::advanced;
    std::uint32_t pc = 0;
    // Eighteen immediate records, three direction firings and one boundary bounce.
    std::array<SoundEvent, 22> sounds{};
    unsigned sound_count = 0;
};
namespace detail {
inline Status validate(const State &state, float multiplier) {
    if (state.pc > 18 || !std::isfinite(multiplier) || multiplier <= 0 ||
        !std::isfinite(state.flight.x) || !std::isfinite(state.flight.y) ||
        !std::isfinite(state.flight.angle) || !std::isfinite(state.flight.speed))
        return Status::invalid;
    if ((state.active_flags & ~motion_flags) != 0 || state.despawning)
        return Status::unsupported;
    return Status::advanced;
}
inline Status install(const Program &program, State &state, float multiplier, Result &result) {
    while (state.pc < program.records.size()) {
        result.pc = state.pc;
        const auto &record = program.records[state.pc];
        if (record.kind == none || (record.allow_while_active == 0 && state.active_flags != 0))
            return Status::advanced;
        // Gating precedes this mask test: a disabled record can still block.
        if ((record.kind & state.enabled_flags) == 0) {
            ++state.pc;
            continue;
        }
        switch (record.kind) {
        case decelerate:
            state.acceleration[0].timer = 0;
            state.acceleration[0].subframe = 0;
            break;
        case vector:
            if (install_vector_acceleration(state.acceleration[1], state.flight, record.float0,
                                            record.float1, record.int0,
                                            multiplier) != Status::advanced)
                return Status::invalid;
            break;
        case polar:
            if (!std::isfinite(record.float0) || !std::isfinite(record.float1))
                return Status::invalid;
            state.acceleration[2].timer = 0;
            state.acceleration[2].subframe = 0;
            state.acceleration[2].duration = record.int0;
            state.acceleration[2].speed_delta = record.float0;
            state.acceleration[2].angle_delta = record.float1;
            break;
        case relative:
        case absolute:
        case aimed:
            if (!std::isfinite(record.float0) || !std::isfinite(record.float1))
                return Status::invalid;
            state.turn.timer = 0;
            state.turn.subframe = 0;
            state.turn.completed = 0;
            state.turn.interval = record.int0;
            state.turn.repeats = record.int1;
            state.turn.angle = record.float0;
            state.turn.speed = record.float1 > -999.0f ? record.float1 : state.flight.speed;
            break;
        case wait:
            state.wait_timer = record.int0;
            state.wait_subframe = 0;
            break;
        case bounce_all:
        case bounce_except_bottom:
            if (!std::isfinite(record.float0))
                return Status::invalid;
            state.bounce_speed = record.float0 >= 0 ? record.float0 : state.flight.speed;
            state.bounce_count = 0;
            state.bounce_limit = record.int0;
            break;
        case wrap_x:
        case wrap_y:
            state.wrap_timer = record.int0;
            state.wrap_subframe = 0;
            break;
        case cull_delay:
            state.offscreen_cull_delay = record.int0;
            ++state.pc;
            continue;
        case sound:
            result.sounds[result.sound_count++] = {record.int0, true, state.flight.x};
            ++state.pc;
            continue;
        case despawn:
            state.despawning = true;
            ++state.pc;
            return Status::advanced;
        default:
            // Sprite replacement, child patterns and unverified
            // default cases require their own state and side-effect contracts.
            return Status::unsupported;
        }
        state.active_flags |= record.kind;
        if ((record.kind == vector || record.kind == polar) && state.pc != 0 &&
            state.transform_sound >= 0)
            result.sounds[result.sound_count++] = {state.transform_sound, false, 0};
        ++state.pc;
        return Status::advanced; // At most one non-immediate installation per call.
    }
    return Status::advanced;
}
inline Status countdown(std::int32_t &timer, float &fraction, float multiplier, bool force_extra,
                        std::uint32_t flag, std::uint32_t &active) {
    if (!std::isfinite(fraction) || fraction < 0 || fraction >= 1)
        return Status::invalid;
    if (timer <= 0) {
        active &= ~flag;
    } else {
        if (force_extra) {
            --timer;
            fraction = 0;
        }
        if (multiplier > .99f) {
            --timer;
        } else {
            fraction -= multiplier;
            if (fraction < 0) {
                --timer;
                fraction += 1;
            }
        }
    }
    return Status::advanced;
}
inline Status bounce(State &state, float multiplier, Result &result) {
    if (!std::isfinite(state.sprite_width) || !std::isfinite(state.sprite_height))
        return Status::unsupported; // The current sprite must be resolved first.
    if (state.sprite_width < 0 || state.sprite_height < 0 || !std::isfinite(state.bounce_speed) ||
        state.bounce_count < 0 || state.bounce_count == std::numeric_limits<std::int32_t>::max())
        return Status::invalid;
    auto &flight = state.flight;
    if (!(state.sprite_width / 2 + flight.x < 0 || flight.x - state.sprite_width / 2 > 384 ||
          state.sprite_height / 2 + flight.y < 0 || flight.y - state.sprite_height / 2 > 448))
        return Status::advanced;
    if (state.transform_sound >= 0)
        result.sounds[result.sound_count++] = {state.transform_sound, false, 0};
    if (flight.x < 0 || flight.x >= 384)
        flight.angle = kinematics::normalize_angle(-flight.angle - kinematics::pi);
    if (flight.y < 0 || (flight.y >= 448 && (state.active_flags & bounce_all) != 0))
        flight.angle = -flight.angle;
    // Even an excluded bottom exit resets speed and consumes a bounce count.
    flight.speed = state.bounce_speed;
    const float magnitude = flight.speed * multiplier;
    flight.velocity_x = std::cos(flight.angle) * magnitude;
    flight.velocity_y = std::sin(flight.angle) * magnitude;
    if (!std::isfinite(flight.velocity_x) || !std::isfinite(flight.velocity_y))
        return Status::invalid;
    if (++state.bounce_count >= state.bounce_limit)
        state.active_flags &= ~(bounce_all | bounce_except_bottom);
    return Status::advanced;
}
} // namespace detail

// Installation is also called once at birth, without ticking any active effects.
// Failed calls leave state unchanged and return no sound events.
inline Result advance_program(const Program &program, State &state, float multiplier) {
    Result result;
    result.pc = state.pc;
    result.status = detail::validate(state, multiplier);
    auto next = state;
    if (result.status == Status::advanced)
        result.status = detail::install(program, next, multiplier, result);
    if (result.status == Status::advanced)
        state = next;
    else
        result.sound_count = 0;
    return result;
}

// Fired pre-displacement phase only, with no cancellation or external mutations.
// force_extra_timer_step is explicit because WAIT/WRAP use ZunTimer::Decrement,
// whereas acceleration/direction use TickTimer and ignore that flag.
inline Result step(const Program &program, State &state, float multiplier,
                   bool force_extra_timer_step,
                   float angle_to_player = std::numeric_limits<float>::quiet_NaN()) {
    auto next = state;
    Result result;
    result.pc = state.pc;
    result.status = detail::validate(state, multiplier);
    if (result.status == Status::advanced)
        result.status = detail::install(program, next, multiplier, result);
    if (result.status != Status::advanced) {
        result.sound_count = 0;
        return result;
    }
    constexpr std::uint32_t acceleration_flags[] = {decelerate, vector, polar};
    for (unsigned i = 0; i < 3 && result.status == Status::advanced; ++i) {
        if ((next.active_flags & acceleration_flags[i]) == 0)
            continue;
        auto &effect = next.acceleration[i];
        if (unsigned(effect.mode) != i) {
            result.status = Status::invalid;
            break;
        }
        effect.active = true;
        result.status = advance_acceleration(next.flight, effect, multiplier);
        if (!effect.active)
            next.active_flags &= ~acceleration_flags[i];
    }
    constexpr std::uint32_t direction_flags[] = {relative, absolute, aimed};
    for (unsigned i = 0; i < 3 && result.status == Status::advanced; ++i) {
        if ((next.active_flags & direction_flags[i]) == 0)
            continue;
        next.turn.mode = TurnMode(i);
        next.turn.active = true;
        const bool fires = next.turn.timer >= next.turn.interval;
        result.status = advance_direction(next.flight, next.turn, multiplier, angle_to_player);
        if (result.status == Status::advanced && fires && next.transform_sound >= 0)
            result.sounds[result.sound_count++] = {next.transform_sound, false, 0};
        if (!next.turn.active)
            next.active_flags &= ~direction_flags[i];
    }
    if (result.status == Status::advanced &&
        (next.active_flags & (bounce_all | bounce_except_bottom)) != 0)
        result.status = detail::bounce(next, multiplier, result);
    constexpr std::uint32_t wrap_flags[] = {wrap_x, wrap_y};
    for (unsigned axis = 0; axis < 2 && result.status == Status::advanced; ++axis) {
        if ((next.active_flags & wrap_flags[axis]) == 0)
            continue;
        float &position = axis == 0 ? next.flight.x : next.flight.y;
        const float extent = axis == 0 ? 384.0f : 448.0f;
        if (position < 0)
            position += extent;
        else if (position > extent)
            position -= extent;
        result.status =
            detail::countdown(next.wrap_timer, next.wrap_subframe, multiplier,
                              force_extra_timer_step, wrap_flags[axis], next.active_flags);
    }
    if (result.status == Status::advanced && (next.active_flags & wait) != 0)
        result.status = detail::countdown(next.wait_timer, next.wait_subframe, multiplier,
                                          force_extra_timer_step, wait, next.active_flags);
    if (result.status == Status::advanced)
        state = next;
    else
        result.sound_count = 0;
    return result;
}
} // namespace th08::bullet::transform
