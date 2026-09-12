#pragma once
#include "resources.hpp"
#include <array>

namespace th08::emitter {
enum class Status { returned, horizon, unsupported, missing_context, instruction_limit, invalid };
const char *name(Status status);
struct Operation {
    std::int32_t time;
    std::int16_t opcode;
    std::uint16_t flags, payload_size;
    std::uint8_t mask;
    std::uint32_t offset, target = 0;
    std::array<std::uint32_t, 8> words{};
};
struct Program {
    std::vector<Operation> code;
    Program() = default;
    Program(resources::View resource, const resources::Ecl &ecl, std::size_t sub);
};
struct Emission {
    std::uint32_t tick, offset;
    std::int16_t opcode;
    std::array<std::uint32_t, 8> words;
    // All transform writes before this index apply to this emission request.
    std::size_t transform_write_count;
};
struct TransformWrite {
    std::uint32_t tick;
    std::array<std::uint32_t, 7> words;
};
struct Workspace {
    // Reused across candidates/runs; no hash maps or per-event allocation.
    std::array<double, 101> registers{};
    std::array<bool, 101> initialized{};
    std::vector<Emission> emissions;
    std::vector<TransformWrite> transforms;
};
struct Result {
    Status status = Status::horizon;
    std::uint32_t tick = 0, offset = 0, executed = 0;
    std::int16_t opcode = 0;
    std::uint64_t requested_bullets = 0, digest = 1469598103934665603ULL;
};
// Restricted scalar scheduling only. No RNG defaults, movement, pool, ANM,
// shot-distance gates, collision, damage or actual player targeting is executed.
Result run(const Program &program, Workspace &workspace, std::uint8_t mask,
           std::uint32_t horizon = 400, std::uint32_t instruction_limit = 100000);
} // namespace th08::emitter
