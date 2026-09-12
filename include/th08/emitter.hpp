#pragma once
#include "resources.hpp"
#include "rng.hpp"
#include <array>

namespace th08::emitter {
enum class Status {
    returned,
    horizon,
    unsupported,
    missing_context,
    instruction_limit,
    invalid,
    frame_complete,
    external_effect
};
const char *name(Status status);
struct Operation {
    std::int32_t time;
    std::int16_t opcode;
    std::uint16_t flags, payload_size;
    std::uint8_t mask;
    std::uint32_t offset, target = 0;
    std::array<std::uint32_t, 8> words{};
    std::uint32_t payload_offset = 0;
};
struct Program {
    std::vector<Operation> code;
    // Complete cold payloads, owned once per immutable program. Common scalar
    // operands stay inline; long world instructions are never truncated.
    resources::Bytes payloads;
    Program() = default;
    Program(resources::View resource, const resources::Ecl &ecl, std::size_t sub);
    resources::View payload(std::uint32_t pc) const;
};
struct Module {
    std::vector<Program> subs;
    Module() = default;
    Module(resources::View resource, const resources::Ecl &ecl);
};
struct CallFrame {
    const Program *program;
    std::uint32_t pc;
    std::int64_t time, wait;
    std::array<double, 101> registers;
    std::array<bool, 101> initialized;
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
    std::array<CallFrame, 15> calls{};
};
struct Result {
    Status status = Status::horizon;
    std::uint32_t tick = 0, offset = 0, executed = 0;
    std::int16_t opcode = 0;
    std::uint64_t requested_bullets = 0, digest = 1469598103934665603ULL;
    bool terminated = false; // Opcode 1 is distinct from a normal root return.
};
enum class Effects { record_requests, yield_to_world };
struct Execution {
    // Code is borrowed and immutable. Copy Execution and Workspace together to
    // fork runtime state; the underlying Program/Module must outlive both copies.
    const Program *active = nullptr;
    const Module *module = nullptr;
    std::uint32_t pc = 0;
    std::size_t depth = 0;
    std::int64_t local_time = 0, wait = 0;
    std::uint8_t mask = 0;
    bool finished = false, pending_effect = false;
    Result result;
};
Execution begin(const Program &program, Workspace &workspace, std::uint8_t mask);
Execution begin(const Module &module, std::size_t sub, Workspace &workspace, std::uint8_t mask);
// Unit-rate context scheduling, stopping at one frame boundary or world effect.
// The instruction limit is cumulative across all advances. Hard errors terminate
// this execution; retry from a caller-owned checkpoint, not its diagnostic outputs.
Result advance(Execution &execution, Workspace &workspace, random::Rng *rng = nullptr,
               std::uint32_t instruction_limit = 100000,
               Effects effects = Effects::record_requests);
// Yielded operands have NOT been read and RNG has NOT been consumed for them.
// The world must execute the pending instruction before acknowledging its token.
// Missing/rejected world behavior must remain pending, never acknowledged as a NOP.
const Operation *pending_operation(const Execution &execution);
bool acknowledge_effect(Execution &execution, std::uint32_t instruction_token);
// Restricted scalar scheduling only. No RNG defaults, movement, pool, ANM,
// shot-distance gates, collision, damage or actual player targeting is executed.
// An optional caller-owned RNG enables isolated scalar execution. Calls share it;
// at most one RNG-consuming expression is accepted per instruction. Shot requests
// stop before execution because their world-side RNG effects are not modeled.
// On failure RNG rolls back only the failing instruction, not the completed prefix.
// These bounded convenience functions start fresh on every call. Use Execution
// and advance for resumable scheduling; copy caller-owned RNG separately as well.
Result run(const Program &program, Workspace &workspace, std::uint8_t mask,
           std::uint32_t horizon = 400, std::uint32_t instruction_limit = 100000,
           random::Rng *rng = nullptr);
Result run(const Module &module, std::size_t sub, Workspace &workspace, std::uint8_t mask,
           std::uint32_t horizon = 400, std::uint32_t instruction_limit = 100000,
           random::Rng *rng = nullptr);
} // namespace th08::emitter
