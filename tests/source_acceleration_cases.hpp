#pragma once
#include <th08/acceleration.hpp>

// Included only after the generated pinned Bullet methods and adapter layouts.
struct AccelerationComparison {
    std::uint64_t frames = 0, mismatches = 0;
};
inline AccelerationComparison compare_acceleration() {
    namespace motion = th08::bullet;
    AccelerationComparison result;
    std::mt19937 random(20260913);
    auto uniform = [&](float low, float high) {
        return std::uniform_real_distribution<float>(low, high)(random);
    };
    auto same = [](float a, float b) { return std::memcmp(&a, &b, sizeof(a)) == 0; };
    const float rates[] = {.25f, .5f, .99f, 1.0f, 1.2f};
    const unsigned flags[] = {1, 0x10, 0x20};
    for (unsigned scenario = 0; scenario < 6000; ++scenario) {
        const unsigned mode = scenario % 3;
        motion::Acceleration state;
        state.mode = motion::AccelerationMode(mode);
        state.active = true;
        state.duration = int(random() % 82) - 2;
        state.subframe = scenario % 2 ? .5f : 0;
        state.vector_x = uniform(-.1f, .1f);
        state.vector_y = uniform(-.1f, .1f);
        state.speed_delta = uniform(-.1f, .1f);
        state.angle_delta = uniform(-1, 1);
        motion::Flight flight{0, 0, uniform(-5, 5), uniform(-5, 5), uniform(-5, 5), uniform(-2, 8)};
        Bullet reference;
        reference.velocity = {flight.velocity_x, flight.velocity_y};
        reference.angle = flight.angle;
        reference.speed = flight.speed;
        reference.activeTransformFlags = flags[mode];
        auto &expected = reference.exStates[mode];
        expected.timer.subFrame = state.subframe;
        expected.durationFrames = state.duration;
        expected.vector = {state.vector_x, state.vector_y};
        expected.speedDelta = state.speed_delta;
        expected.angleDelta = state.angle_delta;
        for (unsigned frame = 0; frame < 300 && state.active; ++frame) {
            g_Supervisor.framerateMultiplier = rates[(scenario + frame) % 5];
            switch (mode) {
            case 0:
                reference.UpdateDeceleration();
                break;
            case 1:
                reference.UpdateVectorAcceleration();
                break;
            case 2:
                reference.UpdatePolarAcceleration();
                break;
            }
            const auto status =
                motion::advance_acceleration(flight, state, g_Supervisor.framerateMultiplier);
            if (status != motion::Status::advanced || !same(flight.angle, reference.angle) ||
                !same(flight.speed, reference.speed) ||
                !same(flight.velocity_x, reference.velocity.x) ||
                !same(flight.velocity_y, reference.velocity.y) ||
                state.timer != int(expected.timer) ||
                !same(state.subframe, expected.timer.subFrame) ||
                state.active != bool(reference.activeTransformFlags & flags[mode])) {
                if (result.mismatches == 0)
                    std::cerr << "Acceleration divergence scenario=" << scenario
                              << " frame=" << frame << '\n';
                ++result.mismatches;
            }
            ++result.frames;
        }
    }
    return result;
}
