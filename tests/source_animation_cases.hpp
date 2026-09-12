#pragma once
#include <th08/animation_control.hpp>

struct AnimationComparison {
    std::uint64_t frames = 0, mismatches = 0;
};
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
