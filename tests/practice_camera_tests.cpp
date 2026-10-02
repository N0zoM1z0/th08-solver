#include "test_support.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <th08/practice_camera.hpp>

namespace c = th08::practice::camera;
namespace r = th08::resources;
using th08::test::check;
namespace {
bool same(float a, float b) {
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}
bool same(c::Vec3 a, c::Vec3 b) {
    return same(a.x, b.x) && same(a.y, b.y) && same(a.z, b.z);
}
bool same(const c::Interpolation &a, const c::Interpolation &b) {
    return same(a.current, b.current) && same(a.start, b.start) && same(a.target, b.target) &&
           a.duration == b.duration && a.timer == b.timer && a.mode == b.mode;
}
bool same(const c::State &a, const c::State &b) {
    return same(a.position, b.position) && same(a.look_at_offset, b.look_at_offset) &&
           same(a.up, b.up) && same(a.forward, b.forward) && same(a.fov, b.fov) && a.pc == b.pc &&
           a.script_time == b.script_time;
}
void initial_and_certificate() {
    const c::State state;
    check(same(state.position.current, {0, 0, 1000}) &&
              same(state.position.current, state.position.start) &&
              same(state.position.start, state.position.target) &&
              same(state.look_at_offset.current, {}) && same(state.up.current, {0, 1, 0}) &&
              same(state.up.start, state.up.current) && same(state.up.target, state.up.current) &&
              state.position.duration == 0 && state.position.timer == 0 && state.pc == 0 &&
              state.script_time == 0 && same(state.fov, 0.5235987901687622f),
          "added-callback camera initialization changed");
    bool rejected = false;
    try {
        const r::Bytes unrelated{0, 1, 2, 3};
        const c::Program program(r::view(unrelated));
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    check(rejected, "uncertified STD became an executable camera program");
}
c::Program load(const char *path) {
    r::Archive archive(r::read_file(path));
    for (std::size_t i = 0; i < archive.entries().size(); ++i)
        if (archive.entries()[i].name == "stage1_s.std") {
            const auto bytes = archive.decode(i);
            return c::Program(r::view(bytes));
        }
    throw std::runtime_error("stage1_s.std missing");
}
void dat(const char *path) {
    // All decoded bytes are already destroyed: the program owns its source data.
    const auto program = load(path);
    c::State state, initial = state;
    check(program.advance(state, std::nullopt, 1.0f).status == c::Status::missing_context &&
              same(state, initial) &&
              program.advance(state, true, 1.0f).status == c::Status::frozen &&
              same(state, initial),
          "unknown freeze or frozen startup advanced the camera");
    const auto first = program.advance(state, false, 1.0f);
    const float length = std::sqrt(29.8f * 29.8f + 500.0f * 500.0f + 460.0f * 460.0f);
    check(first.status == c::Status::advanced && first.pc == 12 && first.offset == 3748 &&
              first.opcode == 8 && first.script_time == 1 &&
              same(state.position.current, {0, 3966.5f, -400}) &&
              same(state.position.start, {0, 3966, -400}) &&
              same(state.position.target, {0, 4478, -400}) &&
              same(state.look_at_offset.current, {29.8f, 500, 460}) &&
              same(state.forward, {29.8f / length, 500.0f / length, 460.0f / length}) &&
              state.position.timer == 1 && state.position.duration == 1024 &&
              state.look_at_offset.timer == 1 && state.look_at_offset.duration == 300,
          "time-zero setter/interpolation/source-PC order changed");
    const auto checkpoint = state;
    for (unsigned i = 0; i < 4; ++i)
        check(program.advance(state, true, 1.0f).status == c::Status::frozen &&
                  same(state, checkpoint),
              "freeze did not preserve a running interpolation");
    auto branch = state;
    check(program.advance(branch, false, 1.0f).status == c::Status::advanced &&
              branch.position.current.y == 3967 && same(state, checkpoint),
          "resumed camera branch mutated the original checkpoint");
    auto invalid = state;
    check(program.advance(invalid, false, 0.5f).status == c::Status::unsupported_timing &&
              same(invalid, state) &&
              program.advance(invalid, true, 0.5f).status == c::Status::frozen &&
              same(invalid, state),
          "nonunit timing advanced the projection or bypassed the early freeze gate");
    invalid.position.mode = 1;
    auto before = invalid;
    check(program.advance(invalid, false, 1.0f).status == c::Status::invalid_state &&
              same(invalid, before),
          "unsupported interpolation mode executed or partially committed");
    invalid = state;
    invalid.look_at_offset.target.x = std::numeric_limits<float>::quiet_NaN();
    before = invalid;
    check(program.advance(invalid, false, 1.0f).status == c::Status::invalid_state &&
              same(invalid, before),
          "invalid camera state partially committed");
    while (state.script_time < 512)
        check(program.advance(state, false, 1.0f).status == c::Status::advanced,
              "camera stopped before its second look interpolation");
    check(same(state.position.current, {0, 4222, -400}) &&
              same(state.look_at_offset.current, {-30, 500, 460}) &&
              state.look_at_offset.timer == 300 && state.look_at_offset.duration == 0 &&
              program.advance(state, false, 1.0f).status == c::Status::advanced && state.pc == 14 &&
              same(state.look_at_offset.current, {-29.8f, 500, 460}) &&
              state.look_at_offset.timer == 1,
          "time512 did not restart from the previous target");
    while (state.script_time < 1024)
        check(program.advance(state, false, 1.0f).status == c::Status::advanced,
              "camera stopped before the world-effect boundary");
    check(same(state.position.current, {0, 4478, -400}) &&
              same(state.look_at_offset.current, {30, 500, 460}) &&
              state.position.duration == 1024 && state.position.timer == 1024,
          "interpolation expiry cleared its duration one update too soon");
    before = state;
    for (unsigned i = 0; i < 2; ++i) {
        const auto blocked = program.advance(state, false, 1.0f);
        check(blocked.status == c::Status::needs_world_effect && blocked.pc == 14 &&
                  blocked.offset == 3788 && blocked.opcode == 4 && blocked.script_time == 1024 &&
                  same(state, before),
              "STD loop or retry bypassed the world-effect origin boundary");
    }
}
} // namespace
int main(int argc, char **argv) {
    initial_and_certificate();
    if (argc > 1)
        dat(argv[1]);
    std::cout << "practice camera checks passed" << (argc > 1 ? " with DAT\n" : "\n");
}
