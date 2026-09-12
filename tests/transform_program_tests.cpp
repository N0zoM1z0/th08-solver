#include <cstdlib>
#include <iostream>
#include <th08/transform_program.hpp>

namespace transform = th08::bullet::transform;
using th08::bullet::Status;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
void boundary_tests() {
    transform::Program empty;
    transform::State state;
    state.flight.x = -8;
    state.active_flags = transform::bounce_all;
    state.bounce_speed = 3;
    state.bounce_limit = 2;
    check(transform::step(empty, state, 1, false).status == Status::unsupported &&
              state.bounce_count == 0,
          "bounce invented an unresolved sprite size");
    state.sprite_width = state.sprite_height = 16;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.bounce_count == 0,
          "bounce ignored inclusive sprite contact with playfield edge");
    state.flight.x = -9;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.bounce_count == 1 && state.flight.angle == -th08::kinematics::pi &&
              state.flight.speed == 3 && state.flight.x == -9,
          "bounce threshold, reflection, speed or position changed");
    state.active_flags = transform::bounce_except_bottom;
    state.flight.x = 100;
    state.flight.y = 457;
    state.flight.angle = 1;
    state.flight.speed = 1;
    state.bounce_count = 0;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.bounce_count == 1 && state.flight.angle == 1 && state.flight.speed == 3,
          "excluded bottom edge skipped the source speed/count side effects");

    state = {};
    state.active_flags = transform::wrap_x | transform::wrap_y;
    state.wrap_timer = 1;
    state.flight.x = -1;
    state.flight.y = -2;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.flight.x == 383 && state.flight.y == 446 && state.wrap_timer == 0 &&
              state.active_flags == transform::wrap_x,
          "two-axis wrap lost shared countdown or wrapped after the expiry check");
    state.flight.x = 384;
    state.flight.y = 448;
    state.active_flags = transform::wrap_x | transform::wrap_y;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.flight.x == 384 && state.flight.y == 448 && state.active_flags == 0,
          "exact far edge wrapped or expired axes stayed active");
    state.active_flags = transform::wrap_x;
    state.flight.x = 900;
    check(transform::step(empty, state, 1, false).status == Status::advanced &&
              state.flight.x == 516 && state.active_flags == 0,
          "wrap used modulo instead of one source translation");

    for (auto &record : empty.records)
        record = {0, 0, 4, 0, transform::sound, 1};
    state = {};
    state.enabled_flags = 0xffffffffU;
    state.active_flags =
        transform::relative | transform::absolute | transform::aimed | transform::bounce_all;
    state.flight.x = 500;
    state.turn.interval = 0;
    state.turn.repeats = 1;
    state.turn.speed = 2;
    state.transform_sound = 17;
    state.sprite_width = state.sprite_height = 16;
    state.bounce_speed = 3;
    state.bounce_limit = 1;
    const auto result = transform::step(empty, state, 1, false, 0);
    check(result.status == Status::advanced && result.sound_count == 22 &&
              result.sounds[21].id == 17 && !result.sounds[21].positioned &&
              state.active_flags == 0 && state.bounce_count == 1,
          "maximum overlapping turn/bounce sound trace exceeded its bound or lost source order");
}
int main() {
    boundary_tests();
    transform::Program program;
    transform::State state;
    state.flight.speed = 2;
    state.enabled_flags = 0xffffffffU;
    program.records[0] = {0, 0, 10, 0, transform::cull_delay, 0};
    program.records[1] = {1, -999, 2, 1, transform::relative, 0};
    program.records[2] = {0, 0, 2, 0, transform::wait, 0};
    auto result = transform::advance_program(program, state, 1);
    check(result.status == Status::advanced && state.pc == 2 && state.offscreen_cull_delay == 10 &&
              state.turn.timer == 0 && state.turn.speed == 2 &&
              state.active_flags == transform::relative,
          "birth installation did not stop at the first timed transform");
    for (int frame = 0; frame < 3; ++frame)
        check(transform::step(program, state, 1, false).status == Status::advanced && state.pc == 2,
              "blocked wait installed before the existing turn finished");
    check(state.active_flags == 0 && state.turn.completed == 1,
          "turn completion did not release the program barrier");
    check(transform::step(program, state, .5f, false).status == Status::advanced && state.pc == 3 &&
              state.wait_timer == 1 && state.wait_subframe == .5f,
          "new wait omitted same-frame fractional decrement");
    check(transform::step(program, state, 1, true).status == Status::advanced &&
              state.wait_timer == -1 && state.wait_subframe == 0 &&
              state.active_flags == transform::wait,
          "extra timer step or wait expiry frame changed");
    check(transform::step(program, state, 1, false).status == Status::advanced &&
              state.active_flags == 0,
          "expired wait failed to clear on its next call");
    state.pc = 0;
    state.active_flags = transform::wait;
    state.enabled_flags = transform::wait;
    check(transform::advance_program(program, state, 1).status == Status::advanced && state.pc == 0,
          "disabled record skipped before its active-effect gate");
    program.records[0].allow_while_active = 1;
    check(transform::advance_program(program, state, 1).status == Status::advanced && state.pc == 1,
          "allow-while-active failed to skip a disabled record");

    state = {};
    state.enabled_flags = 0xffffffffU;
    state.active_flags = transform::relative | transform::absolute | transform::aimed;
    state.turn.interval = 0;
    state.turn.repeats = 1;
    state.turn.speed = 2;
    state.transform_sound = 17;
    for (auto &record : program.records)
        record = {0, 0, 4, 0, transform::sound, 1};
    result = transform::step(program, state, 1, false, 0);
    check(result.status == Status::advanced && result.sound_count == 21 && state.pc == 18 &&
              state.turn.completed == 3 && state.turn.timer == 1 && state.active_flags == 0,
          "shared direction state, source order or maximum sound trace changed");
    for (unsigned i = 0; i < result.sound_count; ++i)
        check(result.sounds[i].id == (i < 18 ? 4 : 17) && result.sounds[i].positioned == (i < 18),
              "transform sound order changed");

    state = {};
    state.enabled_flags = 0xffffffffU;
    program = {};
    program.records[0] = {0, 0, 4, 0, transform::sound, 0};
    program.records[1] = {0, 2, 0, 1, transform::aimed, 0};
    result = transform::step(program, state, 1, false);
    check(result.status == Status::requires_target && result.sound_count == 0 && state.pc == 0 &&
              state.active_flags == 0,
          "missing target leaked program installations or sound events");
    program.records[1].kind = 0x1000000;
    result = transform::advance_program(program, state, 1);
    check(result.status == Status::unsupported && result.pc == 1 && state.pc == 0 &&
              result.sound_count == 0,
          "unmodeled child pattern was silently accepted or partially applied");
    program.records[1].kind = transform::despawn;
    result = transform::step(program, state, 1, false);
    check(result.status == Status::advanced && state.despawning && state.pc == 2 &&
              result.sound_count == 1,
          "despawn request discarded earlier immediate effects");
    check(transform::step(program, state, 1, false).status == Status::unsupported,
          "fired transform runner entered unmodeled despawn animation");
    std::cout << "Transform barriers, immediate records, shared turns, sounds and failure "
                 "atomicity: passed\n";
}
