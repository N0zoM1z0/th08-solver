#pragma once
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <th08/camera_particle.hpp>

struct CameraParticleComparison {
    std::uint64_t initializations = 0, updates = 0, atomic_failures = 0, mismatches = 0;
};
inline CameraParticleComparison compare_camera_particles() {
    namespace cp = th08::effect::camera_particle;
    namespace ref = camera_particle_reference;
    CameraParticleComparison result;
    auto source_vector = [](cp::Vec3 value) { return ref::Float3(value.x, value.y, value.z); };
    auto same_float = [](float a, float b) { return std::memcmp(&a, &b, 4) == 0; };
    auto same_vector = [&](cp::Vec3 a, ref::Float3 b) {
        return same_float(a.x, b.x) && same_float(a.y, b.y) && same_float(a.z, b.z);
    };
    auto source_color = [](cp::Color color) {
        ref::ZunColor result;
        result.r = color.r;
        result.g = color.g;
        result.b = color.b;
        result.a = color.a;
        return result;
    };
    auto same_color = [](cp::Color a, ref::ZunColor b) {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    };
    auto source_state = [&](const cp::State &state) {
        ref::Effect effect;
        effect.position = source_vector(state.position);
        effect.vector1 = source_vector(state.inherited_velocity);
        effect.vector2 = source_vector(state.velocity);
        effect.vector3 = source_vector(state.acceleration);
        effect.vector4 = source_vector(state.particle_position);
        effect.vm.pos2 = source_vector(state.animation.position_offset);
        effect.vm.posInitial = source_vector(state.animation.position_initial);
        effect.vm.posFinal = source_vector(state.animation.position_final);
        effect.vm.rotateInitial = source_vector(state.animation.rotation_initial);
        effect.vm.flags = state.animation.flags;
        effect.vm.color1 = source_color(state.animation.primary);
        effect.vm.color2 = source_color(state.animation.secondary);
        effect.drawGroup = state.draw_group;
        return effect;
    };
    auto equal = [&](const cp::State &state, const ref::Effect &effect) {
        return same_vector(state.position, effect.position) &&
               same_vector(state.inherited_velocity, effect.vector1) &&
               same_vector(state.velocity, effect.vector2) &&
               same_vector(state.acceleration, effect.vector3) &&
               same_vector(state.particle_position, effect.vector4) &&
               same_vector(state.animation.position_offset, effect.vm.pos2) &&
               same_vector(state.animation.position_initial, effect.vm.posInitial) &&
               same_vector(state.animation.position_final, effect.vm.posFinal) &&
               same_vector(state.animation.rotation_initial, effect.vm.rotateInitial) &&
               same_color(state.animation.primary, effect.vm.color1) &&
               same_color(state.animation.secondary, effect.vm.color2) &&
               state.animation.flags == effect.vm.flags && state.draw_group == effect.drawGroup;
    };
    auto prepared = [](unsigned seed) {
        // Every field is an explicit fixture input; no allocation/ANM defaults are inferred.
        cp::State state{};
        const float value = float(seed % 53);
        state.position = {value, -value, 1};
        state.inherited_velocity = {.125f, -.25f, .5f};
        state.velocity = {-1, 2, -3};
        state.acceleration = {4, -5, 6};
        state.particle_position = {-7, 8, -9};
        state.animation.position_offset = {value, 17, 23};
        state.animation.position_initial = {-31, value, 37};
        state.animation.position_final = {41, -43, value};
        state.animation.rotation_initial = {47, value, -53};
        state.animation.primary = {std::uint8_t(seed), std::uint8_t(seed >> 8), 127, 255};
        state.animation.secondary = {73, 79, 83, 89};
        state.animation.flags = 0xa5c01020U ^ seed * 0x9e3779b9U;
        state.draw_group = -3;
        return state;
    };
    auto fail = [&](const char *phase, unsigned sample) {
        if (result.mismatches < 4)
            std::cerr << "Camera particle divergence phase=" << phase << " sample=" << sample
                      << '\n';
        ++result.mismatches;
    };
    auto set_camera = [&](const cp::Camera &camera) {
        ref::g_Background.cameraCurrent.position = source_vector(camera.position);
        ref::g_Background.cameraCurrent.lookAtOffset = source_vector(camera.look_at_offset);
        ref::g_Background.cameraCurrent.forward = source_vector(camera.forward);
    };
    std::array<ref::Enemy, 8> enemies;
    auto set_bosses = [&](cp::Bosses bosses) {
        for (unsigned slot = 0; slot < 8; ++slot) {
            enemies[slot].position = source_vector(bosses.primary_position);
            enemies[slot].flags1 =
                bosses.primary_damageable ? 1U << ref::ENEMY_FLAG_DAMAGEABLE_SHIFT : 0;
            ref::g_EnemyManager.bosses[slot] =
                bosses.occupied_slots & (1U << slot) ? &enemies[slot] : nullptr;
        }
    };
    auto compare_update = [&](cp::State &state, ref::Effect &effect, const cp::Camera &camera,
                              const cp::Bosses &bosses, cp::Color tint, unsigned sample) {
        set_camera(camera);
        set_bosses(bosses);
        ref::g_Background.stageTextVm.color1 = source_color(tint);
        const auto seed = ref::g_Rng.GetSeed();
        const auto expected = ref::UpdateTintedBossTrackingCameraParticle(&effect);
        const auto actual = cp::update(state, &camera, &bosses, &tint);
        ++result.updates;
        if (actual != (expected ? cp::Status::alive : cp::Status::culled) ||
            !equal(state, effect) || ref::g_Rng.GetSeed() != seed)
            fail("update", sample);
    };
    constexpr float rates[] = {0, .125f, .5f, .99f, 1, 1.5f, -.5f};
    for (unsigned seed = 0; seed < 65536; ++seed) {
        auto state = prepared(seed);
        auto effect = source_state(state);
        cp::Camera camera{{float(seed % 31) - 15, -32, 64}, {12, -24, -80}, {0, 0, -1}};
        set_camera(camera);
        ref::g_Supervisor.framerateMultiplier = rates[seed % std::size(rates)];
        ref::g_Rng.SetSeed(std::uint16_t(seed));
        ref::g_Rng.ResetGenerationCount();
        th08::random::Rng rng{std::uint16_t(seed)};
        const auto expected = ref::InitializeTintedBossTrackingCameraParticle(&effect);
        const auto actual = cp::initialize(state, &camera, rates[seed % std::size(rates)], &rng);
        ++result.initializations;
        if (expected != 0 || actual != cp::Status::initialized || !equal(state, effect) ||
            rng.seed() != ref::g_Rng.GetSeed() || rng.generation_count() != 16)
            fail("initialize", seed);
        for (unsigned frame = 0; frame < 4; ++frame) {
            // Change the camera independently between callback phases. The
            // initializer's multiplier is never applied a second time here.
            camera.position.x += .25f;
            const cp::Vec3 delta{state.particle_position.x + state.velocity.x +
                                     state.acceleration.x - camera.position.x,
                                 state.particle_position.y + state.velocity.y +
                                     state.acceleration.y - camera.position.y,
                                 state.particle_position.z + state.velocity.z +
                                     state.acceleration.z - camera.position.z};
            const float length =
                std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
            camera.forward = {delta.x / length, delta.y / length, delta.z / length};
            if (frame == 3 && seed % 2)
                camera.forward = {-camera.forward.x, -camera.forward.y, -camera.forward.z};
            const cp::Bosses bosses{std::uint8_t(frame == 0   ? 0
                                                 : frame == 1 ? 1
                                                              : 0x81),
                                    frame != 1,
                                    {192 + float(frame), 80 - float(frame), float(frame)}};
            const cp::Color tint{std::uint8_t(seed >> 8), std::uint8_t(seed), 255, 128};
            compare_update(state, effect, camera, bosses, tint, seed * 4 + frame);
        }
    }
    const cp::Bosses absent{0, false, {0, 0, 0}}, boss{1, true, {100, 200, 3}};
    const cp::Color white{255, 255, 255, 255};
    // Independent normalization and exact-threshold inputs, including signed
    // zero, the modern tiny-vector cutoff and non-unit forward vectors.
    const float distances[] = {0, -0.0f, 1e-9f, 1e-8f, std::nextafter(1e-8f, 1.0f), 1, 100};
    const float forwards[] = {
        std::nextafter(.94f, 0.0f), .94f, std::nextafter(.94f, 1.0f), -1, 0, 1, 2};
    for (unsigned i = 0; i < std::size(distances); ++i)
        for (unsigned j = 0; j < std::size(forwards); ++j) {
            auto state = prepared(i * 7 + j);
            state.velocity = state.acceleration = {0, 0, 0};
            state.particle_position = {0, 0, distances[i]};
            auto effect = source_state(state);
            const cp::Camera camera{{0, 0, 0}, {17, 19, 23}, {0, 0, forwards[j]}};
            compare_update(state, effect, camera, boss, white, i * 7 + j);
        }
    const float sentinel[] = {-10000, -9999, std::nextafter(-9999.0f, 0.0f), -2, 0, 11};
    for (unsigned mask = 0; mask < 256; ++mask) {
        if (mask && !(mask & 1))
            continue; // The reference would dereference missing boss slot zero.
        for (unsigned index = 0; index < std::size(sentinel); ++index) {
            auto state = prepared(mask);
            state.velocity = state.acceleration = {0, 0, 0};
            state.particle_position = {0, 0, 1};
            state.animation.position_offset = {sentinel[index], -50, 25};
            auto effect = source_state(state);
            const cp::Camera camera{{0, 0, 0}, {0, 0, 0}, {0, 0, 1}};
            const cp::Bosses bosses{std::uint8_t(mask), index % 2 != 0, {100, 200, 3}};
            compare_update(state, effect, camera, bosses, white, mask * 6 + index);
        }
    }
    // Every byte-pair product is checked in all four source color channels.
    for (unsigned primary = 0; primary < 256; ++primary)
        for (unsigned tint = 0; tint < 256; ++tint) {
            auto state = prepared(primary);
            state.velocity = state.acceleration = {0, 0, 0};
            state.particle_position = {0, 0, 1};
            const auto p = std::uint8_t(primary), t = std::uint8_t(tint);
            state.animation.primary = {p, p, p, p};
            auto effect = source_state(state);
            const cp::Camera camera{{0, 0, 0}, {0, 0, 0}, {0, 0, 1}};
            compare_update(state, effect, camera, absent, {t, t, t, t}, primary * 256 + tint);
        }
    // Native failure atomicity is separate from source callback success/culling.
    auto state = prepared(17);
    auto before = source_state(state);
    const cp::Camera camera{{0, 0, 0}, {0, 0, 0}, {0, 0, 1}};
    th08::random::Rng rng(17);
    rng.save_seed();
    auto atomic = [&](bool passed) {
        ++result.atomic_failures;
        if (!passed)
            fail("atomic", unsigned(result.atomic_failures));
    };
    atomic(cp::initialize(state, nullptr, 1, &rng) == cp::Status::missing_context &&
           equal(state, before) && rng.generation_count() == 0);
    atomic(cp::initialize(state, &camera, 1, nullptr) == cp::Status::missing_context &&
           equal(state, before));
    state.inherited_velocity.x = std::numeric_limits<float>::max();
    before = source_state(state);
    atomic(cp::initialize(state, &camera, 2, &rng) == cp::Status::invalid_state &&
           equal(state, before) && rng.seed() == 17 && rng.generation_count() == 0 &&
           rng.state().saved_seed_valid && rng.state().saved_seed == 17);
    state = prepared(17);
    state.velocity = {0, 0, .25f};
    state.acceleration = {0, 0, 0};
    state.particle_position = {0, 0, 1};
    before = source_state(state);
    atomic(cp::update(state, nullptr, &absent, &white) == cp::Status::missing_context &&
           equal(state, before));
    atomic(cp::update(state, &camera, nullptr, &white) == cp::Status::missing_context &&
           equal(state, before));
    atomic(cp::update(state, &camera, &boss, nullptr) == cp::Status::missing_context &&
           equal(state, before));
    const cp::Bosses broken{2, true, {100, 200, 3}};
    atomic(cp::update(state, &camera, &broken, &white) == cp::Status::invalid_state &&
           equal(state, before));
    state.particle_position.x = std::numeric_limits<float>::infinity();
    before = source_state(state);
    atomic(cp::update(state, &camera, &absent, &white) == cp::Status::invalid_state &&
           equal(state, before));
    return result;
}
