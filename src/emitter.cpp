#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <th08/emitter.hpp>

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
    }
    return "INVALID";
}
Program::Program(resources::View resource, const resources::Ecl &ecl, std::size_t sub) {
    const auto &range = ecl.subs.at(sub);
    code.reserve(range.count);
    for (std::uint32_t i = range.first; i < range.first + range.count; ++i) {
        const auto &instruction = ecl.instructions[i];
        Operation op{instruction.time,
                     instruction.opcode,
                     instruction.flags,
                     std::uint16_t(instruction.size - 12),
                     instruction.mask,
                     instruction.offset,
                     0,
                     {}};
        for (std::size_t word = 0;
             word < std::min(std::size_t(op.payload_size / 4), op.words.size()); ++word)
            op.words[word] = resources::u32(resource, instruction.offset + 12 + word * 4);
        code.push_back(op);
    }
    for (auto &op : code)
        if (op.opcode == 4 || op.opcode == 5) {
            const auto offset = std::int64_t(op.offset) + as_int(op.words[1]);
            const auto found = std::lower_bound(code.begin(), code.end(), offset,
                                                [](const Operation &candidate, std::int64_t value) {
                                                    return candidate.offset < value;
                                                });
            if (found == code.end() || found->offset != offset)
                throw std::runtime_error("invalid compiled jump");
            op.target = std::uint32_t(found - code.begin());
        }
}
Result run(const Program &program, Workspace &workspace, std::uint8_t mask, std::uint32_t horizon,
           std::uint32_t instruction_limit) {
    Result result;
    workspace.initialized.fill(false);
    workspace.emissions.clear();
    workspace.transforms.clear();
    std::uint32_t pc = 0;
    std::int64_t local_time = 0, wait = 0;
    try {
        require(horizon > 0 && horizon <= 4096 && mask != 0 && instruction_limit > 0);
        auto selector = [](double value) {
            require(std::isfinite(value) && value >= 10000 && value <= 10100 &&
                        std::floor(value) == value,
                    Status::unsupported);
            return std::size_t(value - 10000);
        };
        auto operand = [&](const Operation &op, unsigned word, bool floating,
                           unsigned flag) -> double {
            require((word + 1) * 4 <= op.payload_size);
            double value =
                floating ? double(as_float(op.words[word])) : double(as_int(op.words[word]));
            require(std::isfinite(value));
            if (op.flags & (1U << flag)) {
                auto slot = selector(value);
                require(workspace.initialized[slot], Status::missing_context);
                value = workspace.registers[slot];
            }
            require(std::isfinite(value));
            if (!floating)
                require(value >= std::numeric_limits<std::int32_t>::min() &&
                        value <= std::numeric_limits<std::int32_t>::max() &&
                        std::floor(value) == value);
            return floating ? double(float(value)) : value;
        };
        auto write = [&](const Operation &op, unsigned word, bool floating, double value) {
            require((word + 1) * 4 <= op.payload_size && (op.flags & (1U << word)),
                    Status::unsupported);
            const double key =
                floating ? double(as_float(op.words[word])) : double(as_int(op.words[word]));
            const auto slot = selector(key);
            // Extra integer registers may be explicitly initialized by a slice
            // (sub40 uses EXTRA_I0). Their entry values are never invented.
            // Writes stay in this isolated context, not in an engine entity.
            require(slot < 8 || (slot >= 16 && slot < 24) || (slot >= 36 && slot < 40),
                    Status::unsupported);
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
        for (result.tick = 0; result.tick < horizon; ++result.tick, ++local_time) {
            if (wait > 0) {
                --wait;
                --local_time;
                continue;
            }
            while (pc < program.code.size() && program.code[pc].time == local_time) {
                const auto &op = program.code[pc];
                result.offset = op.offset;
                result.opcode = op.opcode;
                if ((op.mask & mask) != mask) {
                    ++pc;
                    continue;
                }
                require(result.executed < instruction_limit, Status::instruction_limit);
                ++result.executed;
                auto next = pc + 1;
                switch (op.opcode) {
                case 0:
                case 3:
                    break;
                case 1:
                case 53:
                    result.status = Status::returned;
                    return result;
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
                case 15:
                case 16:
                case 17: {
                    const float left = float(operand(op, 0, true, 0)),
                                right = float(operand(op, 1, true, 1));
                    const float value = op.opcode == 15   ? left + right
                                        : op.opcode == 16 ? left - right
                                                          : left * right;
                    write(op, 0, true, value);
                    break;
                }
                case 111: {
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
                    throw Blocked{Status::unsupported};
                }
                pc = next;
                // Secondary wait begins within this same scheduling tick.
                if (wait > 0) {
                    --wait;
                    --local_time;
                    break;
                }
            }
            require(pc < program.code.size(), Status::invalid);
        }
    } catch (const Blocked &blocked) {
        result.status = blocked.status;
    }
    return result;
}
} // namespace th08::emitter
