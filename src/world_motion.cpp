#include <th08/world_motion.hpp>

namespace th08::world {
namespace {
namespace vm = emitter;
struct Blocked {
    EffectStatus status;
};
void require(bool condition, EffectStatus status = EffectStatus::invalid) {
    if (!condition)
        throw Blocked{status};
}
float angle_to(enemy::Vec3 player, enemy::Vec3 point) {
    const float x = player.x - point.x, y = player.y - point.y;
    return x == 0 && y == 0 ? kinematics::pi / 2 : kinematics::point_angle(y, x);
}
void publish_fields(const enemy::State &motion, vm::ScalarStorage &storage) {
    auto set = [&](unsigned slot, float value) {
        storage.registers[slot] = value;
        storage.initialized[slot] = true;
    };
    auto vector = [&](unsigned first, enemy::Vec3 value) {
        set(first, value.x);
        set(first + 1, value.y);
        set(first + 2, value.z);
    };
    vector(42, motion.world_position);
    set(69, motion.angle);
    set(70, motion.angular_velocity);
    set(71, motion.speed);
    set(72, motion.acceleration);
    set(73, motion.orbit_radius);
    vector(74, motion.interpolation_origin);
    set(77, motion.orbit_angle);
    set(78, motion.orbit_angular_velocity);
    vector(79, motion.interpolation_delta);
    vector(85, motion.last_displacement);
}
EffectStatus decoded_status(vm::Status status) {
    if (status == vm::Status::missing_context)
        return EffectStatus::missing_context;
    if (status == vm::Status::unsupported)
        return EffectStatus::unsupported;
    return EffectStatus::invalid;
}
} // namespace
EffectStatus publish_motion(const enemy::State &motion, vm::ScalarStorage &storage,
                            const enemy::Vec3 *player) {
    auto checked = motion;
    if (enemy::refresh_world(checked) != enemy::Status::advanced ||
        (player &&
         !(std::isfinite(player->x) && std::isfinite(player->y) && std::isfinite(player->z))))
        return EffectStatus::invalid;
    publish_fields(motion, storage);
    for (unsigned slot : {45, 46, 47, 48, 50})
        storage.initialized[slot] = player != nullptr;
    if (player) {
        storage.registers[45] = player->x;
        storage.registers[46] = player->y;
        storage.registers[47] = player->z;
        storage.registers[48] = angle_to(*player, motion.world_position);
        const float x = player->x - motion.world_position.x;
        const float y = player->y - motion.world_position.y;
        const float z = player->z - motion.world_position.z;
        storage.registers[50] = std::sqrt(x * x + y * y + z * z);
    }
    return EffectStatus::applied;
}
EffectStatus apply_motion_effect(vm::Execution &execution, vm::Workspace &workspace,
                                 enemy::State &motion, random::Rng *rng,
                                 const enemy::Vec3 *player) {
    const auto *op = vm::pending_operation(execution);
    if (!op || execution.finished)
        return EffectStatus::invalid;
    if (op->opcode < 63 || op->opcode > 76 || op->opcode == 67)
        return EffectStatus::not_handled;
    auto next = motion;
    vm::ScalarStorage storage = workspace;
    auto stream = rng ? *rng : random::Rng(random::State{});
    try {
        require(enemy::refresh_world(next) == enemy::Status::advanced);
        require(publish_motion(next, storage, player) == EffectStatus::applied);
        auto read = [&](unsigned word, bool floating) {
            publish_fields(next, storage);
            const vm::OperandField field{
                std::uint16_t(word * 4),
                floating ? vm::OperandType::float32 : vm::OperandType::signed32, std::int8_t(word)};
            double value = 0;
            const auto status =
                vm::decode_operands(*op, storage, &field, &value, 1, rng ? &stream : nullptr);
            if (status != vm::Status::operands_decoded)
                throw Blocked{decoded_status(status)};
            return value;
        };
        auto integer = [&](unsigned word) { return std::int32_t(read(word, false)); };
        auto floating = [&](unsigned word) { return float(read(word, true)); };
        auto timer = [&](std::int32_t duration) { next.timer.set(next.duration = duration); };
        auto aimed = [&]() {
            require(player != nullptr, EffectStatus::missing_context);
            return angle_to(*player, next.position); // Source uses LOCAL position here.
        };
        auto finite_polar = [&]() {
            const float angle = kinematics::normalize_angle(floating(2));
            auto component = [&](float direction) {
                // Speed and duration occur in one unsequenced C++ product. One
                // random expression is supported; two require executable evidence.
                publish_fields(next, storage);
                const vm::OperandField fields[] = {{12, vm::OperandType::float32, 3},
                                                   {0, vm::OperandType::signed32, 0}};
                double values[2];
                const auto status =
                    vm::decode_operands(*op, storage, fields, values, 2, rng ? &stream : nullptr);
                if (status != vm::Status::operands_decoded)
                    throw Blocked{decoded_status(status)};
                return direction * float(values[0]) * std::int32_t(values[1]);
            };
            next.interpolation_delta.x = component(std::cos(angle));
            next.interpolation_delta.y = component(std::sin(angle));
            next.interpolation_delta.z = 0;
            next.interpolation_origin = next.world_position;
            timer(integer(0)); // The source resolves duration a third time here.
            next.easing = enemy::Easing(std::uint32_t(integer(1)) & 7U);
            next.mode = enemy::Mode::interpolated;
            if (next.mirror_x)
                next.interpolation_delta.x = -next.interpolation_delta.x;
        };
        const unsigned sizes[] = {8, 16, 8, 16, 0, 8, 16, 4, 4, 28, 16, 12, 16, 0};
        require(op->payload_size == sizes[unsigned(op->opcode - 63)]);
        switch (op->opcode) {
        case 63: {
            const float x = floating(0), y = floating(1);
            require(enemy::set_position(next, x, y) == enemy::Status::advanced);
            break;
        }
        case 64: {
            const float x = floating(2), y = floating(3);
            next.interpolation_delta = {x - next.world_position.x, y - next.world_position.y,
                                        0.0f - next.world_position.z};
            next.interpolation_origin = next.position;
            timer(integer(0));
            next.easing = enemy::Easing(std::uint32_t(integer(1)) & 7U);
            next.mode = enemy::Mode::interpolated;
            next.velocity = {};
            if (next.mirror_x)
                next.interpolation_delta.x = -next.interpolation_delta.x;
            break;
        }
        case 65:
            next.angle = kinematics::normalize_angle(floating(0));
            next.speed = floating(1);
            next.mode = enemy::Mode::polar;
            timer(0);
            break;
        case 66:
        case 69:
            if (integer(0) <= 0) {
                const float direction = floating(2);
                next.angle = kinematics::normalize_angle(direction, op->opcode == 69 ? aimed() : 0);
                next.speed = floating(3);
                next.mode = enemy::Mode::polar;
                timer(op->opcode == 69 ? integer(0) : 0);
            } else {
                finite_polar();
            }
            break;
        case 68: {
            const float direction = floating(0);
            next.angle = kinematics::normalize_angle(direction, aimed());
            next.speed = floating(1);
            break; // Unlike opcode 65, this preserves mode, duration and timer.
        }
        case 70:
            next.angular_velocity = floating(0);
            next.mode = enemy::Mode::polar;
            break;
        case 71:
            next.acceleration = floating(0);
            next.mode = enemy::Mode::polar;
            break;
        case 72:
            timer(integer(0));
            next.interpolation_origin.x = floating(1);
            next.interpolation_origin.y = floating(2);
            next.orbit_angle = floating(3);
            next.orbit_angular_velocity = floating(4);
            next.orbit_radius = floating(5);
            next.radial_velocity = floating(6);
            next.mode = enemy::Mode::orbit;
            break;
        case 73:
            timer(integer(0));
            next.interpolation_origin = next.position;
            next.orbit_angle = floating(1);
            next.orbit_angular_velocity = floating(2);
            next.orbit_radius = 0;
            next.radial_velocity = floating(3);
            next.mode = enemy::Mode::orbit;
            break;
        case 74:
            timer(integer(0));
            next.orbit_angular_velocity = floating(1);
            next.radial_velocity = floating(2);
            next.mode = enemy::Mode::orbit;
            break;
        case 75:
            next.bounds.lower.x = floating(0);
            next.bounds.lower.y = floating(1);
            next.bounds.upper.x = floating(2);
            next.bounds.upper.y = floating(3);
            next.clamp = true;
            break;
        case 76:
            next.clamp = false;
            break;
        }
        require(next.mode != enemy::Mode::interpolated || next.duration != 0);
        require(enemy::refresh_world(next) == enemy::Status::advanced);
        require(publish_motion(next, storage, player) == EffectStatus::applied);
        require(vm::acknowledge_effect(execution, execution.result.executed));
        motion = next;
        static_cast<vm::ScalarStorage &>(workspace) = storage;
        if (rng)
            *rng = stream;
        return EffectStatus::applied;
    } catch (const Blocked &blocked) {
        return blocked.status;
    }
}
} // namespace th08::world
