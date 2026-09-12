#include <cstdlib>
#include <iostream>
#include <th08/animation_control.hpp>
namespace ac = th08::animation::control;
void check(bool ok, const char *why) {
    if (!ok) {
        std::cerr << why << '\n';
        std::exit(1);
    }
}
ac::Operation op(int opcode, int time = 0, std::initializer_list<std::int32_t> words = {}) {
    ac::Operation result;
    result.opcode = std::int16_t(opcode);
    result.time = std::int16_t(time);
    result.payload_size = std::uint16_t(words.size() * 4);
    std::copy(words.begin(), words.end(), result.words.begin());
    return result;
}
int main() {
    namespace res = th08::resources;
    auto compiled = [] {
        res::Bytes bytes(28, 0);
        bytes[8] = 7;
        res::Anm anm;
        anm.instructions = {{0, 3, 0, 12, 0}, {12, 2, 0, 8, 0}, {20, -1, -1, 0, 0}};
        anm.scripts = {{0, 0, 0, 0, 3}};
        return ac::Program(res::view(bytes), anm, 0);
    }();
    ac::State owned;
    check(ac::advance(compiled, owned).status == ac::Status::completed && owned.sprite == 7,
          "compiled ANM borrowed a destroyed resource");
    res::Bytes malformed(24, 0);
    malformed[8] = 9; // Jump into the payload instead of an instruction header.
    res::Anm invalid;
    invalid.instructions = {{0, 4, 0, 16, 0}, {16, -1, -1, 0, 0}};
    invalid.scripts = {{0, 0, 0, 0, 2}};
    bool rejected = false;
    try {
        ac::Program bad(res::view(malformed), invalid, 0);
    } catch (const std::exception &) {
        rejected = true;
    }
    check(rejected, "compiled ANM accepted a jump into an operand");
    ac::Program program;
    program.code = {op(3, 0, {4}), op(79, 0, {2}), op(3, 0, {9}), op(2), op(-1)};
    ac::State state;
    check(ac::advance(program, state).status == ac::Status::advanced && state.time.current == 0 &&
              state.wait.current == 2 && state.sprite == 4 && state.visible,
          "wait did not hold the installing frame");
    auto fork = state;
    check(ac::advance(program, state).status == ac::Status::advanced && state.wait.current == 1,
          "wait countdown changed");
    check(ac::advance(program, state).status == ac::Status::completed && state.sprite == 9 &&
              state.visible && !state.active && state.time.current == 0,
          "static completion hid the sprite or ticked the terminal frame");
    check(fork.wait.current == 2 && fork.sprite == 4, "runtime fork shared mutable state");
    state.frozen = true;
    check(ac::advance(program, state).status == ac::Status::completed,
          "freeze took precedence over null instruction");

    state = {};
    for (int call = 0; call < 3; ++call)
        check(ac::advance(program, state, .5f).status == ac::Status::advanced,
              "fractional wait finished early");
    check(ac::advance(program, state, .5f).status == ac::Status::completed,
          "fractional wait compared float age instead of the source integer clock");

    program.code = {op(3, 0, {4}),   op(23),         op(21, 5, {1}), op(3, 5, {8}), op(89),
                    op(21, 7, {-1}), op(28, 7, {2}), op(20),         op(-1)};
    program.labels = {{-1, 6}, {1, 3}};
    program.fallback = 6;
    state = {};
    ac::advance(program, state);
    check(state.stopped && !state.visible && state.pc == 1, "stop-hide advanced its PC");
    state.pending_interrupt = 1;
    state.frozen = true;
    ac::advance(program, state);
    check(state.pending_interrupt == 1 && state.sprite == 4, "freeze consumed an interrupt");
    state.frozen = false;
    check(ac::advance(program, state).status == ac::Status::advanced && state.sprite == 8 &&
              state.pc == 1 && state.time.current == 0 && state.stopped && !state.visible,
          "interrupt return did not restore the stopped instruction and clock");
    state.pending_interrupt = 99;
    ac::advance(program, state);
    check(state.pc == 7 && !state.visible && state.pending_interrupt == 0 && state.stopped,
          "fallback interrupt or one-bit visibility assignment changed");
    program.fallback = std::numeric_limits<std::uint32_t>::max();
    state.pending_interrupt = 99;
    ac::advance(program, state);
    check(!state.stopped && state.pc == 7 && state.time.current == 7,
          "missing interrupt failed to clear stop without advancing PC/time");
    ac::advance(program, state, 1, true);
    check(state.time.current == 6 && state.stopped,
          "extra decrement step was incorrectly applied to the frame-tail tick");

    program.code = {op(3, 0, {4}), op(59, 0, {10000, 4}), op(-1)};
    state = {};
    check(ac::advance(program, state).status == ac::Status::unsupported && state.sprite == -1 &&
              state.pc == 0,
          "unsupported RNG instruction leaked a partial frame");
    program.code = {op(4, 0, {0, 0}), op(-1)};
    check(ac::advance(program, state, 1, false, 17).status == ac::Status::instruction_limit &&
              state.pc == 0,
          "same-frame jump escaped the atomic instruction budget");
    program.code = {op(89), op(-1)};
    check(ac::advance(program, state).status == ac::Status::invalid,
          "interrupt return fabricated an uninitialized return address");
    program.code = {op(1, 3), op(-1)};
    for (int call = 0; call < 3; ++call)
        check(ac::advance(program, state).status == ac::Status::advanced,
              "future instruction ran too early");
    state.time.current = 9;
    check(ac::advance(program, state).status == ac::Status::completed && !state.visible,
          "ANM used ECL equality scheduling instead of less-than-or-equal");
    std::cout << "ANM lifecycle clocks, interrupts, forks and atomic failures: passed\n";
}
