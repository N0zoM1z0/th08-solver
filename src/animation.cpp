#include <algorithm>
#include <th08/animation.hpp>

namespace th08::animation {
Timing certify_timing(resources::View resource, const resources::Anm &anm, std::size_t script) {
    Timing result;
    if (script >= anm.scripts.size()) {
        result.status = Status::invalid;
        return result;
    }
    const auto &range = anm.scripts[script];
    if (range.first > anm.instructions.size() ||
        range.count > anm.instructions.size() - range.first) {
        result.status = Status::invalid;
        return result;
    }
    std::int32_t time = 0;
    for (std::size_t i = range.first; i < std::size_t(range.first) + range.count; ++i) {
        const auto &instruction = anm.instructions[i];
        result.stopping_offset = instruction.offset;
        result.stopping_opcode = instruction.opcode;
        ++result.inspected;
        if (instruction.offset > resource.size || resource.size - instruction.offset < 8) {
            result.status = Status::invalid;
            return result;
        }
        if (instruction.mask != 0)
            return result;
        time = std::max(time, std::int32_t(instruction.time));
        if (instruction.opcode == -1 || instruction.opcode == 1 || instruction.opcode == 2) {
            if (instruction.opcode != -1 && instruction.size != 8) {
                result.status = Status::invalid;
                return result;
            }
            result.status = Status::certified;
            result.completion_time = time;
            result.hides_on_completion = instruction.opcode != 2;
            // A template completed at time zero already has a null instruction;
            // ExecuteScript returns true on its first later call, not zero calls.
            result.calls_after_template = std::uint32_t(std::max(time, 1));
            return result;
        }
        unsigned payload = 0;
        switch (instruction.opcode) {
        case 3: // Sprite: only one initialization write is certifiable here.
            if (time != 0 || result.sprite != -1)
                return result;
            payload = 4;
            break;
        case 8:  // Alpha
        case 16: // Additive blend mode
        case 25: // VM draw type
            payload = 4;
            break;
        case 7:  // Scale
        case 14: // Scale growth
        case 15: // Alpha interpolation, linear
            payload = 8;
            break;
        case 29: // Scale interpolation, linear
        case 34: // Alpha interpolation
            payload = 12;
            break;
        case 36: // Scale interpolation
            payload = 16;
            break;
        default:
            return result;
        }
        if (instruction.size != payload + 8 || instruction.offset > resource.size ||
            instruction.size > resource.size - instruction.offset) {
            result.status = Status::invalid;
            return result;
        }
        if (instruction.opcode == 3) {
            result.sprite = resources::i32(resource, instruction.offset + 8);
            if (result.sprite < 0) {
                result.status = Status::unsupported;
                return result;
            }
        }
    }
    result.status = Status::invalid;
    return result;
}
} // namespace th08::animation
