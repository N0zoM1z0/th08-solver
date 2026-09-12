#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <th08/animation_control.hpp>
#include <th08/timing.hpp>

namespace th08::animation::control {
namespace {
struct Blocked {
    Status status;
};
void require(bool condition, Status status = Status::invalid) {
    if (!condition)
        throw Blocked{status};
}
bool valid(const Clock &clock) {
    return std::isfinite(clock.fraction) && clock.fraction >= 0 && clock.fraction < 1;
}
void set(Clock &clock, std::int32_t value) {
    clock = {-999, value, 0};
}
void decrement(Clock &clock, float multiplier, bool extra) {
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
// Payload words for literal writes that cannot change this projection. In
// particular, no accepted instruction draws RNG or writes script variables.
int visual_words(std::int16_t opcode) {
    switch (opcode) {
    case 0:
    case 10:
    case 11:
    case 22:
        return 0;
    case 8:
    case 16:
    case 24:
    case 25:
    case 26:
    case 27:
    case 30:
    case 31:
    case 80:
    case 81:
    case 82:
    case 85:
    case 88:
        return 1;
    case 7:
    case 14:
    case 15:
        return 2;
    case 6:
    case 9:
    case 12:
    case 13:
    case 29:
    case 34:
    case 84:
    case 87:
        return 3;
    case 17:
    case 18:
    case 19:
    case 36:
        return 4;
    case 32:
    case 33:
    case 35:
    case 86:
        return 5;
    default:
        return -1;
    }
}
} // namespace
const char *name(Status status) {
    switch (status) {
    case Status::advanced:
        return "ADVANCED";
    case Status::completed:
        return "COMPLETED";
    case Status::unsupported:
        return "UNSUPPORTED";
    case Status::invalid:
        return "INVALID";
    case Status::instruction_limit:
        return "INSTRUCTION_LIMIT";
    }
    return "INVALID";
}
Program::Program(resources::View bytes, const resources::Anm &anm, std::size_t script) {
    const auto &range = anm.scripts.at(script);
    if (range.first > anm.instructions.size() || !range.count ||
        range.count > anm.instructions.size() - range.first)
        throw std::runtime_error("invalid ANM control instruction range");
    code.reserve(range.count);
    for (std::size_t i = range.first; i < std::size_t(range.first) + range.count; ++i) {
        const auto &ins = anm.instructions[i];
        bytes.sub(ins.offset, 8);
        Operation op;
        op.offset = ins.offset;
        op.opcode = ins.opcode;
        op.time = ins.time;
        op.mask = ins.mask;
        if (ins.opcode != -1) {
            if (ins.size < 8)
                throw std::runtime_error("invalid ANM control payload size");
            const auto payload = bytes.sub(std::size_t(ins.offset) + 8, ins.size - 8);
            op.payload_size = std::uint16_t(payload.size);
            for (std::size_t word = 0; word < std::min(op.words.size(), payload.size / 4); ++word)
                op.words[word] = resources::i32(payload, word * 4);
        }
        code.push_back(op);
    }
    if (code.back().opcode != -1)
        throw std::runtime_error("ANM control program has no sentinel");
    for (std::size_t pc = 0; pc < code.size(); ++pc) {
        auto &op = code[pc];
        if (op.opcode == 4 && op.payload_size == 8) {
            const auto target = std::int64_t(range.offset) + op.words[0];
            const auto found =
                std::lower_bound(code.begin(), code.end(), target,
                                 [](const Operation &candidate, std::int64_t offset) {
                                     return candidate.offset < offset;
                                 });
            if (found == code.end() || found->offset != target)
                throw std::runtime_error("ANM jump does not target an instruction boundary");
            op.target = std::uint32_t(found - code.begin());
        }
        if (op.opcode == 21 && op.payload_size == 4) {
            labels.push_back({op.words[0], std::uint32_t(pc + 1)});
            if (op.words[0] == -1)
                fallback = std::uint32_t(pc + 1);
        }
    }
    std::stable_sort(labels.begin(), labels.end(),
                     [](const Label &a, const Label &b) { return a.value < b.value; });
}
Result advance(const Program &program, State &state, float multiplier, bool extra,
               std::uint32_t instruction_limit) {
    Result result;
    auto next = state;
    result.pc = next.pc;
    try {
        // Source null-instruction check precedes the external freeze flag.
        if (!next.active) {
            result.status = Status::completed;
            return result;
        }
        if (next.frozen)
            return result;
        require(std::isfinite(multiplier) && multiplier > 0 && valid(next.time) &&
                valid(next.wait) && valid(next.return_time) && instruction_limit > 0);
        auto count = [&]() {
            require(result.executed < instruction_limit, Status::instruction_limit);
            ++result.executed;
        };
        auto freeze_time = [&]() { decrement(next.time, multiplier, extra); };
        if (next.pending_interrupt != 0) {
            count();
            const auto label = std::lower_bound(
                program.labels.begin(), program.labels.end(), next.pending_interrupt,
                [](const Label &candidate, std::int32_t value) { return candidate.value < value; });
            const auto target =
                label != program.labels.end() && label->value == next.pending_interrupt
                    ? label->next
                    : program.fallback;
            next.pending_interrupt = 0;
            next.stopped = false;
            if (target == std::numeric_limits<std::uint32_t>::max()) {
                freeze_time();
                goto frame_tail;
            }
            require(target < program.code.size());
            next.return_time = next.time;
            next.return_pc = next.pc;
            next.has_return = true;
            next.pc = target;
            set(next.time, program.code[target].time);
            next.visible = true;
        }
        for (;;) {
            result.pc = next.pc;
            require(next.pc < program.code.size());
            const auto &op = program.code[next.pc];
            if (op.time > next.time.current)
                break;
            count();
            require(op.mask == 0, Status::unsupported);
            switch (op.opcode) {
            case -1:
            case 1:
            case 2:
                require(op.opcode == -1 || op.payload_size == 0);
                if (op.opcode != 2)
                    next.visible = false;
                next.active = false;
                state = next;
                result.status = Status::completed;
                return result;
            case 3:
                require(op.payload_size == 4);
                require(op.words[0] >= 0 && op.words[0] <= 32767, Status::unsupported);
                next.sprite = op.words[0];
                next.visible = true;
                next.last_sprite_time = next.time.current;
                break;
            case 4:
                require(op.payload_size == 8 && op.target < program.code.size());
                set(next.time, op.words[1]);
                next.pc = op.target;
                continue;
            case 20:
            case 23:
                require(op.payload_size == 0);
                if (op.opcode == 23)
                    next.visible = false;
                next.stopped = true;
                freeze_time();
                goto frame_tail;
            case 21:
                require(op.payload_size == 4);
                break;
            case 28:
                require(op.payload_size == 4);
                next.visible = (std::uint32_t(op.words[0]) & 1U) != 0;
                break;
            case 79:
                require(op.payload_size == 4);
                if (next.wait.current == 0)
                    set(next.wait, op.words[0]);
                else
                    decrement(next.wait, multiplier, extra);
                if (next.wait.current <= 0) {
                    set(next.wait, 0);
                    break;
                }
                freeze_time();
                goto frame_tail;
            case 89:
                require(op.payload_size == 0 && next.has_return);
                next.time = next.return_time;
                next.pc = next.return_pc;
                continue;
            default: {
                const auto words = visual_words(op.opcode);
                require(words >= 0, Status::unsupported);
                require(op.payload_size == unsigned(words) * 4);
                break;
            }
            }
            ++next.pc;
        }
    frame_tail:
        require(next.time.current < std::numeric_limits<std::int32_t>::max());
        next.time.previous = next.time.current;
        timing::tick(next.time.current, next.time.fraction, multiplier);
        state = next;
    } catch (const Blocked &error) {
        result.status = error.status;
    }
    return result;
}
} // namespace th08::animation::control
