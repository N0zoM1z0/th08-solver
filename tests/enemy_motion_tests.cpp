#include <cstdlib>
#include <iostream>
#include <th08/enemy_motion.hpp>

namespace enemy = th08::enemy;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    enemy::State state;
    state.position = {10, 20, 3};
    state.position_offset = {100, 200, 4};
    state.velocity = {7, 8, 9};
    check(enemy::refresh_world(state) == enemy::Status::advanced && state.world_position.x == 110 &&
              state.world_position.z == 7,
          "ECL publication confused world/local coordinates or forced z early");
    check(enemy::configure_relative(state, 4, 0, 150, 240) == enemy::Status::advanced &&
              state.interpolation_origin.x == 10 && state.interpolation_delta.x == 40 &&
              state.interpolation_delta.z == -7 && state.velocity.z == 0,
          "relative interpolation installed the wrong origin or world displacement");
    check(enemy::update_velocity(state, .5f, false) == enemy::Status::advanced &&
              state.timer.current == 3 && state.timer.fraction == .5f && state.velocity.x == 5 &&
              state.position.x == 10,
          "fractional interpolation lost the subframe or integrated before shot dispatch");
    check(enemy::integrate_position(state, .5f) == enemy::Status::advanced &&
              state.position.x == 12.5f && state.world_position.x == 112.5f &&
              state.world_position.z == 0 && state.last_displacement.x == 10,
          "interpolation integration omitted the second multiplier or lagged displacement");
    state.timer.set(1);
    check(enemy::update_velocity(state, 1, false) == enemy::Status::advanced &&
              state.mode == enemy::Mode::none && state.position.x == 50 && state.velocity.x == 0,
          "interpolation endpoint failed to snap before the manager integration");
    const auto completed = state;
    check(enemy::configure_relative(state, 0, 0, 0, 0) == enemy::Status::invalid_state &&
              state.position.x == completed.position.x && state.timer.current == 0,
          "zero-duration division domain was silently accepted or mutated");

    state = {};
    state.position = {20, 40, 0};
    state.position_offset = {100, 200, 0};
    enemy::refresh_world(state);
    state.mirror_x = true;
    check(enemy::configure_polar(state, 4, 15, 0, 2) == enemy::Status::advanced &&
              state.interpolation_origin.x == 120 && state.interpolation_delta.x == -8 &&
              state.easing == enemy::Easing::unnamed_linear,
          "finite polar configuration lost world origin, mirror or low three easing bits");
    enemy::update_velocity(state, 1, false);
    check(state.velocity.x == -98 && state.position.x == 20,
          "mirrored interpolation must invert velocity before mirror integration");
    enemy::integrate_position(state, 1);
    check(state.position.x == 118, "mirrored interpolation applied the reflection only once");

    state = {};
    state.mode = enemy::Mode::polar;
    state.speed = 2;
    state.acceleration = -6;
    state.duration = 2;
    state.timer.set(2);
    state.velocity.z = 17;
    enemy::update_velocity(state, .5f, true);
    check(state.mode == enemy::Mode::none && state.speed == -1 && state.velocity.x == -1 &&
              state.velocity.z == 0 && state.timer.current == 0 && state.timer.fraction == .5f &&
              state.timer.previous == 1,
          "polar update lost signed speed, z reset or extra-step fractional countdown");
    state.mode = enemy::Mode::orbit;
    state.duration = 0;
    state.orbit_radius = 8;
    state.velocity.z = 17;
    enemy::update_velocity(state, 1, false);
    check(state.velocity.x == 8 && state.velocity.z == 17,
          "orbit mode reset the source-retained z velocity");

    state = {};
    state.position = {-10, 50, 1};
    state.previous_position = {-20, 40, 2};
    state.velocity = {30, -80, 5};
    state.clamp = true;
    state.bounds = {{0, 0, 0}, {10, 10, 0}};
    state.inherit_parent_position = true;
    const enemy::Vec3 parent{100, 200, 300};
    check(enemy::integrate_position(state, 1, nullptr, false) == enemy::Status::requires_parent &&
              state.position.x == -10,
          "unresolved parent was replaced by an invented origin");
    check(enemy::integrate_position(state, 1, &parent) == enemy::Status::advanced &&
              state.position.x == 10 && state.position.y == 0 && state.position.z == 6 &&
              state.previous_position.x == 0 && state.previous_position.y == 10 &&
              state.last_displacement.x == 20 && state.last_displacement.y == -30 &&
              state.world_position.x == 110 && state.world_position.y == 200 &&
              state.world_position.z == 0 && state.position_offset.z == 300,
          "manager phase reordered clamping, lagged displacement or parent offset publication");
    state.skip_integration = true;
    state.position.x = -99;
    check(enemy::integrate_position(state, 1, nullptr, false) == enemy::Status::advanced &&
              state.position.x == -99 && state.world_position.x == 1,
          "skip movement unexpectedly clamped, inherited or skipped world publication");
    state.mode = static_cast<enemy::Mode>(7);
    check(enemy::update_velocity(state, 1, false) == enemy::Status::invalid_state,
          "unknown movement mode did not fail closed");
    std::cout << "Enemy movement phases, interpolation, clocks and parent ownership: passed\n";
}
