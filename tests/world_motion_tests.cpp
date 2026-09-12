#include <cstdlib>
#include <cstring>
#include <iostream>
#include <th08/world_motion.hpp>
namespace vm = th08::emitter;
namespace world = th08::world;
namespace enemy = th08::enemy;
void check(bool ok, const char *why) {
    if (!ok) {
        std::cerr << why << '\n';
        std::exit(1);
    }
}
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
vm::Operation op(int opcode, std::initializer_list<std::uint32_t> words, unsigned flags = 0) {
    vm::Operation result{
        0, std::int16_t(opcode), std::uint16_t(flags), std::uint16_t(words.size() * 4), 255, 0, 0,
        {}};
    std::copy(words.begin(), words.end(), result.words.begin());
    return result;
}
void decoder_tests() {
    vm::ScalarStorage storage;
    storage.registers[0] = 17;
    storage.initialized[0] = true;
    auto operation = op(97, {10000U | (65534U << 16), bits(1.25f)}, 1);
    const vm::OperandField fields[] = {{0, vm::OperandType::signed16, 0},
                                       {2, vm::OperandType::signed16, 1},
                                       {4, vm::OperandType::float32, 2}};
    double values[3] = {-99, -99, -99};
    check(vm::decode_operands(operation, storage, fields, values, 3) ==
                  vm::Status::operands_decoded &&
              values[0] == 17 && values[1] == -2 && values[2] == 1.25,
          "packed operand widths or per-argument flag positions changed");
    operation = op(72, {10032, 10032}, 3);
    const vm::OperandField random_fields[] = {{0, vm::OperandType::signed32, 0},
                                              {4, vm::OperandType::signed32, 1}};
    th08::random::Rng rng(123), expected(123);
    values[0] = values[1] = -99;
    check(vm::decode_operands(operation, storage, random_fields, values, 2, &rng) ==
                  vm::Status::unsupported &&
              values[0] == -99 && values[1] == -99 && rng.generation_count() == 0,
          "unsequenced random operands leaked output or RNG");
    check(vm::decode_operands(operation, storage, random_fields, values, 2, &rng,
                              vm::OperandOrder::source_ordered_fields) ==
                  vm::Status::operands_decoded &&
              values[0] == (expected.next_u32() & 0x7fffffffU) &&
              values[1] == (expected.next_u32() & 0x7fffffffU),
          "explicitly source-ordered fields did not share the RNG stream");
    operation = op(122, {});
    operation.payload_size = 40;
    th08::resources::Bytes bytes(40, 0);
    bytes[36] = 29;
    const vm::OperandField cold{36, vm::OperandType::signed32, -1};
    check(vm::decode_operands(operation, storage, &cold, values, 1) == vm::Status::invalid,
          "missing long world payload became zero");
    check(vm::decode_operands(operation, storage, &cold, values, 1, nullptr,
                              vm::OperandOrder::single_random_expression,
                              th08::resources::view(bytes)) == vm::Status::operands_decoded &&
              values[0] == 29,
          "cold world payload was truncated to hot operand words");
}
int main() {
    decoder_tests();
    vm::Program program;
    vm::Workspace storage;
    enemy::State motion;
    motion.position = {20, 30, 0};
    motion.position_offset = {10, 5, 0};
    enemy::Vec3 player{20, 30, 0};
    vm::Execution execution;
    auto pending = [&](vm::Operation instruction) {
        program.code = {instruction, op(53, {})};
        execution = vm::begin(program, storage, 8);
        check(vm::advance(execution, storage, nullptr, 1000, vm::Effects::yield_to_world).status ==
                  vm::Status::external_effect,
              "movement instruction did not yield to the world");
    };
    pending(op(68, {bits(0), bits(2)}));
    motion.mode = enemy::Mode::orbit;
    motion.timer.set(7);
    motion.duration = 7;
    check(world::apply_motion_effect(execution, storage, motion, nullptr, &player) ==
                  world::EffectStatus::applied &&
              motion.angle == th08::kinematics::pi / 2 && motion.mode == enemy::Mode::orbit &&
              motion.timer.current == 7,
          "coincident player aim lost the source pi/2 rule or changed opcode68 mode/timer");
    for (float speed : {3.0f, -2.0f, 3.0f}) {
        pending(op(68, {bits(0), bits(speed)}));
        check(world::apply_motion_effect(execution, storage, motion, nullptr, &player) ==
                      world::EffectStatus::applied &&
                  motion.angle == th08::kinematics::pi / 2,
              "irrelevant speed changed the coincident-position aim rule");
    }
    auto world_player = motion.world_position;
    check(world::publish_motion(motion, storage, &world_player) == world::EffectStatus::applied &&
              storage.registers[48] == th08::kinematics::pi / 2 && storage.registers[50] == 0,
          "coincident world-position aim selector lost its pi/2 special case");
    pending(op(65, {bits(.75f), bits(10069.0f)}, 2));
    check(world::apply_motion_effect(execution, storage, motion) == world::EffectStatus::applied &&
              motion.speed == motion.angle && motion.speed == .75f,
          "speed operand did not observe the angle written earlier in the instruction");
    pending(op(72, {4, bits(80), bits(10074.0f), bits(0), bits(2), bits(10078.0f), bits(1)},
               (1U << 2) | (1U << 5)));
    check(world::apply_motion_effect(execution, storage, motion) == world::EffectStatus::applied &&
              motion.interpolation_origin.y == 80 && motion.orbit_radius == 2,
          "orbit fields were decoded against a stale instruction-entry snapshot");
    pending(op(64, {10074, 0, bits(60), bits(90)}, 1));
    check(world::apply_motion_effect(execution, storage, motion) == world::EffectStatus::applied &&
              motion.duration == 20 && motion.interpolation_delta.x == 30 &&
              motion.interpolation_origin.x == 20,
          "relative motion did not distinguish local origin from world displacement");
    pending(op(63, {bits(42), bits(50)}));
    check(world::apply_motion_effect(execution, storage, motion) == world::EffectStatus::applied &&
              motion.world_position.x == 52 && storage.registers[42] == 52,
          "world coordinates were not published before ECL resumption");
    check(vm::advance(execution, storage).status == vm::Status::returned,
          "movement acknowledgement did not resume the same-frame script");
    pending(op(68, {bits(10033.0f), bits(1)}, 1));
    th08::random::Rng rng(4321);
    const auto before = motion;
    check(world::apply_motion_effect(execution, storage, motion, &rng) ==
                  world::EffectStatus::missing_context &&
              rng.generation_count() == 0 && execution.pending_effect &&
              motion.angle == before.angle,
          "missing target leaked RNG, movement or effect acknowledgement");
    pending(op(66, {3, 0, bits(0), bits(10033.0f)}, 1U << 3));
    th08::random::Rng expected(4321);
    const float first = expected.unit(), second = expected.unit();
    check(world::apply_motion_effect(execution, storage, motion, &rng) ==
                  world::EffectStatus::applied &&
              motion.interpolation_delta.x == first * 3 &&
              motion.interpolation_delta.y == 0 * second * 3 && rng.generation_count() == 4 &&
              rng.seed() == expected.seed(),
          "finite polar motion cached the source's repeated random speed reads");
    pending(op(67, {3, 0, bits(0), bits(1)}));
    check(world::apply_motion_effect(execution, storage, motion, &rng) ==
                  world::EffectStatus::not_handled &&
              execution.pending_effect,
          "unimplemented random movement was acknowledged as a NOP");
    std::cout << "ECL movement effects, typed operands and shared-world transactions: passed\n";
}
