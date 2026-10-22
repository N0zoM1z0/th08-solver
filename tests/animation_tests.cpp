#include "test_support.hpp"
#include <iostream>
#include <th08/animation.hpp>

namespace res = th08::resources;
namespace anm = th08::animation;
using th08::test::check;
int main() {
    anm::TerminalClock clock;
    check(anm::remaining_terminal_calls(clock) == 0, "unknown current cursor invented timing");
    clock.context_supported = true;
    clock.cursor = anm::Cursor::present;
    clock.size = 8;
    clock.scheduled_time = 9;
    clock.current_time = 1;
    for (int opcode : {-1, 1, 2}) {
        clock.opcode = opcode;
        check(anm::remaining_terminal_calls(clock) == 9,
              "current terminal clock has a one-update activation error");
    }
    clock.current_time = 10;
    check(anm::remaining_terminal_calls(clock) == 1, "past terminal failed its next-call boundary");
    for (int refusal = 0; refusal < 6; ++refusal) {
        auto unsupported = clock;
        if (refusal == 0)
            unsupported.opcode = 4;
        if (refusal == 1)
            unsupported.mask = 1;
        if (refusal == 2)
            unsupported.size = 12;
        if (refusal == 3)
            unsupported.blocked = true;
        if (refusal == 4)
            unsupported.current_time = -1;
        if (refusal == 5)
            unsupported.subframe = .5f;
        check(anm::remaining_terminal_calls(unsupported) == 0,
              "unsupported current animation state was assigned a completion time");
    }
    clock.cursor = anm::Cursor::null_instruction;
    clock.blocked = true;
    check(anm::remaining_terminal_calls(clock) == 1, "null cursor tested blocking flags first");
    clock.context_supported = false;
    check(anm::remaining_terminal_calls(clock) == 0, "unsupported context acquired a unit clock");
    res::Bytes bytes(40);
    bytes[8] = 146;
    res::Anm program;
    program.scripts.push_back({0, 21, 0, 0, 3});
    program.instructions = {{0, 3, 0, 12, 0}, {12, 8, 0, 12, 0}, {24, 1, 10, 8, 0}};
    auto timing = anm::certify_timing(res::view(bytes), program, 0);
    check(timing.status == anm::Status::certified && timing.sprite == 146 &&
              timing.completion_time == 10 && timing.calls_after_template == 10 &&
              timing.hides_on_completion,
          "template initialization frame was counted twice");
    program.instructions[1].time = 20;
    timing = anm::certify_timing(res::view(bytes), program, 0);
    check(timing.completion_time == 20 && timing.calls_after_template == 20,
          "backward timestamp rewound ANM time");
    program.instructions[1].time = 0;
    program.instructions[2].time = 0;
    program.instructions[2].opcode = 2;
    timing = anm::certify_timing(res::view(bytes), program, 0);
    check(timing.completion_time == 0 && timing.calls_after_template == 1 &&
              !timing.hides_on_completion,
          "static template completion contract mismatch");
    program.instructions[1].opcode = 4;
    check(anm::certify_timing(res::view(bytes), program, 0).status == anm::Status::unsupported,
          "unknown or branching animation certified");
    program.instructions[1].opcode = 3;
    check(anm::certify_timing(res::view(bytes), program, 0).status == anm::Status::unsupported,
          "sprite replacement certified as immutable");
    program.instructions[1].opcode = 8;
    program.instructions[1].mask = 1;
    check(anm::certify_timing(res::view(bytes), program, 0).status == anm::Status::unsupported,
          "variable-dependent animation certified");
    check(anm::certify_timing(res::view(bytes), program, 1).status == anm::Status::invalid,
          "missing animation accepted");
    program.instructions[1].mask = 0;
    program.instructions[2].offset = 40;
    check(anm::certify_timing(res::view(bytes), program, 0).status == anm::Status::invalid,
          "out-of-bounds animation terminal accepted");
    std::cout << "Animation timing projection and refusal boundaries: passed\n";
}
