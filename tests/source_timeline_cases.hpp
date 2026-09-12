#pragma once
#include <th08/timeline.hpp>

struct TimelineComparison {
    std::uint64_t frames = 0, mismatches = 0;
};
inline TimelineComparison compare_timeline() {
    namespace tl = th08::timeline;
    namespace res = th08::resources;
    namespace ref = timeline_reference;
    TimelineComparison result;
    std::mt19937 random(20260923);
    const float rates[] = {.125f, .5f, .99f, 1, 1.5f};
    for (unsigned scenario = 0; scenario < 2000; ++scenario) {
        res::Bytes bytes;
        res::Ecl ecl;
        auto word = [&](std::uint32_t value) {
            for (unsigned shift = 0; shift < 32; shift += 8)
                bytes.push_back(std::uint8_t(value >> shift));
        };
        auto append = [&](std::uint16_t opcode, int time,
                          std::initializer_list<std::int32_t> payload, std::uint8_t mask = 255) {
            const auto offset = std::uint32_t(bytes.size());
            const auto size = std::uint8_t(8 + 4 * payload.size());
            word(std::uint32_t(time));
            word(opcode | (std::uint32_t(size) << 16) | (std::uint32_t(mask) << 24));
            for (auto value : payload)
                word(std::uint32_t(value));
            ecl.timeline_instructions.push_back({offset, time, opcode, size, mask});
        };
        const int event = int(scenario % 7) - 2;
        append(14, 0, {event});
        append(0, 0, {12, 0, 0, 20, -1, 100});
        append(1, 0, {13, 0, 0, 20, -1, 100}, 5);
        append(13, 0, {event});
        append(7, 0, {});
        append(6, 0, {3});
        append(10, 0, {0});
        append(8, 0, {1, 2});
        append(11, 1, {14, 0, 0, 20, 1, 2, 100});
        append(12, 1, {15, 0, 0, 20, 1, 2, 100});
        append(9, 1, {128});
        append(15, 1, {16, 0, 0, 20, -1, 100});
        append(14, 2, {3});
        append(13, 2, {4});
        append(16, 2, {});
        append(6, 10, {4});
        append(16, 3, {}); // Stale instruction must be skipped, not caught up.
        const auto terminal = std::uint32_t(bytes.size());
        word(0xffffffff);
        word(0);
        ecl.timelines.push_back({0, terminal, 0, std::uint32_t(ecl.timeline_instructions.size())});
        const tl::Program program(res::view(bytes), ecl, 0);
        tl::State state;
        ref::EclTimeline source;
        source.timer.previous = 0; // Source manager memset after timer construction.
        source.instruction = reinterpret_cast<ref::EclTimelineInstruction *>(bytes.data());
        ref::timeline_start = bytes.data();
        ref::active_timeline = &source;
        ref::Enemy bosses[2];
        ref::Gui::Impl gui;
        for (unsigned frame = 0; frame < 100; ++frame) {
            const auto rate = rates[random() % 5];
            const bool extra = random() % 29 == 0;
            g_Supervisor.framerateMultiplier = rate;
            g_Supervisor.flags.forceExtraTimerStep = extra;
            const auto difficulty = std::uint8_t(1U << (scenario % 4));
            ref::g_GameManager.difficultyMask = difficulty;
            ref::g_Gui.bossPresent = random() % 5 == 0;
            ref::g_EnemyManager.suppressTimelineSpawns = random() % 7 == 0;
            ref::g_Gui.impl = random() % 5 ? &gui : nullptr;
            gui.message.ignoreWaitCounter = int(random() % 3) - 1;
            gui.message.currentMsgIdx = int(random() % 4) - 1;
            bosses[0].flags1 = random() % 4 == 0 ? 1 : 0;
            ref::g_EnemyManager.bosses[0] = random() % 5 ? &bosses[0] : nullptr;
            ref::g_EnemyManager.bosses[1] = &bosses[1];
            tl::Context world;
            auto known = [](bool value) { return value ? tl::KnownBool::yes : tl::KnownBool::no; };
            world.gui_boss_present = known(ref::g_Gui.bossPresent);
            world.spawns_suppressed = known(ref::g_EnemyManager.suppressTimelineSpawns != 0);
            // Derive the adapter input independently from fields, not by calling
            // the source predicate whose dispatch/gate behavior is under comparison.
            world.message_waiting = known(ref::g_Gui.impl && gui.message.ignoreWaitCounter <= 0 &&
                                          gui.message.currentMsgIdx >= 0);
            world.boss_active[0] = known(ref::g_EnemyManager.bosses[0] && (bosses[0].flags1 & 1));
            world.events_known = true;
            for (unsigned slot = 0; slot < 4; ++slot)
                world.events[slot] = ref::g_EnemyManager.timelineEventSlots[slot] =
                    int(random() % 7) - 2;
            ref::effect_count = 0;
            source.Run();
            std::array<std::uint32_t, 64> effects{};
            unsigned count = 0;
            tl::Result actual;
            for (;;) {
                actual = tl::advance(program, state, world, difficulty, rate, extra);
                if (actual.status != tl::Status::external_effect)
                    break;
                if (count >= effects.size() ||
                    !tl::acknowledge_effect(program, state, actual.effect_token))
                    throw std::runtime_error("timeline effect comparison bound");
                effects[count++] = actual.offset;
            }
            bool equal =
                actual.status == tl::Status::frame_complete && count == ref::effect_count &&
                state.time.current == source.timer.current &&
                state.time.previous == source.timer.previous &&
                std::memcmp(&state.time.fraction, &source.timer.subFrame, sizeof(float)) == 0 &&
                program.code[state.pc].offset ==
                    std::size_t(reinterpret_cast<const std::uint8_t *>(source.instruction) -
                                bytes.data()) &&
                actual.at_end == (source.instruction->time < 0);
            for (unsigned i = 0; i < count && i < ref::effect_count; ++i)
                equal = equal && effects[i] == ref::effect_offsets[i];
            for (unsigned i = 0; i < 4; ++i)
                equal = equal && world.events[i] == ref::g_EnemyManager.timelineEventSlots[i];
            ++result.frames;
            if (!equal) {
                if (!result.mismatches)
                    std::cerr << "timeline divergence scenario=" << scenario << " frame=" << frame
                              << " pc=" << state.pc << " status=" << tl::name(actual.status)
                              << '\n';
                ++result.mismatches;
                break;
            }
        }
    }
    ref::active_timeline = nullptr;
    g_Supervisor.flags.forceExtraTimerStep = false;
    return result;
}
