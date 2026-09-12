#pragma once
#include <th08/world_motion.hpp>

// Included after world_motion_reference; no original game executable is run.
struct WorldMotionComparison {
    std::uint64_t effects = 0, atomic_failures = 0, mismatches = 0;
};
namespace world_motion_test {
namespace actual = th08::enemy;
namespace reference = world_motion_reference;
namespace vm = th08::emitter;
inline bool same(float a, float b) {
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}
inline bool same(actual::Vec3 a, Float3 b) {
    return same(a.x, b.x) && same(a.y, b.y) && same(a.z, b.z);
}
inline Float3 vector(actual::Vec3 value) {
    return {value.x, value.y, value.z};
}
inline unsigned flags(const actual::State &state) {
    return 1U | (unsigned(state.mode) << 12) | (unsigned(state.easing) << 14) |
           (unsigned(state.mirror_x) << 18) | (unsigned(state.clamp) << 19) |
           (unsigned(state.skip_integration) << 29) |
           (unsigned(state.inherit_parent_position) << 9);
}
inline reference::Enemy source_state(const actual::State &state) {
    reference::Enemy result;
    result.position = vector(state.position);
    result.positionOffset = vector(state.position_offset);
    result.worldPosition = vector(state.world_position);
    result.velocity = vector(state.velocity);
    result.previousPosition = vector(state.previous_position);
    result.lastFrameDisplacement = vector(state.last_displacement);
    result.movementInterpolationOrigin = vector(state.interpolation_origin);
    result.movementInterpolationDelta = vector(state.interpolation_delta);
    result.movementAngle = state.angle;
    result.angularVelocity = state.angular_velocity;
    result.speed = state.speed;
    result.acceleration = state.acceleration;
    result.orbitAngle = state.orbit_angle;
    result.orbitAngularVelocity = state.orbit_angular_velocity;
    result.orbitRadius = state.orbit_radius;
    result.radialVelocity = state.radial_velocity;
    result.movementTimer.previous = state.timer.previous;
    result.movementTimer.subFrame = state.timer.fraction;
    result.movementTimer.current = state.timer.current;
    result.movementDuration = state.duration;
    result.flags1 = flags(state);
    result.movementBounds.lower = vector(state.bounds.lower);
    result.movementBounds.upper = vector(state.bounds.upper);
    return result;
}
inline bool same(const actual::State &state, const reference::Enemy &source) {
    return same(state.position, source.position) &&
           same(state.position_offset, source.positionOffset) &&
           same(state.world_position, source.worldPosition) &&
           same(state.velocity, source.velocity) &&
           same(state.previous_position, source.previousPosition) &&
           same(state.last_displacement, source.lastFrameDisplacement) &&
           same(state.interpolation_origin, source.movementInterpolationOrigin) &&
           same(state.interpolation_delta, source.movementInterpolationDelta) &&
           same(state.angle, source.movementAngle) &&
           same(state.angular_velocity, source.angularVelocity) &&
           same(state.speed, source.speed) && same(state.acceleration, source.acceleration) &&
           same(state.orbit_angle, source.orbitAngle) &&
           same(state.orbit_angular_velocity, source.orbitAngularVelocity) &&
           same(state.orbit_radius, source.orbitRadius) &&
           same(state.radial_velocity, source.radialVelocity) &&
           state.timer.previous == source.movementTimer.previous &&
           same(state.timer.fraction, source.movementTimer.subFrame) &&
           state.timer.current == source.movementTimer.current &&
           state.duration == source.movementDuration && flags(state) == source.flags1 &&
           same(state.bounds.lower, source.movementBounds.lower) &&
           same(state.bounds.upper, source.movementBounds.upper);
}
inline unsigned generation_count() {
    // The hash-pinned source class declares two u16 fields followed by u32.
    // Read object representation without changing source access or RNG methods.
    static_assert(sizeof(Rng) == 8, "unexpected native source RNG layout");
    unsigned count;
    std::memcpy(&count, reinterpret_cast<const unsigned char *>(&reference::g_Rng) + 4, 4);
    return count;
}
inline std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, 4);
    return result;
}
inline bool integer_field(unsigned opcode, unsigned word) {
    return ((opcode == 64 || opcode == 66 || opcode == 69) && word < 2) ||
           ((opcode == 72 || opcode == 73 || opcode == 74) && word == 0);
}
inline vm::Operation operation(unsigned opcode) {
    static constexpr unsigned words[] = {2, 4, 2, 4, 0, 2, 4, 1, 1, 7, 4, 3, 4, 0};
    vm::Operation result{};
    result.opcode = std::int16_t(opcode);
    result.mask = 255;
    result.offset = 4096 + opcode * 4;
    result.payload_size = std::uint16_t(words[opcode - 63] * 4);
    for (unsigned word = 0; word < words[opcode - 63]; ++word)
        result.words[word] = integer_field(opcode, word) ? (word ? 5 : 13) : bits(.375f + word);
    return result;
}
inline actual::State initial_state() {
    actual::State state;
    state.position = {64, 96, 4};
    state.position_offset = {12, 20, -2};
    state.velocity = {2, -3, 5};
    state.previous_position = {61, 99, 4};
    state.last_displacement = {3, -3, 0};
    state.interpolation_origin = {28, 36, 7};
    state.interpolation_delta = {8, 9, 10};
    state.angle = .125f;
    state.angular_velocity = .25f;
    state.speed = .5f;
    state.acceleration = -.125f;
    state.orbit_angle = -.25f;
    state.orbit_angular_velocity = .375f;
    state.orbit_radius = 8;
    state.radial_velocity = -2;
    state.timer = {17, .5f, 18};
    state.duration = 24;
    state.mode = actual::Mode::orbit;
    state.easing = actual::Easing::out_cubic;
    state.bounds = {{8, 12, 0}, {376, 436, 0}};
    actual::refresh_world(state);
    return state;
}
} // namespace world_motion_test
inline WorldMotionComparison compare_world_motion() {
    using namespace world_motion_test;
    namespace world = th08::world;
    WorldMotionComparison comparison;
    vm::Program program;
    program.code.resize(2);
    program.code[1] = {0, 53, 0, 0, 255, 4100, 0, {}};
    vm::Workspace workspace;
    auto compare = [&](vm::Operation instruction, actual::State state, actual::Vec3 player,
                       std::uint16_t seed, const char *label) {
        ++comparison.effects;
        program.code[0] = instruction;
        auto execution = vm::begin(program, workspace, 8);
        workspace.registers[0] = 174;
        workspace.initialized[0] = true;
        th08::random::Rng rng(seed);
        auto source = source_state(state);
        reference::g_Player.position = vector(player);
        reference::g_Rng.SetSeed(seed);
        reference::g_Rng.ResetGenerationCount();
        reference::EclRawInstruction raw{};
        raw.operandFlags = instruction.flags;
        std::memcpy(raw.operands, instruction.words.data(), sizeof(raw.operands));
        reference::execute(&source, &raw, instruction.opcode);
        const auto stop =
            vm::advance(execution, workspace, &rng, 100000, vm::Effects::yield_to_world);
        const auto result = world::apply_motion_effect(execution, workspace, state, &rng, &player);
        bool equal =
            stop.status == vm::Status::external_effect && result == world::EffectStatus::applied &&
            same(state, source) && rng.seed() == reference::g_Rng.GetSeed() &&
            rng.generation_count() == generation_count() && execution.pc == 1 &&
            !execution.pending_effect && execution.result.executed == 1 &&
            execution.result.tick == 0 && workspace.registers[0] == 174 &&
            workspace.initialized[0] && workspace.emissions.empty() && workspace.transforms.empty();
        constexpr unsigned published[] = {42, 43, 44, 45, 46, 47, 48, 50, 69, 70, 71, 72,
                                          73, 74, 75, 76, 77, 78, 79, 80, 81, 85, 86, 87};
        for (unsigned slot : published)
            equal =
                equal && workspace.initialized[slot] &&
                same(float(workspace.registers[slot]), source.ResolveFloat(float(10000 + slot)));
        if (!equal) {
            if (comparison.mismatches < 6)
                std::cerr << "World movement divergence label=" << label
                          << " opcode=" << instruction.opcode << " seed=" << seed
                          << " status=" << unsigned(result) << " angle=" << state.angle << '/'
                          << source.movementAngle << " speed=" << state.speed << '/' << source.speed
                          << " duration=" << state.duration << '/' << source.movementDuration
                          << '\n';
            ++comparison.mismatches;
        }
    };
    const actual::Vec3 player{192, 400, 0};
    std::mt19937 random(20260917);
    auto uniform = [&](float lower, float upper) {
        return std::uniform_real_distribution<float>(lower, upper)(random);
    };
    for (unsigned opcode = 63; opcode <= 76; ++opcode) {
        if (opcode == 67)
            continue;
        for (unsigned sample = 0; sample < 1000; ++sample) {
            auto instruction = operation(opcode);
            for (unsigned word = 0; word < instruction.payload_size / 4; ++word)
                instruction.words[word] = integer_field(opcode, word)
                                              ? (word ? random() : std::uint32_t(1 + random() % 40))
                                              : bits(uniform(-150, 150));
            if ((opcode == 66 || opcode == 69) && sample % 3 == 0)
                instruction.words[0] = std::uint32_t(-int(random() % 5));
            auto state = initial_state();
            state.mode = actual::Mode(sample % 4);
            state.mirror_x = sample % 3 == 0;
            state.clamp = sample % 5 == 0;
            state.position = {uniform(-32, 416), uniform(-32, 480), uniform(-8, 8)};
            state.position_offset = {uniform(-64, 64), uniform(-64, 64), uniform(-8, 8)};
            compare(instruction, state, player, std::uint16_t(random()), "literal");
        }
    }
    // A coincidence is NOT atan2(0,0): the source supplies pi/2. Signed zeros
    // enter the same equality branch, both for aimed movement and selector 48.
    for (unsigned opcode : {65U, 68U, 69U})
        for (unsigned sign = 0; sign < 4; ++sign) {
            auto state = initial_state();
            state.position = {sign & 1 ? -0.0f : 0.0f, sign & 2 ? -0.0f : 0.0f, 0};
            state.position_offset = {};
            auto instruction = operation(opcode);
            if (opcode == 65) {
                instruction.flags = 1;
                instruction.words[0] = bits(10048.0f);
            }
            if (opcode == 69)
                instruction.words[0] = 0;
            compare(instruction, state, {0, 0, 0}, 71, "coincident_player");
        }
    for (unsigned sample = 0; sample < 1000; ++sample) {
        auto state = initial_state();
        state.position.x = 1 + sample % 97;
        auto instruction = operation(65);
        instruction.words[0] = bits(uniform(-16, 16));
        instruction.flags = 2;
        instruction.words[1] = bits(10069.75f);
        compare(instruction, state, player, 31, "speed_reads_new_angle");
        instruction = operation(72);
        instruction.flags = 4;
        instruction.words[1] = bits(uniform(-150, 150));
        instruction.words[2] = bits(10074.0f);
        compare(instruction, state, player, 32, "origin_y_reads_new_x");
        instruction = operation(64);
        instruction.flags = 1;
        instruction.words[0] = 10074;
        compare(instruction, state, player, 33, "duration_reads_new_origin");
        instruction.words[0] = 10079;
        compare(instruction, state, player, 34, "integer_delta_selector_is_raw");
        instruction = operation(66);
        instruction.flags = 9;
        instruction.words[0] = 10074;
        instruction.words[3] = bits(10079.0f);
        compare(instruction, state, player, 35, "component_y_reads_new_delta_x");
        instruction = operation(69);
        instruction.flags = 1;
        instruction.words[0] = 10071;
        state.speed = -1;
        instruction.words[3] = bits(float(sample + 1));
        compare(instruction, state, player, 36, "aimed_duration_rereads_new_speed");
    }
    // The helper repeats the random speed call in separate x/y statements.
    // Two random factors within one product are intentionally never compared.
    for (unsigned seed = 0; seed < 65536; ++seed) {
        auto instruction = operation(66);
        instruction.flags = 8;
        instruction.words[3] = bits(10033.0f);
        compare(instruction, initial_state(), player, std::uint16_t(seed), "repeated_random_speed");
        if (seed % 8 == 0) {
            instruction = operation(69);
            instruction.flags = 1;
            instruction.words[0] = 10034;
            compare(instruction, initial_state(), player, std::uint16_t(seed),
                    "random_duration_reread");
        }
    }
    // Exercise every field against the explicitly extracted selector domain,
    // including float-selector truncation and unmapped integer delta IDs.
    constexpr unsigned float_selectors[] = {32, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48,
                                            50, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
                                            79, 80, 81, 82, 85, 86, 87, 98, 100};
    constexpr unsigned int_selectors[] = {32, 34, 42, 43, 44, 45, 46, 48, 50, 73,
                                          74, 75, 76, 79, 80, 81, 82, 85, 86};
    for (unsigned opcode = 63; opcode <= 76; ++opcode) {
        if (opcode == 67)
            continue;
        for (unsigned word = 0; word < operation(opcode).payload_size / 4; ++word)
            for (unsigned sample = 0; sample < 16; ++sample) {
                const bool is_integer = integer_field(opcode, word);
                const auto *selectors = is_integer ? int_selectors : float_selectors;
                const auto count =
                    is_integer ? std::size(int_selectors) : std::size(float_selectors);
                for (unsigned i = 0; i < count; ++i) {
                    auto instruction = operation(opcode);
                    instruction.flags = std::uint16_t(1U << word);
                    instruction.words[word] = is_integer ? 10000 + selectors[i]
                                                         : bits(float(10000 + selectors[i]) + .75f);
                    auto state = initial_state();
                    state.mirror_x = sample % 2 == 0;
                    state.clamp = sample % 3 == 0;
                    compare(instruction, state, player, std::uint16_t(random()),
                            "masked_field_matrix");
                }
            }
    }
    // Failure atomicity is a native contract, not a claimed source rollback.
    auto failure = [&](vm::Operation instruction, world::EffectStatus expected, bool has_player,
                       bool has_rng, const char *label) {
        ++comparison.atomic_failures;
        program.code[0] = instruction;
        auto execution = vm::begin(program, workspace, 8);
        auto state = initial_state();
        th08::random::Rng rng(41);
        vm::advance(execution, workspace, &rng, 100000, vm::Effects::yield_to_world);
        const auto before_execution = execution;
        const auto before_storage = static_cast<const vm::ScalarStorage &>(workspace);
        const auto before_state = source_state(state);
        const auto status = world::apply_motion_effect(
            execution, workspace, state, has_rng ? &rng : nullptr, has_player ? &player : nullptr);
        const bool equal = status == expected && same(state, before_state) && rng.seed() == 41 &&
                           rng.generation_count() == 0 && execution.pc == before_execution.pc &&
                           execution.pending_effect == before_execution.pending_effect &&
                           execution.result.executed == before_execution.result.executed &&
                           execution.result.tick == before_execution.result.tick &&
                           workspace.registers == before_storage.registers &&
                           workspace.initialized == before_storage.initialized;
        if (!equal) {
            if (comparison.mismatches < 6)
                std::cerr << "World movement rollback divergence label=" << label << '\n';
            ++comparison.mismatches;
        }
    };
    auto instruction = operation(68);
    instruction.flags = 1;
    instruction.words[0] = bits(10033.0f);
    failure(instruction, world::EffectStatus::missing_context, false, true,
            "missing_player_after_draw");
    failure(instruction, world::EffectStatus::missing_context, true, false, "missing_rng");
    instruction = operation(66);
    instruction.flags = 9;
    instruction.words[0] = 10032;
    instruction.words[3] = bits(10033.0f);
    failure(instruction, world::EffectStatus::unsupported, true, true,
            "ambiguous_two_random_factors");
    instruction = operation(72);
    instruction.flags = 10;
    instruction.words[1] = bits(10033.0f);
    instruction.words[3] = bits(10016.0f);
    failure(instruction, world::EffectStatus::missing_context, true, true, "missing_late_register");
    return comparison;
}
