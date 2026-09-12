#pragma once
#include <th08/enemy_motion.hpp>

// Included after the source-generated enemy_motion_reference namespace.
struct EnemyMotionComparison {
    std::uint64_t phases = 0, mismatches = 0;
};
inline EnemyMotionComparison compare_enemy_motion() {
    namespace actual = th08::enemy;
    namespace reference = enemy_motion_reference;
    EnemyMotionComparison comparison;
    std::mt19937 random(20260916);
    auto uniform = [&](float low, float high) {
        return std::uniform_real_distribution<float>(low, high)(random);
    };
    auto same = [](float a, float b) { return std::memcmp(&a, &b, sizeof(a)) == 0; };
    auto equal_vector = [&](actual::Vec3 a, Float3 b) {
        return same(a.x, b.x) && same(a.y, b.y) && same(a.z, b.z);
    };
    auto source_vector = [](actual::Vec3 value) { return Float3(value.x, value.y, value.z); };
    auto source_flags = [](const actual::State &state) {
        return 1U | (static_cast<unsigned>(state.mode) << 12) |
               (static_cast<unsigned>(state.easing) << 14) | (unsigned(state.mirror_x) << 18) |
               (unsigned(state.clamp) << 19) | (unsigned(state.skip_integration) << 29) |
               (unsigned(state.inherit_parent_position) << 9);
    };
    auto equal = [&](const actual::State &state, const reference::Enemy &source) {
        return equal_vector(state.position, source.position) &&
               equal_vector(state.world_position, source.worldPosition) &&
               equal_vector(state.position_offset, source.positionOffset) &&
               equal_vector(state.velocity, source.velocity) &&
               equal_vector(state.previous_position, source.previousPosition) &&
               equal_vector(state.last_displacement, source.lastFrameDisplacement) &&
               equal_vector(state.interpolation_origin, source.movementInterpolationOrigin) &&
               equal_vector(state.interpolation_delta, source.movementInterpolationDelta) &&
               same(state.angle, source.movementAngle) &&
               same(state.angular_velocity, source.angularVelocity) &&
               same(state.speed, source.speed) && same(state.acceleration, source.acceleration) &&
               same(state.orbit_angle, source.orbitAngle) &&
               same(state.orbit_angular_velocity, source.orbitAngularVelocity) &&
               same(state.orbit_radius, source.orbitRadius) &&
               same(state.radial_velocity, source.radialVelocity) &&
               state.timer.current == source.movementTimer.current &&
               state.timer.previous == source.movementTimer.previous &&
               same(state.timer.fraction, source.movementTimer.subFrame) &&
               state.duration == source.movementDuration && source_flags(state) == source.flags1;
    };
    auto record = [&](bool matches, unsigned scenario, unsigned phase, const char *kind) {
        ++comparison.phases;
        if (!matches) {
            if (comparison.mismatches < 3)
                std::cerr << "Enemy movement divergence scenario=" << scenario << " phase=" << phase
                          << " kind=" << kind << '\n';
            ++comparison.mismatches;
        }
        return matches;
    };
    constexpr float rates[] = {.25f, .5f, .99f, 1.0f, 1.2f};
    for (unsigned scenario = 0; scenario < 6000; ++scenario) {
        actual::State state;
        reference::Enemy source, parent;
        auto random_vector = [&]() {
            return actual::Vec3{uniform(-32, 420), uniform(-32, 480), uniform(-4, 4)};
        };
        state.position = random_vector();
        state.position_offset = {uniform(-32, 32), uniform(-32, 32), uniform(-4, 4)};
        state.previous_position = random_vector();
        state.velocity = {uniform(-4, 4), uniform(-4, 4), uniform(-4, 4)};
        state.interpolation_origin = random_vector();
        state.interpolation_delta = {uniform(-64, 64), uniform(-64, 64), uniform(-4, 4)};
        state.angle = uniform(-130, 130);
        state.angular_velocity = uniform(-.2f, .2f);
        state.speed = uniform(-4, 4);
        state.acceleration = uniform(-.2f, .2f);
        state.orbit_angle = uniform(-130, 130);
        state.orbit_angular_velocity = uniform(-.2f, .2f);
        state.orbit_radius = uniform(-64, 64);
        state.radial_velocity = uniform(-2, 2);
        state.duration = 1 + int(random() % 32);
        state.timer.set(state.duration);
        state.timer.fraction = float(random() % 4) * .25f;
        state.mode = static_cast<actual::Mode>(scenario % 4);
        state.easing = static_cast<actual::Easing>((scenario / 4) % 8);
        state.mirror_x = scenario % 3 == 0;
        state.clamp = scenario % 3 == 1;
        state.skip_integration = scenario % 7 == 0;
        state.inherit_parent_position = scenario % 3 != 0;
        state.bounds = {{8, 12, 0}, {376, 436, 0}};
        if (scenario % 11 == 0) // Exercise the original ordered inverted-bounds branch.
            state.bounds = {{200, 220, 0}, {100, 120, 0}};
        if (scenario % 13 == 0 && state.mode != actual::Mode::interpolated)
            state.duration = 0;
        if (scenario % 17 == 0 && state.mode == actual::Mode::interpolated) {
            state.duration = -2;
            state.timer.set(-2);
        }
        actual::refresh_world(state);
        source.position = source_vector(state.position);
        source.positionOffset = source_vector(state.position_offset);
        source.worldPosition = source_vector(state.world_position);
        source.velocity = source_vector(state.velocity);
        source.previousPosition = source_vector(state.previous_position);
        source.movementInterpolationOrigin = source_vector(state.interpolation_origin);
        source.movementInterpolationDelta = source_vector(state.interpolation_delta);
        source.movementAngle = state.angle;
        source.angularVelocity = state.angular_velocity;
        source.speed = state.speed;
        source.acceleration = state.acceleration;
        source.orbitAngle = state.orbit_angle;
        source.orbitAngularVelocity = state.orbit_angular_velocity;
        source.orbitRadius = state.orbit_radius;
        source.radialVelocity = state.radial_velocity;
        source.movementTimer.current = state.timer.current;
        source.movementTimer.previous = state.timer.previous;
        source.movementTimer.subFrame = state.timer.fraction;
        source.movementDuration = state.duration;
        source.flags1 = source_flags(state);
        source.movementBounds.lower = source_vector(state.bounds.lower);
        source.movementBounds.upper = source_vector(state.bounds.upper);
        actual::Vec3 parent_position = random_vector();
        parent.position = source_vector(parent_position);
        source.parentEnemy = scenario % 5 == 0 ? nullptr : &parent;
        if (scenario % 3 != 0) {
            const int duration = 1 + int(random() % 31), easing = int(random() % 256) - 128;
            const float x = uniform(-32, 420), y = uniform(-32, 480);
            reference::EclRawInstruction instruction{duration, easing, x, y};
            actual::Status status;
            if (scenario % 3 == 1) {
                reference::ConfigureRelativeMotion(&source, &instruction);
                status = actual::configure_relative(state, duration, easing, x, y);
            } else {
                reference::ConfigurePolarMotion(&source, &instruction);
                status = actual::configure_polar(state, duration, easing, x, y);
            }
            if (!record(status == actual::Status::advanced && equal(state, source), scenario, 0,
                        "configure"))
                continue;
        }
        for (unsigned phase = 0; phase < 48; ++phase) {
            g_Supervisor.framerateMultiplier = rates[(scenario + phase) % std::size(rates)];
            g_Supervisor.flags.forceExtraTimerStep = (scenario + phase) % 11 == 0;
            source.UpdateMovement();
            auto status = actual::update_velocity(state, g_Supervisor.framerateMultiplier,
                                                  g_Supervisor.flags.forceExtraTimerStep);
            if (!record(status == actual::Status::advanced && equal(state, source), scenario, phase,
                        "velocity"))
                break;
            reference::integrate(&source);
            status = actual::integrate_position(state, g_Supervisor.framerateMultiplier,
                                                source.parentEnemy ? &parent_position : nullptr);
            if (!record(status == actual::Status::advanced && equal(state, source), scenario, phase,
                        "integration"))
                break;
            // A moving parent's local coordinate is supplied anew next phase.
            parent_position.x += .25f;
            parent.position = source_vector(parent_position);
            actual::refresh_world(state);
            source.worldPosition = source.position + source.positionOffset;
        }
    }
    g_Supervisor.flags.forceExtraTimerStep = false;
    return comparison;
}
