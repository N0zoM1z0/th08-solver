#include <cstdlib>
#include <iostream>
#include <limits>
#include <th08/camera_particle.hpp>

namespace cp = th08::effect::camera_particle;
void check(bool value, const char *message) {
    if (!value) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
cp::State prepared() {
    // Explicit fixture snapshot, not a default effect-pool/template assumption.
    cp::State state{};
    state.position = {11, 12, 13};
    state.inherited_velocity = {.25f, -.5f, .125f};
    state.animation.position_offset = {17, 18, 19};
    state.animation.position_initial = {20, 21, 22};
    state.animation.position_final = {23, 24, 25};
    state.animation.rotation_initial = {26, 27, 28};
    state.animation.primary = {255, 128, 64, 255};
    state.animation.secondary = {3, 4, 5, 6};
    state.animation.flags = 0xa5002040U;
    state.draw_group = -5;
    return state;
}
int main() {
    const cp::Camera camera{{0, 0, 0}, {0, 0, -1}, {0, 0, 1}};
    const cp::Bosses absent{0, false, {0, 0, 0}};
    const cp::Color white{255, 255, 255, 255};
    th08::random::Rng rng(5123), expected_rng(5123);
    auto state = prepared();
    check(cp::initialize(state, nullptr, 1, &rng) == cp::Status::missing_context &&
              rng.generation_count() == 0 && state.position.x == 11,
          "camera initializer invented camera state");
    check(cp::initialize(state, &camera, 1, nullptr) == cp::Status::missing_context,
          "camera initializer invented an RNG stream");
    check(cp::initialize(state, &camera, .5f, &rng) == cp::Status::initialized,
          "camera initializer rejected the explicit fixture");
    for (unsigned i = 0; i < 16; ++i)
        expected_rng.next_u16();
    check(rng.generation_count() == 16 && rng.seed() == expected_rng.seed(),
          "effect51 initializer did not consume exactly eight float RNG calls");
    check(state.position.x == 11 && state.draw_group == 1 &&
              state.animation.position_offset.x == -9999 &&
              state.animation.position_offset.y == 18 && state.animation.position_offset.z == 19 &&
              state.animation.position_initial.x == 0 && state.animation.position_initial.y == 21 &&
              state.animation.position_initial.z == 22 && state.animation.position_final.x == 0 &&
              state.animation.position_final.y == 0 && state.animation.position_final.z == 0 &&
              state.animation.rotation_initial.x == 0 && state.animation.rotation_initial.y == 0 &&
              state.animation.rotation_initial.z == 0 && state.animation.flags == 0xa5002040U &&
              state.animation.primary.r == 255,
          "initializer changed untouched template fields or skipped ANM consumers");
    const auto checkpoint = rng.state();
    state = prepared();
    state.inherited_velocity.x = std::numeric_limits<float>::max();
    check(cp::initialize(state, &camera, 2, &rng) == cp::Status::invalid_state &&
              rng.seed() == checkpoint.seed &&
              rng.generation_count() == checkpoint.generation_count && state.draw_group == -5,
          "late initialization failure leaked a partial state or RNG draws");
    state = prepared();
    state.velocity = {0, 0, -1};
    state.acceleration = {0, 0, -.25f};
    state.particle_position = {0, 0, 2};
    check(cp::update(state, &camera, &absent, &white) == cp::Status::alive &&
              state.velocity.z == -1.25f && state.position.z == .75f &&
              state.animation.secondary.r == 254 && state.animation.secondary.g == 127 &&
              state.animation.secondary.b == 63 && state.animation.secondary.a == 254 &&
              state.animation.flags == (0xa5002040U | 0x20000U),
          "callback scaling, tint denominator or flag preservation changed");
    const auto prior_secondary = state.animation.secondary;
    const auto prior_target = state.animation.position_offset;
    check(cp::update(state, &camera, nullptr, nullptr) == cp::Status::culled &&
              state.velocity.z == -1.5f && state.position.z == -.75f &&
              state.animation.secondary.r == prior_secondary.r &&
              state.animation.position_offset.x == prior_target.x,
          "cull rolled back prior motion or read later boss/tint inputs");
    state = prepared();
    state.particle_position = {0, 0, 1};
    const cp::Bosses boss{1, true, {100, 200, 3}};
    state.animation.position_offset = {-9999, 17, 21};
    check(cp::update(state, &camera, &boss, &white) == cp::Status::alive &&
              state.animation.position_offset.x == 100 && state.animation.position_offset.z == 3,
          "first boss tracking did not replace all three sentinel target coordinates");
    const cp::Bosses moved{0x81, true, {110, 180, 13}};
    check(cp::update(state, &camera, &moved, &white) == cp::Status::alive &&
              state.animation.position_offset.x == 101 &&
              state.animation.position_offset.y == 198 && state.animation.position_offset.z == 4,
          "boss tracking did not apply the source ten-percent local-position update");
    const cp::Bosses invulnerable{1, false, {300, 300, 3}};
    check(cp::update(state, &camera, &invulnerable, &white) == cp::Status::alive &&
              state.animation.position_offset.x == 101,
          "non-damageable boss changed the stored tracking position");
    state.velocity.z = .5f;
    const auto before = state;
    check(cp::update(state, &camera, &boss, nullptr) == cp::Status::missing_context &&
              state.position.z == before.position.z &&
              state.particle_position.z == before.particle_position.z &&
              state.animation.position_offset.x == before.animation.position_offset.x,
          "missing tint leaked earlier movement or boss interpolation");
    const cp::Bosses invalid_bosses{2, true, {1, 2, 3}};
    check(cp::update(state, &camera, &invalid_bosses, &white) == cp::Status::invalid_state,
          "nonempty boss list without slot zero silently selected a different boss");
    state = prepared();
    state.particle_position = {0, 0, 1};
    auto threshold_camera = camera;
    threshold_camera.forward.z = .94f;
    check(cp::update(state, &threshold_camera, &absent, &white) == cp::Status::alive,
          "camera particle culled equality at the source dot threshold");
    threshold_camera.forward.z = std::nextafter(.94f, 0.0f);
    check(cp::update(state, &threshold_camera, nullptr, nullptr) == cp::Status::culled,
          "camera particle retained the float immediately below the threshold");
    state.particle_position = {0, 0, 0};
    check(cp::update(state, &camera, nullptr, nullptr) == cp::Status::culled,
          "zero camera delta did not follow the modern zero-normalization rule");
    for (unsigned visible : {0U, 1U, 0U}) {
        state.animation.flags = visible;
        state.particle_position = {0, 0, 1.0e-9f};
        check(cp::update(state, &camera, nullptr, nullptr) == cp::Status::culled,
              "tiny camera delta bypassed the source normalization cutoff");
        state.particle_position.z = 1.0e-7f;
        check(cp::update(state, &camera, &absent, &white) == cp::Status::alive,
              "normalization cutoff or unrelated visibility suppressed a valid direction");
    }
    std::cout << "Effect51 camera callback RNG, tint, boss tracking and cull ordering: passed\n";
}
