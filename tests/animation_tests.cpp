#include <cstdlib>
#include <iostream>
#include <th08/animation.hpp>

namespace res = th08::resources;
namespace anm = th08::animation;
void check(bool valid, const char *message) {
    if (!valid) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
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
