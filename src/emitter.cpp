#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <th08/emitter.hpp>
#include <th08/kinematics.hpp>

namespace th08::emitter {
namespace {
float as_float(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, 4);
    return value;
}
std::uint32_t as_bits(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    return bits;
}
std::int32_t as_int(std::uint32_t bits) {
    std::int32_t value;
    std::memcpy(&value, &bits, 4);
    return value;
}
struct Blocked {
    Status status;
};
void require(bool condition, Status status = Status::invalid) {
    if (!condition)
        throw Blocked{status};
}
// These are isolated scalar storage locations, not computed engine properties.
bool writable_scalar(std::size_t slot, bool floating) {
    if (floating)
        return (slot >= 16 && slot < 32) || (slot >= 57 && slot <= 60) ||
               (slot >= 65 && slot <= 68) || (slot >= 94 && slot <= 95);
    return slot < 16 || (slot >= 36 && slot <= 39) || (slot >= 53 && slot <= 56) ||
           (slot >= 61 && slot <= 64);
}
std::int32_t checked_integer(double value) {
    require(std::isfinite(value) && value >= std::numeric_limits<std::int32_t>::min() &&
            value <= std::numeric_limits<std::int32_t>::max());
    return static_cast<std::int32_t>(value); // Retail float-to-int reads truncate toward zero.
}
double arithmetic(unsigned operation, bool floating, double left, double right) {
    if (floating) {
        const float a = float(left), b = float(right);
        switch (operation) {
        case 0:
            return a + b;
        case 1:
            return a - b;
        case 2:
            return a * b;
        case 3:
            require(b != 0);
            return a / b;
        case 4:
            require(b != 0);
            return std::fmod(a, b);
        }
    } else {
        const std::int64_t a = checked_integer(left), b = checked_integer(right);
        std::int64_t value = 0;
        switch (operation) {
        case 0:
            value = a + b;
            break;
        case 1:
            value = a - b;
            break;
        case 2:
            value = a * b;
            break;
        case 3:
        case 4:
            require(b != 0 && !(a == std::numeric_limits<std::int32_t>::min() && b == -1));
            value = operation == 3 ? a / b : a % b;
            break;
        }
        // Overflow behavior is outside this verified subset; never invoke signed UB.
        require(value >= std::numeric_limits<std::int32_t>::min() &&
                value <= std::numeric_limits<std::int32_t>::max());
        return double(value);
    }
    throw Blocked{Status::invalid};
}
} // namespace
const char *name(Status status) {
    switch (status) {
    case Status::returned:
        return "RETURNED_SLICE";
    case Status::horizon:
        return "BOUNDED_PREFIX";
    case Status::unsupported:
        return "UNSUPPORTED";
    case Status::missing_context:
        return "REQUIRES_CONTEXT";
    case Status::instruction_limit:
        return "INSTRUCTION_LIMIT";
    case Status::invalid:
        return "INVALID";
    case Status::frame_complete:
        return "FRAME_COMPLETE";
    case Status::external_effect:
        return "EXTERNAL_EFFECT";
    }
    return "INVALID";
}
Program::Program(resources::View resource, const resources::Ecl &ecl, std::size_t sub) {
    const auto &range = ecl.subs.at(sub);
    if (range.first > ecl.instructions.size() ||
        range.count > ecl.instructions.size() - range.first)
        throw std::runtime_error("invalid compiled instruction range");
    std::uint64_t payload_bytes = 0;
    for (std::size_t i = range.first; i < std::size_t(range.first) + range.count; ++i) {
        const auto &instruction = ecl.instructions[i];
        if (instruction.size < 12)
            throw std::runtime_error("invalid compiled instruction size");
        resource.sub(instruction.offset, instruction.size);
        payload_bytes += instruction.size - 12;
    }
    if (payload_bytes > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error("compiled payload arena exceeds offset range");
    payloads.reserve(std::size_t(payload_bytes));
    code.reserve(range.count);
    for (std::size_t i = range.first; i < std::size_t(range.first) + range.count; ++i) {
        const auto &instruction = ecl.instructions[i];
        Operation op{instruction.time,
                     instruction.opcode,
                     instruction.flags,
                     std::uint16_t(instruction.size - 12),
                     instruction.mask,
                     instruction.offset,
                     0,
                     {}};
        op.payload_offset = std::uint32_t(payloads.size());
        if (op.payload_size != 0) {
            const auto bytes = resource.sub(std::size_t(instruction.offset) + 12, op.payload_size);
            payloads.insert(payloads.end(), bytes.data, bytes.data + bytes.size);
        }
        for (std::size_t word = 0;
             word < std::min(std::size_t(op.payload_size / 4), op.words.size()); ++word)
            op.words[word] = resources::u32(resource, instruction.offset + 12 + word * 4);
        code.push_back(op);
    }
    for (auto &op : code)
        if (op.opcode == 4 || op.opcode == 5 || (op.opcode >= 40 && op.opcode <= 51)) {
            const unsigned displacement_word = op.opcode >= 40 ? 3 : 1;
            const auto offset = std::int64_t(op.offset) + as_int(op.words[displacement_word]);
            const auto found = std::lower_bound(code.begin(), code.end(), offset,
                                                [](const Operation &candidate, std::int64_t value) {
                                                    return candidate.offset < value;
                                                });
            if (found == code.end() || found->offset != offset)
                throw std::runtime_error("invalid compiled jump");
            op.target = std::uint32_t(found - code.begin());
        }
}
resources::View Program::payload(std::uint32_t pc) const {
    const auto &op = code.at(pc);
    if (op.payload_offset > payloads.size() ||
        op.payload_size > payloads.size() - op.payload_offset)
        throw std::runtime_error("missing compiled instruction payload");
    if (op.payload_size == 0)
        return {};
    return {payloads.data() + op.payload_offset, op.payload_size};
}
Module::Module(resources::View resource, const resources::Ecl &ecl) {
    subs.reserve(ecl.subs.size());
    for (std::size_t sub = 0; sub < ecl.subs.size(); ++sub)
        subs.emplace_back(resource, ecl, sub);
}
Execution begin(const Program &program, Workspace &workspace, std::uint8_t mask) {
    workspace.initialized.fill(false);
    workspace.emissions.clear();
    workspace.transforms.clear();
    Execution execution;
    execution.active = &program;
    execution.mask = mask;
    if (mask == 0) {
        execution.result.status = Status::invalid;
        execution.finished = true;
    }
    return execution;
}
Execution begin(const Module &module, std::size_t sub, Workspace &workspace, std::uint8_t mask) {
    if (sub >= module.subs.size()) {
        Execution execution;
        execution.result.status = Status::invalid;
        execution.finished = true;
        return execution;
    }
    auto execution = begin(module.subs[sub], workspace, mask);
    execution.module = &module;
    return execution;
}
const Operation *pending_operation(const Execution &execution) {
    if (!execution.pending_effect || !execution.active ||
        execution.pc >= execution.active->code.size())
        return nullptr;
    return &execution.active->code[execution.pc];
}
bool acknowledge_effect(Execution &execution, std::uint32_t instruction_token) {
    if (!pending_operation(execution) || execution.finished ||
        instruction_token != execution.result.executed)
        return false;
    ++execution.pc;
    execution.pending_effect = false;
    return true;
}
Result advance(Execution &execution, Workspace &workspace, random::Rng *rng,
               std::uint32_t instruction_limit, Effects effects) {
    auto &result = execution.result;
    if (execution.finished || execution.pending_effect)
        return result;
    auto &pc = execution.pc;
    auto &active = execution.active;
    auto &depth = execution.depth;
    auto &local_time = execution.local_time;
    auto &wait = execution.wait;
    const auto mask = execution.mask;
    const auto *module = execution.module;
    auto yield = [&]() {
        execution.pending_effect = true;
        result.status = Status::external_effect;
        return result;
    };
    random::State instruction_rng{};
    bool rng_checkpoint_valid = false;
    unsigned rng_expressions = 0;
    try {
        require(active != nullptr && mask != 0 && instruction_limit > 0 &&
                result.tick < std::numeric_limits<std::uint32_t>::max());
        auto selector = [](double value) {
            require(std::isfinite(value) && value >= 10000 && value < 10101, Status::unsupported);
            return std::size_t(value - 10000);
        };
        auto random_expression = [&]() -> random::Rng & {
            require(rng != nullptr, Status::missing_context);
            // Source operators/function arguments do not generally sequence
            // multiple RNG consumers. Do not impose an unverified draw order.
            require(++rng_expressions == 1, Status::unsupported);
            return *rng;
        };
        auto operand = [&](const Operation &op, unsigned word, bool floating,
                           unsigned flag) -> double {
            require((word + 1) * 4 <= op.payload_size);
            double value =
                floating ? double(as_float(op.words[word])) : double(as_int(op.words[word]));
            require(std::isfinite(value));
            if ((op.flags & (1U << flag)) && value >= 10000 && value < 10101) {
                const auto slot = selector(value);
                const bool resolved =
                    floating ? slot < 100 && slot != 98 : !(slot >= 79 && slot <= 82);
                if (resolved) {
                    if (slot >= 32 && slot <= 35) {
                        auto &stream = random_expression();
                        switch (slot) {
                        case 32:
                            value = stream.next_u32() & 0x7fffffffU;
                            break;
                        case 33:
                            value = stream.unit();
                            break;
                        case 34:
                            value = as_int(stream.next_u32());
                            break;
                        case 35:
                            value = stream.signed_unit();
                            break;
                        }
                    } else if (floating && slot == 82) {
                        value = random_expression().range_float(6.2831855f) - 3.1415927f;
                    } else {
                        require(workspace.initialized[slot], Status::missing_context);
                        value = workspace.registers[slot];
                    }
                }
            }
            require(std::isfinite(value));
            return floating ? double(float(value)) : double(checked_integer(value));
        };
        auto write = [&](const Operation &op, unsigned word, bool floating, double value) {
            require((word + 1) * 4 <= op.payload_size && (op.flags & (1U << word)),
                    Status::unsupported);
            const double key =
                floating ? double(as_float(op.words[word])) : double(as_int(op.words[word]));
            const auto slot = selector(key);
            // Unmapped lvalues mutate instruction bytes in retail. The immutable
            // predecoded scheduler deliberately does not approximate that behavior.
            require(writable_scalar(slot, floating), Status::unsupported);
            if (floating)
                value = float(value);
            else
                require(value >= std::numeric_limits<std::int32_t>::min() &&
                        value <= std::numeric_limits<std::int32_t>::max() &&
                        std::floor(value) == value);
            require(std::isfinite(value));
            workspace.registers[slot] = value;
            workspace.initialized[slot] = true;
        };
        if (wait > 0) {
            --wait;
            ++result.tick;
            result.status = Status::frame_complete;
            return result;
        }
        while (pc < active->code.size() && active->code[pc].time == local_time) {
            const auto &op = active->code[pc];
            result.offset = op.offset;
            result.opcode = op.opcode;
            if ((op.mask & mask) != mask) {
                ++pc;
                continue;
            }
            require(result.executed < instruction_limit, Status::instruction_limit);
            ++result.executed;
            rng_expressions = 0;
            if (rng) {
                instruction_rng = rng->state();
                rng_checkpoint_valid = true;
            }
            auto next = pc + 1;
            switch (op.opcode) {
            case 0:
            case 3:
                break;
            case 1:
                execution.finished = true;
                result.terminated = true;
                result.status = Status::returned;
                return result;
            case 52: {
                require(module != nullptr, Status::missing_context);
                require(op.payload_size == 4);
                const auto sub = as_int(op.words[0]);
                // Negative/truncated IDs and retail depth saturation need a
                // separate lifetime contract; do not treat them as normal calls.
                require(sub >= 0 && sub <= 32767 && std::size_t(sub) < module->subs.size());
                require(depth < workspace.calls.size(), Status::unsupported);
                workspace.calls[depth++] = {
                    active, next, local_time, wait, workspace.registers, workspace.initialized};
                active = &module->subs[std::size_t(sub)];
                next = 0;
                local_time = wait = 0;
                for (std::size_t i = 0; i < 8; ++i) {
                    workspace.registers[53 + i] = workspace.registers[61 + i];
                    workspace.initialized[53 + i] = workspace.initialized[61 + i];
                }
                break;
            }
            case 53: {
                if (depth == 0) {
                    execution.finished = true;
                    result.status = Status::returned;
                    return result;
                }
                const auto &frame = workspace.calls[--depth];
                active = frame.program;
                next = frame.pc;
                local_time = frame.time;
                wait = frame.wait;
                // Context storage rolls back, entity and shared scalar storage
                // survives. This distinction matters even for a one-tick call.
                for (std::size_t slot = 0; slot < workspace.registers.size(); ++slot)
                    if (slot < 8 || (slot >= 16 && slot <= 23) || (slot >= 36 && slot <= 39) ||
                        (slot >= 53 && slot <= 60) || (slot >= 94 && slot <= 95)) {
                        workspace.registers[slot] = frame.registers[slot];
                        workspace.initialized[slot] = frame.initialized[slot];
                    }
                break;
            }
            case 2:
                wait = std::int64_t(operand(op, 0, false, 0));
                require(wait >= 0 && wait <= 4096);
                break;
            case 4:
            case 5: {
                bool take = true;
                if (op.opcode == 5) {
                    const auto value = operand(op, 2, false, 2) - 1;
                    write(op, 2, false, value);
                    take = value > 0;
                }
                if (take) {
                    local_time = as_int(op.words[0]);
                    next = op.target;
                }
                break;
            }
            case 6:
            case 7: {
                const bool floating = op.opcode == 7;
                write(op, 0, floating, operand(op, 1, floating, 1));
                break;
            }
            case 8:
            case 9: {
                const bool floating = op.opcode == 9;
                const int sign = random_expression().next_u16() & 1U ? 1 : -1;
                const auto value = operand(op, 1, floating, 1);
                write(op, 0, floating, sign * value);
                break;
            }
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29: {
                const bool floating = (op.opcode >= 15 && op.opcode <= 19) || op.opcode >= 25;
                const unsigned first = op.opcode < 20 ? 0 : 1;
                const auto left = operand(op, first, floating, first);
                const auto right = operand(op, first + 1, floating, first + 1);
                write(op, 0, floating, arithmetic((op.opcode - 10) % 5, floating, left, right));
                break;
            }
            case 30:
            case 31:
                write(op, 0, false, arithmetic(op.opcode - 30, false, operand(op, 0, false, 0), 1));
                break;
            case 32:
            case 33: {
                const float value = float(operand(op, 1, true, 1));
                write(op, 0, true, op.opcode == 32 ? std::sin(value) : std::cos(value));
                break;
            }
            case 34:
            case 39: {
                const float x1 = float(operand(op, 1, true, 1));
                const float y1 = float(operand(op, 2, true, 2));
                const float x2 = float(operand(op, 3, true, 3));
                const float y2 = float(operand(op, 4, true, 4));
                const float x = x1 - x2, y = y1 - y2;
                write(op, 0, true,
                      op.opcode == 34 ? kinematics::point_angle(y2 - y1, x2 - x1)
                                      : std::sqrt(x * x + y * y));
                break;
            }
            case 37:
                write(op, 0, true, kinematics::normalize_angle(float(operand(op, 0, true, 0))));
                break;
            case 38: {
                const float angle = kinematics::normalize_angle(float(operand(op, 2, true, 2)));
                const float magnitude = float(operand(op, 3, true, 3));
                write(op, 0, true, std::cos(angle) * magnitude);
                write(op, 1, true, std::sin(angle) * magnitude);
                break;
            }
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51: {
                const bool floating = (op.opcode & 1) != 0;
                const auto left = operand(op, 0, floating, 0);
                const auto right = operand(op, 1, floating, 1);
                const bool comparisons[] = {left == right, left != right,
                                            left<right, left <= right, left> right, left >= right};
                if (comparisons[(op.opcode - 40) / 2]) {
                    require(op.payload_size == 16 && op.target < active->code.size());
                    local_time = as_int(op.words[2]);
                    next = op.target;
                }
                break;
            }
            case 111: {
                if (effects == Effects::yield_to_world)
                    return yield();
                require(op.payload_size == 28);
                TransformWrite transform{result.tick, {}};
                for (unsigned word = 0; word < 7; ++word) {
                    const double value = operand(op, word, word >= 5, word);
                    transform.words[word] =
                        word >= 5 ? as_bits(float(value)) : std::uint32_t(std::int32_t(value));
                }
                workspace.transforms.push_back(transform);
                break;
            }
            case 96:
            case 97:
            case 98:
            case 99:
            case 100:
            case 101:
            case 102:
            case 103:
            case 104: {
                if (effects == Effects::yield_to_world)
                    return yield();
                // The emitter records requests, not allocation, suppression,
                // random spread or external callbacks. An RNG-enabled run
                // cannot cross this boundary and preserve its shared stream.
                require(rng == nullptr, Status::missing_context);
                require(op.payload_size == 32);
                auto words = op.words;
                // Packed type/color are 16-bit operands 0/1; count1/count2
                // are words 1/2 but operands 2/3. Flags index operands.
                require((op.flags & 3) == 0, Status::unsupported);
                for (unsigned word = 1; word < 7; ++word) {
                    const double value = operand(op, word, word >= 3, word + 1);
                    if (word < 3)
                        require(value >= 0 && value <= 1536);
                    words[word] = word >= 3 ? as_bits(float(value)) : std::uint32_t(value);
                }
                result.requested_bullets += std::uint64_t(words[1]) * words[2];
                result.digest = (result.digest ^ result.tick) * 1099511628211ULL;
                for (auto word : words)
                    result.digest = (result.digest ^ word) * 1099511628211ULL;
                workspace.emissions.push_back(
                    {result.tick, op.offset, op.opcode, words, workspace.transforms.size()});
                break;
            }
            default:
                if (effects == Effects::yield_to_world && op.opcode >= 0 && op.opcode <= 184)
                    return yield();
                throw Blocked{Status::unsupported};
            }
            pc = next;
            rng_checkpoint_valid = false;
            // Secondary wait begins within this same scheduling tick.
            if (wait > 0) {
                --wait;
                --local_time;
                break;
            }
        }
        require(pc < active->code.size(), Status::invalid);
        ++result.tick;
        ++local_time;
        result.status = Status::frame_complete;
    } catch (const Blocked &blocked) {
        if (rng_checkpoint_valid)
            *rng = random::Rng(instruction_rng);
        result.status = blocked.status;
        execution.finished = true;
    }
    return result;
}
static Result bounded(Execution execution, Workspace &workspace, std::uint32_t horizon,
                      std::uint32_t instruction_limit, random::Rng *rng) {
    if (horizon == 0 || horizon > 4096 || instruction_limit == 0) {
        execution.result.status = Status::invalid;
        return execution.result;
    }
    while (execution.result.tick < horizon && !execution.finished)
        advance(execution, workspace, rng, instruction_limit);
    auto result = execution.result;
    if (!execution.finished)
        result.status = Status::horizon;
    return result;
}
Result run(const Program &program, Workspace &workspace, std::uint8_t mask, std::uint32_t horizon,
           std::uint32_t instruction_limit, random::Rng *rng) {
    return bounded(begin(program, workspace, mask), workspace, horizon, instruction_limit, rng);
}
Result run(const Module &module, std::size_t sub, Workspace &workspace, std::uint8_t mask,
           std::uint32_t horizon, std::uint32_t instruction_limit, random::Rng *rng) {
    return bounded(begin(module, sub, workspace, mask), workspace, horizon, instruction_limit, rng);
}
} // namespace th08::emitter
