#pragma once
#include <th08/animation_control.hpp>

struct AnimationComparison {
    std::uint64_t frames = 0, mismatches = 0;
};
struct AnimationScalarComparison {
    std::uint64_t calls = 0, mismatches = 0;
};
// The reference executes unchanged ANM opcode blocks and all four original
// typed accessor bodies. Inputs exclude undefined arithmetic and bytecode writes.
inline AnimationScalarComparison compare_animation_scalars() {
    namespace ac = th08::animation::control;
    namespace res = th08::resources;
    namespace ref = anm_reference;
    AnimationScalarComparison result;
    auto bits = [](float value) {
        std::int32_t word;
        std::memcpy(&word, &value, 4);
        return word;
    };
    auto same_float = [](float a, float b) { return std::memcmp(&a, &b, 4) == 0; };
    auto same_clock = [&](const ac::Clock &a, const ZunTimer &b) {
        return a.current == b.current && a.previous == b.previous &&
               same_float(a.fraction, b.subFrame);
    };
    const float rates[] = {.125f, .5f, .99f, 1, 1.5f};
    auto scenario = [&](unsigned sample, int opcode) {
        ac::State state;
        ref::AnmVm source;
        ref::AnmLoaded loaded;
        ref::AnmManager manager;
        source.anmFile = &loaded;
        state.integers = {std::int32_t(sample % 29) - 14, -5, 7, 13};
        state.floats = {-.75f, -.25f, .25f, .75f};
        state.counters = {-7, 11};
        const auto integer_destination = sample % 6 < 4 ? 10000 + sample % 6 : 10004 + sample % 6;
        const auto float_destination = 10004 + sample % 4;
        const bool arithmetic = opcode >= 37 && opcode <= 60;
        const bool floating = arithmetic ? opcode % 2 == 0 : opcode >= 61 && opcode <= 66;
        std::vector<std::int32_t> words;
        std::uint16_t mask = 1;
        auto int_operand = [&](unsigned index) {
            if (sample % 3 == 0)
                return std::int32_t(sample % 41) - 20;
            mask |= std::uint16_t(1U << index);
            // Float selectors test truncating cross-bank reads, including zero.
            return std::int32_t(10000 + (sample + index) % 12);
        };
        auto float_operand = [&](unsigned index) {
            if (sample % 3 == 0)
                return bits(float(int(sample % 61) - 30) / 32);
            mask |= std::uint16_t(1U << index);
            return bits(float(10000 + (sample + index) % 12) + .75f);
        };
        if (arithmetic) {
            words = {floating ? bits(float(float_destination) + .75f)
                              : std::int32_t(integer_destination),
                     floating ? float_operand(1) : int_operand(1)};
            if (opcode >= 49 && opcode <= 58)
                words.push_back(floating ? float_operand(2) : int_operand(2));
            if ((opcode >= 45 && opcode <= 48) || (opcode >= 55 && opcode <= 58)) {
                // Division/modulo right operand is explicit and nonzero.
                words.back() = floating ? bits(-.75f) : -3;
                mask &= std::uint16_t(~(1U << (words.size() - 1)));
            }
            if (opcode == 59 || opcode == 60) {
                words[1] = opcode == 59 ? (sample % 3 == 0   ? 0
                                           : sample % 3 == 1 ? 713
                                                             : -713)
                                        : bits(sample % 3 == 0   ? 0.f
                                               : sample % 3 == 1 ? 7.25f
                                                                 : -7.25f);
                mask = 1;
            }
        } else if (opcode >= 61 && opcode <= 66) {
            words = {bits(float(float_destination) + .75f)};
            if (opcode != 66)
                words.push_back(opcode == 64 ? bits(float(int(sample % 201) - 100) / 100)
                                             : float_operand(1));
        } else if (opcode >= 67 && opcode <= 78) {
            mask = 0;
            words = opcode % 2
                        ? std::vector<std::int32_t>{int_operand(0), int_operand(1), 44, 0}
                        : std::vector<std::int32_t>{float_operand(0), float_operand(1), 44, 0};
            mask |= 12; // Jump target and clock are raw fields, even when masked.
        } else if (opcode == 5) {
            words = {10008, 0, 0};
            state.counters[0] = std::int32_t(sample % 7 + 1);
        } else if (opcode == 3 || opcode == 79) {
            words = {10008};
            state.counters[0] = std::int32_t(sample % 7);
        } else if (opcode == 83) {
            words = {std::int32_t(sample) - 2048}; // This opcode ignores the variable mask.
        }
        source.intVar0 = state.integers[0];
        source.intVar1 = state.integers[1];
        source.intVar2 = state.integers[2];
        source.intVar3 = state.integers[3];
        source.floatVar0 = state.floats[0];
        source.floatVar1 = state.floats[1];
        source.floatVar2 = state.floats[2];
        source.floatVar3 = state.floats[3];
        source.counterVar0 = state.counters[0];
        source.counterVar1 = state.counters[1];
        res::Bytes bytes;
        res::Anm anm;
        auto put16 = [&](std::uint16_t value) {
            bytes.push_back(std::uint8_t(value));
            bytes.push_back(std::uint8_t(value >> 8));
        };
        auto append = [&](int code, std::uint16_t flags, const std::vector<std::int32_t> &args) {
            const auto offset = std::uint32_t(bytes.size());
            const auto size = std::uint16_t(8 + 4 * args.size());
            put16(std::uint16_t(code));
            put16(size);
            put16(0);
            put16(flags);
            for (auto word : args) {
                put16(std::uint16_t(word));
                put16(std::uint16_t(std::uint32_t(word) >> 16));
            }
            anm.instructions.push_back({offset, std::int16_t(code), 0, size, flags});
        };
        append(opcode, mask, words);
        if (opcode >= 67 && opcode <= 78) {
            append(83, 0, {71});
            append(2, 0, {});
            append(83, 0, {93});
        }
        append(2, 0, {});
        append(-1, 0, {});
        anm.scripts.push_back({0, 0, 0, 0, std::uint32_t(anm.instructions.size())});
        const ac::Program program(res::view(bytes), anm, 0);
        source.currentInstruction = source.beginningOfScript =
            reinterpret_cast<ref::AnmRawInstr *>(bytes.data());
        th08::random::Rng rng{std::uint16_t(sample)};
        ref::g_Rng.SetSeed(std::uint16_t(sample));
        ref::g_Rng.ResetGenerationCount();
        for (unsigned call = 0; call < 100; ++call) {
            const auto rate = rates[(sample + call) % 5];
            const bool extra = sample % 11 == 0;
            g_Supervisor.framerateMultiplier = rate;
            g_Supervisor.flags.forceExtraTimerStep = extra;
            const auto expected = manager.ExecuteScript(&source);
            const auto actual = ac::advance(program, state, rate, extra, 100000, &rng);
            const std::array<std::int32_t, 4> ints{source.intVar0, source.intVar1, source.intVar2,
                                                   source.intVar3};
            const std::array<float, 4> floats{source.floatVar0, source.floatVar1, source.floatVar2,
                                              source.floatVar3};
            bool equal =
                actual.status == (expected ? ac::Status::completed : ac::Status::advanced) &&
                state.integers == ints && state.counters[0] == source.counterVar0 &&
                state.counters[1] == source.counterVar1 && state.sprite == source.sprite &&
                state.active == (source.currentInstruction != nullptr) &&
                state.visible == bool(source.visible) &&
                state.last_sprite_time == source.timeOfLastSpriteSet &&
                state.player_bullet_hit_animation_type == source.playerBulletHitAnimationType &&
                same_clock(state.time, source.currentTimeInScript) &&
                same_clock(state.wait, source.waitTimer) && rng.seed() == ref::g_Rng.GetSeed() &&
                rng.generation_count() == (opcode == 60 || (opcode == 59 && sample % 3) ? 2U : 0U);
            for (unsigned field = 0; field < 4; ++field)
                equal = equal && same_float(state.floats[field], floats[field]);
            if (source.currentInstruction)
                equal =
                    equal &&
                    program.code[state.pc].offset ==
                        std::size_t(reinterpret_cast<std::uint8_t *>(source.currentInstruction) -
                                    bytes.data());
            ++result.calls;
            if (!equal || (!expected && call == 99)) {
                if (!result.mismatches)
                    std::cerr << "ANM scalar divergence sample=" << sample << " opcode=" << opcode
                              << " call=" << call << " status=" << ac::name(actual.status) << '\n';
                ++result.mismatches;
                break;
            }
            if (expected)
                break;
        }
    };
    for (unsigned sample = 0; sample < 2048; ++sample) {
        for (int opcode = 37; opcode <= 78; ++opcode)
            scenario(sample, opcode);
        for (int opcode : {3, 5, 79, 83})
            scenario(sample, opcode);
    }
    for (unsigned seed = 2048; seed < 65536; ++seed) {
        scenario(seed, 59);
        scenario(seed, 60);
    }
    g_Supervisor.flags.forceExtraTimerStep = false;
    return result;
}
inline AnimationComparison compare_animation_control() {
    namespace ac = th08::animation::control;
    namespace res = th08::resources;
    namespace ref = anm_reference;
    AnimationComparison result;
    std::mt19937 random(20260917);
    const float rates[] = {.125f, .5f, .99f, 1, 1.5f};
    const std::int16_t interrupts[] = {1, 2, 3, 99, -1};
    auto same_clock = [](const ac::Clock &actual, const ZunTimer &expected) {
        return actual.current == expected.current && actual.previous == expected.previous &&
               std::memcmp(&actual.fraction, &expected.subFrame, sizeof(float)) == 0;
    };
    for (unsigned scenario = 0; scenario < 2000; ++scenario) {
        res::Bytes bytes;
        res::Anm anm;
        auto put16 = [&](std::uint16_t value) {
            bytes.push_back(std::uint8_t(value));
            bytes.push_back(std::uint8_t(value >> 8));
        };
        auto append = [&](int opcode, int time, std::initializer_list<std::int32_t> words) {
            const auto offset = std::uint32_t(bytes.size());
            const auto size = std::uint16_t(8 + 4 * words.size());
            put16(std::uint16_t(opcode));
            put16(size);
            put16(std::uint16_t(time));
            put16(0);
            for (auto word : words) {
                put16(std::uint16_t(word));
                put16(std::uint16_t(std::uint32_t(word) >> 16));
            }
            anm.instructions.push_back({offset, std::int16_t(opcode), std::int16_t(time), size, 0});
        };
        append(3, 0, {4});
        append(79, 0, {std::int32_t(scenario % 7)});
        append(scenario & 1 ? 20 : 23, 0, {});
        append(21, 5, {1});
        append(3, 5, {8});
        append(79, 5, {std::int32_t(scenario % 5)});
        append(89, 5, {});
        append(21, 7, {scenario % 3 ? -1 : 11});
        append(3, 7, {12});
        append(20, 7, {});
        append(21, 8, {scenario % 3 ? -1 : 12});
        append(28, 8, {2});
        append(20, 8, {});
        append(21, 9, {2});
        const auto jump_target = std::int32_t(bytes.size());
        append(3, 9, {14});
        append(scenario & 1 ? 1 : 2, 9, {});
        append(21, 10, {3});
        append(4, 10, {jump_target, 9});
        append(-1, -1, {});
        anm.scripts.push_back({0, 0, 0, 0, std::uint32_t(anm.instructions.size())});
        const ac::Program compiled(res::view(bytes), anm, 0);
        const auto program = compiled; // Immutable program copies own their decoded instructions.
        ac::State state;
        ref::AnmLoaded loaded;
        ref::AnmVm source;
        ref::AnmManager manager;
        source.anmFile = &loaded;
        source.beginningOfScript = reinterpret_cast<ref::AnmRawInstr *>(bytes.data());
        source.currentInstruction = source.beginningOfScript;
        for (unsigned frame = 0; frame < 200; ++frame) {
            const auto rate = rates[random() % 5];
            const bool extra = random() % 23 == 0;
            const bool frozen = random() % 11 == 0;
            state.frozen = source.flag19 = frozen;
            if (state.stopped && random() % 7 == 0)
                state.pending_interrupt = source.pendingInterrupt = interrupts[random() % 5];
            g_Supervisor.framerateMultiplier = rate;
            g_Supervisor.flags.forceExtraTimerStep = extra;
            const auto expected = manager.ExecuteScript(&source);
            const auto actual = ac::advance(program, state, rate, extra);
            bool equal =
                actual.status == (expected ? ac::Status::completed : ac::Status::advanced) &&
                state.active == (source.currentInstruction != nullptr) &&
                state.visible == bool(source.visible) && state.stopped == bool(source.stopped) &&
                state.sprite == source.sprite &&
                state.last_sprite_time == source.timeOfLastSpriteSet &&
                state.pending_interrupt == source.pendingInterrupt &&
                same_clock(state.time, source.currentTimeInScript) &&
                same_clock(state.wait, source.waitTimer) &&
                same_clock(state.return_time, source.interruptReturnTime);
            if (source.currentInstruction)
                equal = equal && program.code[state.pc].offset ==
                                     std::size_t(reinterpret_cast<const std::uint8_t *>(
                                                     source.currentInstruction) -
                                                 bytes.data());
            if (state.has_return)
                equal = equal && program.code[state.return_pc].offset ==
                                     std::size_t(reinterpret_cast<const std::uint8_t *>(
                                                     source.interruptReturnInstruction) -
                                                 bytes.data());
            ++result.frames;
            if (!equal) {
                if (result.mismatches == 0)
                    std::cerr << "ANM control divergence scenario=" << scenario
                              << " frame=" << frame << " status=" << ac::name(actual.status)
                              << " pc=" << state.pc << '\n';
                ++result.mismatches;
                break;
            }
            if (expected)
                break;
        }
    }
    g_Supervisor.flags.forceExtraTimerStep = false;
    return result;
}
