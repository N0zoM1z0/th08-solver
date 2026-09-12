#include <cmath>
#include <limits>
#include <stdexcept>
#include <th08/timeline.hpp>
#include <th08/timing.hpp>

namespace th08::timeline {
namespace {
constexpr std::array<unsigned, 17> payload_sizes = {24, 24, 28, 20, 28, 20, 4,  0, 8,
                                                    4,  4,  28, 28, 4,  4,  24, 0};
struct Blocked {
    Status status;
};
void require(bool condition, Status status = Status::invalid) {
    if (!condition)
        throw Blocked{status};
}
bool read(KnownBool value) {
    require(value != KnownBool::unknown, Status::requires_context);
    require(value == KnownBool::no || value == KnownBool::yes);
    return value == KnownBool::yes;
}
void wait(Clock &clock, float multiplier, bool extra) {
    require(clock.current > std::numeric_limits<std::int32_t>::min() + 1);
    if (extra) {
        --clock.current;
        clock.fraction = 0;
        clock.previous = -999;
    }
    if (multiplier > .99f) {
        --clock.current;
    } else {
        clock.previous = clock.current;
        clock.fraction -= multiplier;
        if (clock.fraction < 0) {
            --clock.current;
            clock.fraction += 1;
        }
    }
}
void validate(bool condition) {
    if (!condition)
        throw std::runtime_error("invalid timeline program input");
}
} // namespace

const char *name(Status status) {
    switch (status) {
    case Status::frame_complete:
        return "FRAME_COMPLETE";
    case Status::external_effect:
        return "EXTERNAL_EFFECT";
    case Status::requires_context:
        return "REQUIRES_CONTEXT";
    case Status::unsupported:
        return "UNSUPPORTED";
    case Status::invalid:
        return "INVALID";
    case Status::instruction_limit:
        return "INSTRUCTION_LIMIT";
    }
    return "INVALID";
}

Program::Program(resources::View resource, const resources::Ecl &ecl, std::size_t timeline) {
    const auto &range = ecl.timelines.at(timeline);
    validate(range.first <= ecl.timeline_instructions.size() &&
             range.count <= ecl.timeline_instructions.size() - range.first);
    validate(range.offset <= range.terminal && range.terminal <= resource.size &&
             resource.size - range.terminal >= 8 && resources::i32(resource, range.terminal) < 0);
    code.reserve(std::size_t(range.count) + 1);
    payloads.reserve(range.terminal - range.offset);
    std::uint32_t expected_offset = range.offset;
    for (std::size_t n = range.first; n < std::size_t(range.first) + range.count; ++n) {
        const auto &raw = ecl.timeline_instructions[n];
        validate(raw.offset == expected_offset && raw.offset <= range.terminal && raw.size >= 8 &&
                 raw.size <= range.terminal - raw.offset && raw.time >= 0);
        validate(resources::i32(resource, raw.offset) == raw.time &&
                 resources::u16(resource, raw.offset + 4) == raw.opcode &&
                 resource.data[raw.offset + 6] == raw.size &&
                 resource.data[raw.offset + 7] == raw.mask);
        const auto size = unsigned(raw.size - 8);
        validate(size % 4 == 0 &&
                 (raw.opcode >= payload_sizes.size() || size == payload_sizes[raw.opcode]));
        Operation op;
        op.offset = raw.offset;
        op.time = raw.time;
        op.opcode = raw.opcode;
        op.mask = raw.mask;
        op.payload_size = std::uint16_t(size);
        op.payload_offset = std::uint32_t(payloads.size());
        const auto bytes = resource.sub(raw.offset + 8, size);
        payloads.insert(payloads.end(), bytes.data, bytes.data + bytes.size);
        for (std::size_t word = 0; word < size / 4 && word < op.words.size(); ++word)
            op.words[word] = resources::i32(bytes, word * 4);
        code.push_back(op);
        expected_offset += raw.size;
    }
    validate(expected_offset == range.terminal);
    Operation terminal;
    terminal.offset = range.terminal;
    terminal.time = resources::i32(resource, range.terminal);
    terminal.opcode = resources::u16(resource, range.terminal + 4);
    terminal.mask = resource.data[range.terminal + 7];
    terminal.payload_offset = std::uint32_t(payloads.size());
    code.push_back(terminal);
}

resources::View Program::payload(std::uint32_t pc) const {
    const auto &op = code.at(pc);
    if (op.payload_offset > payloads.size() ||
        op.payload_size > payloads.size() - op.payload_offset)
        throw std::runtime_error("timeline payload outside owned arena");
    if (!op.payload_size)
        return {};
    return {payloads.data() + op.payload_offset, op.payload_size};
}

const Operation *pending_operation(const Program &program, const State &state) {
    if (!state.pending_effect || !state.effect_token || state.pc >= program.code.size())
        return nullptr;
    const auto &op = program.code[state.pc];
    if (op.time < 0 || op.opcode >= payload_sizes.size() ||
        op.payload_size != payload_sizes[op.opcode] || op.opcode == 7 || op.opcode == 10 ||
        op.opcode == 13 || op.opcode == 14)
        return nullptr;
    return &op;
}
bool acknowledge_effect(const Program &program, State &state, std::uint64_t token) {
    if (!pending_operation(program, state) || token != state.effect_token ||
        state.pc == std::numeric_limits<std::uint32_t>::max())
        return false;
    ++state.pc;
    state.pending_effect = false;
    return true;
}

Result advance(const Program &program, State &state, Context &context, std::uint8_t difficulty_mask,
               float multiplier, bool extra, std::uint32_t instruction_limit) {
    State next = state;
    Context world = context;
    Result result;
    auto identify = [&]() {
        result.pc = next.pc;
        if (next.pc < program.code.size()) {
            result.offset = program.code[next.pc].offset;
            result.opcode = program.code[next.pc].opcode;
        }
    };
    try {
        identify();
        require(next.pc < program.code.size());
        if (next.pending_effect) {
            require(pending_operation(program, next) != nullptr);
            result.status = Status::external_effect;
            result.effect_token = next.effect_token;
            return result;
        }
        require(std::isfinite(multiplier) && multiplier > 0 && std::isfinite(next.time.fraction) &&
                next.time.fraction >= 0 && next.time.fraction < 1);
        for (;;) {
            identify();
            require(next.pc < program.code.size());
            const auto &op = program.code[next.pc];
            if (op.time < 0) {
                result.at_end = true;
                break;
            }
            if (next.time.current < op.time)
                break;
            require(result.examined < instruction_limit, Status::instruction_limit);
            ++result.examined;
            if (next.time.current > op.time || (op.mask & difficulty_mask) == 0) {
                ++next.pc;
                continue;
            }
            require(op.opcode < payload_sizes.size(), Status::unsupported);
            require(op.payload_size == payload_sizes[op.opcode]);
            bool external = false, waiting = false;
            switch (op.opcode) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 11:
            case 12:
                // Preserve source short-circuiting: a present GUI boss avoids the
                // suppression read, and rejected spawn requests never draw RNG.
                external = !read(world.gui_boss_present) && !read(world.spawns_suppressed);
                break;
            case 6:
            case 8:
            case 9:
            case 15:
            case 16:
                external = true;
                break;
            case 7:
                waiting = read(world.message_waiting);
                break;
            case 10:
                require(op.words[0] >= 0 && std::size_t(op.words[0]) < world.boss_active.size());
                waiting = read(world.boss_active[op.words[0]]);
                break;
            case 13:
                require(world.events_known, Status::requires_context);
                waiting = true;
                for (auto &event : world.events) {
                    if (event == op.words[0]) {
                        event = -1;
                        waiting = false;
                    }
                }
                break;
            case 14:
                require(world.events_known, Status::requires_context);
                // The source broadcasts to EVERY free slot, not the first one.
                for (auto &event : world.events)
                    if (event < 0)
                        event = op.words[0];
                break;
            }
            if (external) {
                require(next.effect_token < std::numeric_limits<std::uint64_t>::max());
                ++next.effect_token;
                next.pending_effect = true;
                result.effect_token = next.effect_token;
                result.status = Status::external_effect;
                state = next;
                context = world;
                return result;
            }
            if (waiting) {
                wait(next.time, multiplier, extra);
                break;
            }
            ++next.pc;
        }
        require(next.time.current < std::numeric_limits<std::int32_t>::max());
        next.time.previous = next.time.current;
        timing::tick(next.time.current, next.time.fraction, multiplier);
        state = next;
        context = world;
    } catch (const Blocked &blocked) {
        result.status = blocked.status;
    }
    return result;
}
} // namespace th08::timeline
