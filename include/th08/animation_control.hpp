#pragma once
#include "formats.hpp"
#include "rng.hpp"
#include <array>
#include <limits>

namespace th08::animation::control {
enum class Status {
    advanced,
    completed,
    unsupported,
    invalid,
    instruction_limit,
    requires_context
};
const char *name(Status status);
struct Operation {
    std::uint32_t offset = 0, target = 0;
    std::int16_t opcode = 0, time = 0;
    std::uint16_t mask = 0, payload_size = 0;
    std::array<std::int32_t, 5> words{};
};
struct Label {
    std::int32_t value;
    std::uint32_t next;
};
struct Program {
    std::vector<Operation> code;
    std::vector<Label> labels; // Stable sorted: first exact label wins.
    std::uint32_t fallback = std::numeric_limits<std::uint32_t>::max();
    Program() = default;
    Program(resources::View bytes, const resources::Anm &anm, std::size_t script);
};
struct Clock {
    std::int32_t previous = -999, current = 0;
    float fraction = 0;
};
struct State {
    std::uint32_t pc = 0, return_pc = 0;
    Clock time, wait{0, 0, 0}, return_time;
    std::int32_t sprite = -1, last_sprite_time = 0;
    std::int16_t pending_interrupt = 0;
    bool active = true, visible = false, stopped = false, frozen = false;
    bool has_return = false;
    std::array<std::int32_t, 4> integers{};
    std::array<float, 4> floats{};
    std::array<std::int32_t, 2> counters{};
    std::int32_t player_bullet_hit_animation_type = 0;
};
struct Result {
    Status status = Status::advanced;
    std::uint32_t pc = 0, executed = 0;
};
// Lifecycle/scalar projection: clocks, sprite identity, typed variables and hit metadata.
// Visual writes cannot feed these observables and are projected out after operand checks.
// Mutable bytecode and unknown opcodes stop; RNG requires a caller-owned stream.
// Programs are immutable shared data; State owns every mutable projected field.
// A fresh State needs one advance for SetAndExecuteScript's time-zero execution.
// The caller resolves sprite resources and gates external interrupts/freezes.
// State and RNG failure are atomic. Budget is per call, including interrupt dispatch.
Result advance(const Program &program, State &state, float multiplier = 1,
               bool force_extra_timer_step = false, std::uint32_t instruction_limit = 100000,
               random::Rng *rng = nullptr);
} // namespace th08::animation::control
