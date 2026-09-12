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
int main() {
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
