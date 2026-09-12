#pragma once
#include "resources.hpp"
#include <array>

namespace th08::timeline {
enum class Status {
    frame_complete,
    external_effect,
    requires_context,
    unsupported,
    invalid,
    instruction_limit
};
const char *name(Status status);
struct Operation {
    std::uint32_t offset = 0, payload_offset = 0;
    std::int32_t time = -1;
    std::uint16_t opcode = 0, payload_size = 0;
    std::uint8_t mask = 0;
    std::array<std::int32_t, 7> words{};
};
struct Program {
    std::vector<Operation> code; // Includes the source's negative-time sentinel.
    resources::Bytes payloads;
    Program() = default;
    Program(resources::View resource, const resources::Ecl &ecl, std::size_t timeline);
    resources::View payload(std::uint32_t pc) const;
};
enum class KnownBool : std::uint8_t { unknown, no, yes };
struct Context {
    KnownBool gui_boss_present = KnownBool::unknown;
    KnownBool spawns_suppressed = KnownBool::unknown;
    KnownBool message_waiting = KnownBool::unknown;
    // no means null or inactive, both equivalent for opcode 10 only.
    std::array<KnownBool, 8> boss_active{};
    std::array<std::int32_t, 4> events{};
    bool events_known = false;
};
struct Clock {
    // EnemyManager::Initialize zeroes the timeline timer after construction.
    std::int32_t previous = 0, current = 0;
    float fraction = 0;
};
struct State {
    Clock time;
    std::uint32_t pc = 0;
    std::uint64_t effect_token = 0;
    bool pending_effect = false;
};
struct Result {
    Status status = Status::frame_complete;
    std::uint32_t pc = 0, offset = 0, examined = 0;
    std::uint16_t opcode = 0;
    std::uint64_t effect_token = 0;
    bool at_end = false;
};
// Runs through source control/gates to a frame boundary or an external effect.
// Difficulty is any-bit intersection, unlike enemy ECL mask containment.
// Known context fields are caller-owned current observations, never inferred.
// Event publish/consume is committed to Context with the State. Copy both to fork.
// Errors/missing context roll back this call, not earlier acknowledged effects.
// A negative-time sentinel still ticks on every call; at_end is not a stop latch.
Result advance(const Program &program, State &state, Context &context, std::uint8_t difficulty_mask,
               float multiplier = 1, bool force_extra_timer_step = false,
               std::uint32_t instruction_limit = 100000);
// Spawn/message/boss-sub/power/menu effects have not read operands or consumed RNG.
// The owner must apply the complete pending effect before acknowledging its token.
// Source gates have already been evaluated; an acknowledged effect is not a NOP.
const Operation *pending_operation(const Program &program, const State &state);
bool acknowledge_effect(const Program &program, State &state, std::uint64_t token);
} // namespace th08::timeline
