#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <th08/animation_control.hpp>
#include <th08/kinematics.hpp>
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
std::int32_t integer(std::int64_t value) {
    require(value >= INT32_MIN && value <= INT32_MAX);
    return std::int32_t(value);
}
std::int32_t integer(float value) {
    require(std::isfinite(value) && double(value) >= INT32_MIN && double(value) <= INT32_MAX);
    return std::int32_t(value);
}
float finite(float value) {
    require(std::isfinite(value));
    return value;
}
float floating(std::int32_t word) {
    float value;
    std::memcpy(&value, &word, sizeof(value));
    return finite(value);
}
struct Operands {
    const Operation &operation;
    State &state;
    bool masked(unsigned index) const {
        return (operation.mask & (1U << index)) != 0;
    }
    std::int32_t read_int(unsigned index) const {
        const auto value = operation.words[index];
        if (!masked(index))
            return value;
        if (value >= 10000 && value < 10004)
            return state.integers[value - 10000];
        if (value >= 10004 && value < 10008)
            return integer(state.floats[value - 10004]);
        if (value >= 10008 && value < 10010)
            return state.counters[value - 10008];
        return value;
    }
    float read_float(unsigned index) const {
        const auto value = floating(operation.words[index]);
        if (!masked(index))
            return value;
        const auto selector = integer(value); // Source truncates float selectors before dispatch.
        if (selector >= 10000 && selector < 10004)
            return float(state.integers[selector - 10000]);
        if (selector >= 10004 && selector < 10008)
            return finite(state.floats[selector - 10004]);
        if (selector >= 10008 && selector < 10010)
            return float(state.counters[selector - 10008]);
        return value;
    }
    std::int32_t &write_int(unsigned index = 0) const {
        require(masked(index), Status::unsupported);
        const auto selector = operation.words[index];
        if (selector >= 10000 && selector < 10004)
            return state.integers[selector - 10000];
        if (selector >= 10008 && selector < 10010)
            return state.counters[selector - 10008];
        throw Blocked{Status::unsupported}; // Otherwise the source mutates instruction storage.
    }
    float &write_float(unsigned index = 0) const {
        require(masked(index), Status::unsupported);
        const auto selector = integer(floating(operation.words[index]));
        require(selector >= 10004 && selector < 10008, Status::unsupported);
        return state.floats[selector - 10004];
    }
};
std::int32_t calculate_int(unsigned operation, std::int32_t left, std::int32_t right) {
    switch (operation) {
    case 0:
        return integer(std::int64_t(left) + right);
    case 1:
        return integer(std::int64_t(left) - right);
    case 2:
        return integer(std::int64_t(left) * right);
    default:
        require(right != 0 && !(left == INT32_MIN && right == -1));
        return operation == 3 ? left / right : left % right;
    }
}
float calculate_float(unsigned operation, float left, float right) {
    switch (operation) {
    case 0:
        return finite(left + right);
    case 1:
        return finite(left - right);
    case 2:
        return finite(left * right);
    case 3:
        require(right != 0);
        return finite(left / right);
    default:
        require(right != 0);
        return finite(std::fmod(left, right));
    }
}
template <class T> bool compare(unsigned operation, T left, T right) {
    switch (operation) {
    case 0:
        return left == right;
    case 1:
        return left != right;
    case 2:
        return left < right;
    case 3:
        return left <= right;
    case 4:
        return left > right;
    default:
        return left >= right;
    }
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
// These writes cannot feed scalar/control observables. Their typed reads are
// checked below; they neither draw RNG nor write script variables.
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
void check_visual_operands(const Operands &args) {
    unsigned float_mask = 0, int_mask = 0;
    switch (args.operation.opcode) {
    case 6:
    case 12:
    case 13:
        float_mask = 7;
        break;
    case 7:
    case 14:
        float_mask = 3;
        break;
    case 8:
    case 85:
        int_mask = 1;
        break;
    case 9:
    case 84:
        int_mask = 7;
        break;
    case 15:
        int_mask = 2;
        break;
    case 17:
    case 18:
    case 19:
        float_mask = 7;
        int_mask = 8;
        break;
    case 26:
    case 27:
    case 80:
    case 81:
        float_mask = 1;
        break;
    case 29:
        float_mask = 3;
        int_mask = 4;
        break;
    case 32:
    case 35:
        float_mask = 28;
        int_mask = 1;
        break;
    case 33:
    case 86:
        int_mask = 29;
        break;
    case 34:
    case 87:
        int_mask = 5;
        break;
    case 36:
        float_mask = 12;
        int_mask = 1;
        break;
    default:
        break; // Remaining fields are raw literals, even when a mask bit is set.
    }
    for (unsigned index = 0; index < 5; ++index) {
        if (float_mask & (1U << index))
            args.read_float(index);
        if (int_mask & (1U << index))
            args.read_int(index);
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
    case Status::requires_context:
        return "REQUIRES_CONTEXT";
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
        const int target_word = op.opcode == 4                       ? 0
                                : op.opcode == 5                     ? 1
                                : op.opcode >= 67 && op.opcode <= 78 ? 2
                                                                     : -1;
        if (target_word >= 0 && op.payload_size == unsigned(target_word + 2) * 4) {
            const auto target = std::int64_t(range.offset) + op.words[target_word];
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
               std::uint32_t instruction_limit, random::Rng *rng) {
    Result result;
    auto next = state;
    const auto rng_entry = rng ? rng->state() : random::State{};
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
            const Operands args{op, next};
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
                next.sprite = args.read_int(0);
                require(next.sprite >= 0 && next.sprite <= 32767, Status::unsupported);
                next.visible = true;
                next.last_sprite_time = next.time.current;
                break;
            case 4:
                require(op.payload_size == 8 && op.target < program.code.size());
                set(next.time, op.words[1]);
                next.pc = op.target;
                continue;
            case 5:
                require(op.payload_size == 12 && op.target < program.code.size());
                args.write_int() = integer(std::int64_t(args.write_int()) - 1);
                if (args.read_int(0) > 0) {
                    set(next.time, op.words[2]);
                    next.pc = op.target;
                    continue;
                }
                break;
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
                    set(next.wait, args.read_int(0));
                else
                    decrement(next.wait, multiplier, extra);
                if (next.wait.current <= 0) {
                    set(next.wait, 0);
                    break;
                }
                freeze_time();
                goto frame_tail;
            case 83:
                require(op.payload_size == 4);
                next.player_bullet_hit_animation_type = op.words[0];
                break;
            case 89:
                require(op.payload_size == 0 && next.has_return);
                next.time = next.return_time;
                next.pc = next.return_pc;
                continue;
            default: {
                if (op.opcode >= 37 && op.opcode <= 58) {
                    const bool floating = op.opcode % 2 == 0;
                    const bool compound = op.opcode >= 39 && op.opcode <= 48;
                    const bool assignment = op.opcode <= 38;
                    require(op.payload_size == (op.opcode >= 49 ? 12 : 8));
                    const unsigned operation = unsigned(op.opcode - (compound ? 39 : 49)) / 2;
                    if (floating) {
                        auto &destination = args.write_float();
                        destination = assignment
                                          ? args.read_float(1)
                                          : calculate_float(operation,
                                                            compound ? finite(destination)
                                                                     : args.read_float(1),
                                                            args.read_float(compound ? 1 : 2));
                    } else {
                        auto &destination = args.write_int();
                        destination = assignment
                                          ? args.read_int(1)
                                          : calculate_int(operation,
                                                          compound ? destination : args.read_int(1),
                                                          args.read_int(compound ? 1 : 2));
                    }
                    break;
                }
                if (op.opcode == 59 || op.opcode == 60) {
                    require(op.payload_size == 8);
                    if (op.opcode == 59) {
                        auto &destination = args.write_int();
                        const auto range = std::uint32_t(args.read_int(1));
                        require(rng != nullptr, Status::requires_context);
                        const auto value = rng->range_u32(range);
                        std::memcpy(&destination, &value, sizeof(value));
                    } else {
                        auto &destination = args.write_float();
                        const auto range = args.read_float(1);
                        require(rng != nullptr, Status::requires_context);
                        destination = finite(rng->range_float(range));
                    }
                    break;
                }
                if (op.opcode >= 61 && op.opcode <= 66) {
                    require(op.payload_size == (op.opcode == 66 ? 4 : 8));
                    auto &destination = args.write_float();
                    const auto value = args.read_float(op.opcode == 66 ? 0 : 1);
                    switch (op.opcode) {
                    case 61:
                        destination = finite(std::sin(value));
                        break;
                    case 62:
                        destination = finite(std::cos(value));
                        break;
                    case 63:
                        destination = finite(std::tan(value));
                        break;
                    case 64:
                        destination = finite(std::acos(value));
                        break;
                    case 65:
                        destination = finite(std::atan(value));
                        break;
                    case 66:
                        destination = kinematics::normalize_angle(value);
                        break;
                    }
                    break;
                }
                if (op.opcode >= 67 && op.opcode <= 78) {
                    require(op.payload_size == 16 && op.target < program.code.size());
                    const unsigned comparison = unsigned(op.opcode - 67) / 2;
                    const bool taken =
                        op.opcode % 2 ? compare(comparison, args.read_int(0), args.read_int(1))
                                      : compare(comparison, args.read_float(0), args.read_float(1));
                    if (taken) {
                        set(next.time, op.words[3]);
                        next.pc = op.target;
                        continue;
                    }
                    break;
                }
                const auto words = visual_words(op.opcode);
                require(words >= 0, Status::unsupported);
                require(op.payload_size == unsigned(words) * 4);
                check_visual_operands(args);
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
        if (rng)
            *rng = random::Rng(rng_entry);
        result.status = error.status;
    }
    return result;
}
} // namespace th08::animation::control
