#include <cmath>
#include <cstring>
#include <stdexcept>
#include <th08/practice_camera.hpp>

namespace th08::practice::camera {
namespace {
float as_float(std::uint32_t word) {
    float result;
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
Vec3 vector(const resources::StageInstruction &instruction) {
    return {as_float(instruction.words[0]), as_float(instruction.words[1]),
            as_float(instruction.words[2])};
}
bool finite(Vec3 value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}
bool valid(const Interpolation &value) {
    return finite(value.current) && finite(value.target) && value.duration >= 0 &&
           value.timer >= 0 &&
           (value.duration == 0 ||
            (finite(value.start) && value.timer <= value.duration && value.mode == 0));
}
void set(Interpolation &value, Vec3 target) {
    // A setter starts from the PREVIOUS TARGET, not the currently interpolated
    // value. STD time zero deliberately sets position repeatedly around opcode6.
    value.start = value.target;
    value.target = target;
    if (value.duration == 0)
        value.current = target;
}
void start(Interpolation &value, const resources::StageInstruction &instruction) {
    value.duration = std::int32_t(instruction.words[0]);
    value.timer = 0;
    value.mode = std::int32_t(instruction.words[1]);
}
bool interpolate(Interpolation &value) {
    if (value.duration == 0)
        return true;
    float fraction;
    if (value.timer < value.duration) {
        // Source postfix++ advances a ZunTimer. The required unit-rate profile
        // keeps its fractional part zero, so this is precisely one integer tick.
        ++value.timer;
        fraction = float(value.timer) / float(value.duration);
    } else {
        // Reaching duration on the increment does not clear it until the NEXT
        // update. Keep that distinction for setters executed before this tail.
        value.timer = value.duration;
        fraction = 1.0f;
        value.duration = 0;
    }
    auto linear = [fraction](float start, float end) { return (end - start) * fraction + start; };
    value.current = {linear(value.start.x, value.target.x), linear(value.start.y, value.target.y),
                     linear(value.start.z, value.target.z)};
    return finite(value.current);
}
} // namespace

Program::Program(resources::View bytes) {
    if (resources::sha256(bytes) !=
        "c3895cdfeac5e66a7c35e48077e4ee8841c3136000c0cfea2dd537c74daa58c6")
        throw std::invalid_argument("practice camera requires pinned stage1_s.std");
    instructions_ = resources::parse_std(bytes).instructions;
    constexpr std::int16_t opcodes[] = {9, 11, 0, 13, 7, 5, 1, 5, 6, 5, 8, 7, 8, 7, 4};
    if (instructions_.size() != std::size(opcodes))
        throw std::invalid_argument("practice camera script shape mismatch");
    for (std::size_t pc = 0; pc < instructions_.size(); ++pc) {
        const auto &instruction = instructions_[pc];
        const auto time = pc < 12 ? 0 : pc < 14 ? 512 : 1024;
        if (instruction.opcode != opcodes[pc] || instruction.time != time ||
            instruction.payload_size != 12 ||
            ((instruction.opcode == 6 || instruction.opcode == 8) &&
             (instruction.words[0] != (instruction.opcode == 6 ? 1024U : 300U) ||
              instruction.words[1] != 0)) ||
            (instruction.opcode == 4 && (instruction.words[0] != 7 || instruction.words[1] != 0)))
            throw std::invalid_argument("practice camera script shape mismatch");
    }
}

Result Program::identify(Status status, const State &state) const {
    const auto *instruction = state.pc < instructions_.size() ? &instructions_[state.pc] : nullptr;
    return {status, state.pc, instruction ? instruction->offset : 0,
            instruction ? instruction->opcode : std::int16_t(-1), state.script_time};
}

Result Program::advance(State &state, std::optional<bool> deathbomb_frozen,
                        float multiplier) const {
    if (!deathbomb_frozen)
        return identify(Status::missing_context, state);
    if (*deathbomb_frozen)
        return identify(Status::frozen, state);
    if (multiplier != 1.0f)
        return identify(Status::unsupported_timing, state);
    if (state.pc >= instructions_.size() || state.script_time < 0 || state.script_time > 1024 ||
        !valid(state.position) || !valid(state.look_at_offset) || !valid(state.up) ||
        !std::isfinite(state.fov))
        return identify(Status::invalid_state, state);

    auto next = state;
    // Background runs every due command before any interpolation. The full
    // resource certificate is what permits projecting past noncamera writes:
    // their fields do not feed this script's camera before its first jump.
    while (next.pc < instructions_.size() && instructions_[next.pc].time <= next.script_time) {
        const auto &instruction = instructions_[next.pc];
        switch (instruction.opcode) {
        case 0: // Stage position is not executed as a world-origin update here.
        case 1: // Fog and clear color belong to the omitted Background fields.
        case 13:
            break;
        case 4:
            // The loop shifts existing stage effects. Ignoring that world write
            // would make an apparently reusable camera trace unsafe to compose.
            return identify(Status::needs_world_effect, next);
        case 5:
            set(next.position, vector(instruction));
            break;
        case 6:
            start(next.position, instruction);
            break;
        case 7:
            set(next.look_at_offset, vector(instruction));
            break;
        case 8:
            start(next.look_at_offset, instruction);
            break;
        case 9:
            set(next.up, vector(instruction));
            break;
        case 11:
            next.fov = as_float(instruction.words[0]);
            break;
        default:
            return identify(Status::invalid_state, next);
        }
        ++next.pc;
    }
    if (!interpolate(next.position) || !interpolate(next.look_at_offset) || !interpolate(next.up))
        return identify(Status::invalid_state, next);
    const auto look = next.look_at_offset.current;
    const float length = std::sqrt(look.x * look.x + look.y * look.y + look.z * look.z);
    if (!std::isfinite(length))
        return identify(Status::invalid_state, next);
    // The pinned modern D3DX profile normalizes the OFFSET, not a world target
    // minus position, and emits zero at or below its 1e-8 length threshold.
    next.forward =
        length > 1.0e-8f ? Vec3{look.x / length, look.y / length, look.z / length} : Vec3{};
    ++next.script_time;
    state = next;
    return identify(Status::advanced, state);
}
} // namespace th08::practice::camera
