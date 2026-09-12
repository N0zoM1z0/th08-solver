#pragma once
#include <th08/transform_program.hpp>

// Included after the source-generated transform_reference namespace.
struct TransformComparison {
    std::uint64_t steps = 0, mismatches = 0;
};
inline TransformComparison compare_transforms() {
    namespace actual = th08::bullet::transform;
    namespace reference = transform_reference;
    TransformComparison comparison;
    std::mt19937 random(20260914);
    auto uniform = [&](float low, float high) {
        return std::uniform_real_distribution<float>(low, high)(random);
    };
    auto same = [](float a, float b) { return std::memcmp(&a, &b, sizeof(a)) == 0; };
    constexpr unsigned kinds[] = {actual::decelerate, actual::vector,     actual::polar,
                                  actual::relative,   actual::absolute,   actual::aimed,
                                  actual::wait,       actual::cull_delay, actual::sound,
                                  actual::despawn,    actual::none};
    constexpr float rates[] = {.25f, .5f, .99f, 1.0f, 1.2f};
    for (unsigned scenario = 0; scenario < 2000; ++scenario) {
        actual::Program program;
        actual::State state;
        reference::Bullet source;
        state.enabled_flags = 0xffffffffU;
        if (scenario % 5 == 0)
            state.enabled_flags ^= actual::vector | actual::aimed;
        state.transform_sound = scenario % 3 ? 5 : -1;
        state.flight = {uniform(0, 384), 100,           uniform(-3, 3), uniform(-3, 3),
                        uniform(-3, 3),  uniform(-2, 5)};
        source.position = {state.flight.x, state.flight.y};
        source.velocity = {state.flight.velocity_x, state.flight.velocity_y};
        source.angle = state.flight.angle;
        source.speed = state.flight.speed;
        source.transformFlags = state.enabled_flags;
        source.transformSound = state.transform_sound;
        for (unsigned i = 0; i < 18; ++i) {
            auto &record = program.records[i];
            record.kind = kinds[random() % std::size(kinds)];
            record.allow_while_active = int(random() % 2);
            record.int0 = int(random() % 8) - 1;
            record.int1 = int(random() % 3) + 1;
            record.float0 = uniform(-1, 1);
            record.float1 = random() % 3 == 0 ? -999.0f : uniform(-2, 3);
            auto &expected = source.transforms[i];
            expected.kind = record.kind;
            expected.allowWhileActive = record.allow_while_active;
            expected.payload.raw = {record.float0, record.float1, record.int0, record.int1};
        }
        for (unsigned phase = 0; phase < 81 && !state.despawning; ++phase) {
            g_Supervisor.framerateMultiplier = rates[(phase + scenario) % std::size(rates)];
            g_Supervisor.flags.forceExtraTimerStep = scenario % 7 == 0;
            supplied_target_angle = uniform(-3, 3);
            reference::g_SoundPlayer.count = 0;
            actual::Result result;
            if (phase == 0) {
                source.AdvanceTransformProgram();
                result = actual::advance_program(program, state, g_Supervisor.framerateMultiplier);
            } else {
                reference::step(&source);
                result =
                    actual::step(program, state, g_Supervisor.framerateMultiplier,
                                 g_Supervisor.flags.forceExtraTimerStep, supplied_target_angle);
            }
            bool equal = result.status == th08::bullet::Status::advanced &&
                         state.pc == unsigned(source.transformIndex) &&
                         state.active_flags == source.activeTransformFlags &&
                         state.offscreen_cull_delay == source.offscreenCullDelayFrames &&
                         state.despawning == (source.state == reference::BULLET_STATE_DESPAWNING) &&
                         same(state.flight.angle, source.angle) &&
                         same(state.flight.speed, source.speed) &&
                         same(state.flight.velocity_x, source.velocity.x) &&
                         same(state.flight.velocity_y, source.velocity.y) &&
                         result.sound_count == reference::g_SoundPlayer.count;
            for (unsigned i = 0; i < std::min(result.sound_count, reference::g_SoundPlayer.count);
                 ++i) {
                const auto &a = result.sounds[i], &b = reference::g_SoundPlayer.events[i];
                equal = equal && a.id == b.id && a.positioned == b.positioned && same(a.x, b.x);
            }
            for (unsigned i = 0; i < 3; ++i)
                equal = equal && state.acceleration[i].timer == int(source.exStates[i].timer) &&
                        same(state.acceleration[i].subframe, source.exStates[i].timer.subFrame);
            const auto &vector = state.acceleration[1];
            const auto &polar = state.acceleration[2];
            const auto &turn = source.exStates[3];
            equal = equal && same(vector.vector_x, source.exStates[1].vector.x) &&
                    same(vector.vector_y, source.exStates[1].vector.y) &&
                    vector.duration == source.exStates[1].durationFrames &&
                    same(polar.speed_delta, source.exStates[2].speedDelta) &&
                    same(polar.angle_delta, source.exStates[2].angleDelta) &&
                    polar.duration == source.exStates[2].durationFrames &&
                    state.turn.timer == source.exStates[3].timer.current &&
                    same(state.turn.subframe, turn.timer.subFrame) &&
                    state.turn.interval == turn.directionChangeIntervalFrames &&
                    state.turn.repeats == turn.directionChangeRepeatCount &&
                    state.turn.completed == turn.directionChangesCompleted &&
                    same(state.turn.angle, turn.directionChangeAngle) &&
                    same(state.turn.speed, turn.directionChangeSpeed) &&
                    state.wait_timer == source.exStates[5].timer.current &&
                    same(state.wait_subframe, source.exStates[5].timer.subFrame);
            if (!equal) {
                if (comparison.mismatches < 3)
                    std::cerr << "Transform divergence scenario=" << scenario << " phase=" << phase
                              << " pc=" << state.pc << '/' << source.transformIndex
                              << " flags=" << state.active_flags << '/'
                              << source.activeTransformFlags << '\n';
                ++comparison.mismatches;
                break;
            }
            ++comparison.steps;
        }
    }
    g_Supervisor.flags.forceExtraTimerStep = false;
    return comparison;
}
