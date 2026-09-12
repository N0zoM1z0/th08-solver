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
    cull_delay = 0x2000,
    wait = 0x20000,
    despawn = 0x40000,
    sound = 0x80000
};
inline constexpr std::uint32_t motion_flags =
    decelerate | vector | polar | relative | aimed | absolute | wait;
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
    // Eighteen immediate records plus three overlapping direction firings.
    std::array<SoundEvent, 21> sounds{};
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
            // Sprite replacement, bounce/wrap, child patterns and unverified
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
// force_extra_timer_step is explicit because WAIT uses ZunTimer::Decrement,
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
    if (result.status == Status::advanced && (next.active_flags & wait) != 0) {
        if (!std::isfinite(next.wait_subframe) || next.wait_subframe < 0 ||
            next.wait_subframe >= 1) {
            result.status = Status::invalid;
        } else if (next.wait_timer <= 0) {
            next.active_flags &= ~wait;
        } else {
            if (force_extra_timer_step) {
                --next.wait_timer;
                next.wait_subframe = 0;
            }
            if (multiplier > .99f) {
                --next.wait_timer;
            } else {
                next.wait_subframe -= multiplier;
                if (next.wait_subframe < 0) {
                    --next.wait_timer;
                    next.wait_subframe += 1;
                }
            }
        }
    }
    if (result.status == Status::advanced)
        state = next;
    else
        result.sound_count = 0;
    return result;
}
} // namespace th08::bullet::transform
