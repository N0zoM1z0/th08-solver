#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <th08/timeline.hpp>

namespace tl = th08::timeline;
namespace res = th08::resources;
namespace {
void check(bool valid, const char *why) {
    if (!valid)
        throw std::runtime_error(why);
}
struct Builder {
    res::Bytes bytes;
    res::Ecl ecl;
    void word(std::uint32_t value) {
        for (unsigned shift = 0; shift < 32; shift += 8)
            bytes.push_back(std::uint8_t(value >> shift));
    }
    void append(std::uint16_t opcode, std::int32_t time,
                std::initializer_list<std::int32_t> payload = {}, std::uint8_t mask = 255) {
        const auto offset = std::uint32_t(bytes.size());
        const auto size = std::uint8_t(8 + payload.size() * 4);
        word(std::uint32_t(time));
        word(opcode | (std::uint32_t(size) << 16) | (std::uint32_t(mask) << 24));
        for (auto value : payload)
            word(std::uint32_t(value));
        ecl.timeline_instructions.push_back({offset, time, opcode, size, mask});
    }
    tl::Program finish() {
        const auto terminal = std::uint32_t(bytes.size());
        word(0xffffffff);
        word(0);
        ecl.timelines.push_back({0, terminal, 0, std::uint32_t(ecl.timeline_instructions.size())});
        return {res::view(bytes), ecl, 0};
    }
};
bool same(const tl::State &a, const tl::State &b) {
    return a.pc == b.pc && a.effect_token == b.effect_token &&
           a.pending_effect == b.pending_effect && a.time.current == b.time.current &&
           a.time.previous == b.time.previous && a.time.fraction == b.time.fraction;
}
void gates_and_effects() {
    Builder b;
    b.append(2, 0, {7, 0, 0, 0, 10, -1, 100});
    b.append(15, 0, {8, 0, 0, 20, -1, 200});
    b.append(7, 0);
    b.append(16, 0);
    const auto program = b.finish();
    tl::State state;
    tl::Context world;
    check(tl::advance(program, state, world, 1).status == tl::Status::requires_context &&
              state.pc == 0,
          "unknown spawn gate must stop without advancing");
    world.gui_boss_present = tl::KnownBool::yes;
    auto result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::external_effect && result.pc == 1 &&
              result.effect_token == 1 && state.time.current == 0,
          "known boss suppresses random spawn without reading unknown suppression");
    const auto pending = state;
    world.gui_boss_present = tl::KnownBool::unknown;
    result = tl::advance(program, state, world, 0, -1);
    check(result.status == tl::Status::external_effect && same(state, pending),
          "pending effect is stable and must not reevaluate gates or tick");
    check(!tl::acknowledge_effect(program, state, 9) && same(state, pending), "stale effect token");
    check(tl::pending_operation(program, state)->opcode == 15, "complete pending instruction");
    check(tl::acknowledge_effect(program, state, 1) && !tl::acknowledge_effect(program, state, 1),
          "effect acknowledgment exactly once");
    const auto before_unknown = state;
    check(tl::advance(program, state, world, 1).status == tl::Status::requires_context &&
              same(before_unknown, state),
          "message gate must not invent dialogue state");
    world.message_waiting = tl::KnownBool::yes;
    result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::frame_complete && state.pc == 2 && state.time.current == 0 &&
              state.time.previous == -1,
          "wait decrement then tick");
    world.message_waiting = tl::KnownBool::no;
    result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::external_effect && result.pc == 3 &&
              result.effect_token == 2,
          "same-frame resume after resolved message wait");
    check(tl::acknowledge_effect(program, state, result.effect_token),
          "menu effect acknowledgment");
    result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::frame_complete && result.at_end && state.time.current == 1,
          "sentinel frame must tick");
    result = tl::advance(program, state, world, 1);
    check(result.at_end && state.time.current == 2, "sentinel is not a frozen completion latch");
}
void event_broadcast_and_rollback() {
    Builder b;
    b.append(14, 0, {9});
    b.append(13, 0, {9});
    b.append(13, 0, {7});
    auto program = b.finish();
    tl::State state;
    tl::Context world;
    world.events_known = true;
    world.events = {-1, 3, -2, 7};
    auto result = tl::advance(program, state, world, 1, 1, false, 1);
    check(result.status == tl::Status::instruction_limit && state.pc == 0 &&
              world.events == std::array<std::int32_t, 4>{-1, 3, -2, 7},
          "failed call rolls back event publication with timeline state");
    // A pending external effect makes the intermediate broadcast observable.
    program.code[1].opcode = 9;
    result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::external_effect &&
              world.events == std::array<std::int32_t, 4>{9, 3, 9, 7},
          "event publication fills all negative slots, not just the first");
    auto branch = state;
    auto branch_world = world;
    check(tl::acknowledge_effect(program, state, result.effect_token), "event boundary ack");
    result = tl::advance(program, state, world, 1);
    check(result.at_end && world.events == std::array<std::int32_t, 4>{9, 3, 9, -1},
          "event wait consumes matching slots");
    check(branch.pending_effect && branch.pc == 1 && branch_world.events[3] == 7,
          "copied runtime and context fork independently");
    Builder matches;
    matches.append(13, 0, {9});
    auto consume = matches.finish();
    tl::State consumer;
    result = tl::advance(consume, consumer, branch_world, 1);
    check(result.at_end && branch_world.events == std::array<std::int32_t, 4>{-1, 3, -1, 7},
          "event consumption clears every matching slot");
}
void timing_and_validation() {
    Builder b;
    b.append(10, 0, {0});
    b.append(16, 1);
    auto program = b.finish();
    tl::State state;
    tl::Context world;
    world.boss_active[0] = tl::KnownBool::yes;
    for (unsigned i = 0; i < 20; ++i) {
        auto result = tl::advance(program, state, world, 1, .5f);
        check(result.status == tl::Status::frame_complete && state.pc == 0 &&
                  state.time.current == 0 && state.time.fraction == 0 && state.time.previous == -1,
              "fractional boss wait retains decrement-and-tick clock fields");
    }
    auto result = tl::advance(program, state, world, 1, .5f, true);
    check(result.status == tl::Status::frame_complete && state.time.current == -1 &&
              state.time.previous == -2 && state.time.fraction == 0,
          "extra timer step affects wait decrement, not tail tick");
    const auto before = state;
    check(tl::advance(program, state, world, 1, NAN).status == tl::Status::invalid &&
              same(state, before),
          "non-finite clock rate is atomic");
    state.time.current = 2;
    world = {};
    result = tl::advance(program, state, world, 1);
    check(result.at_end && state.time.current == 3,
          "past instructions are skipped without execution");
    state = {};
    program.code[0].mask = 2;
    program.code[0].opcode = 250;
    result = tl::advance(program, state, world, 1);
    check(result.status == tl::Status::frame_complete && state.pc == 1,
          "masked-out unknown instruction never reads context");
    state = {};
    result = tl::advance(program, state, world, 2);
    check(result.status == tl::Status::unsupported && state.pc == 0,
          "selected unknown opcode is not a silent NOP");
    program.code[0].opcode = 10;
    program.code[0].words[0] = 8;
    check(tl::advance(program, state, world, 2).status == tl::Status::invalid,
          "boss gate slot is bounds checked");
    state.pc = std::uint32_t(program.code.size());
    check(tl::advance(program, state, world, 1).status == tl::Status::invalid, "invalid PC");
    state = {};
    state.pending_effect = true;
    state.effect_token = 1;
    check(tl::advance(program, state, world, 1).status == tl::Status::invalid &&
              !tl::acknowledge_effect(program, state, 1),
          "forged pending state cannot skip an unresolved control gate");
    Builder masks;
    masks.append(16, 0, {}, 1);
    const auto masked = masks.finish();
    state = {};
    check(tl::advance(masked, state, world, 3).status == tl::Status::external_effect,
          "timeline masks use any-bit intersection, not enemy ECL containment");
}
void ownership() {
    Builder b;
    b.append(11, 0, {42, 123, 456, 20, 2, 3, 100});
    auto program = b.finish();
    auto copied = program;
    auto moved = std::move(copied);
    b.bytes.assign(b.bytes.size(), 0);
    check(res::i32(moved.payload(0), 0) == 42 && res::i32(moved.payload(0), 24) == 100,
          "compiled timeline owns every byte after source destruction and copy/move");
    check(moved.payload(1).size == 0, "sentinel owns no fabricated payload");
    moved.code[0].payload_offset = std::numeric_limits<std::uint32_t>::max();
    bool rejected = false;
    try {
        (void)moved.payload(0);
    } catch (const std::exception &) {
        rejected = true;
    }
    check(rejected, "payload accessor checks owned bounds");
    rejected = false;
    try {
        tl::Program invalid(res::view(b.bytes), b.ecl, 0);
    } catch (const std::exception &) {
        rejected = true;
    }
    check(rejected, "compiler rejects inconsistent parser metadata");
}
void dat(const char *path) {
    res::Archive archive(res::read_file(path));
    std::size_t timelines = 0, records = 0;
    bool practice_checked = false;
    for (std::size_t index = 0; index < archive.entries().size(); ++index) {
        const auto &entry = archive.entries()[index];
        if (entry.name.size() < 4 || entry.name.substr(entry.name.size() - 4) != ".ecl")
            continue;
        const auto bytes = archive.decode(index);
        const auto ecl = res::parse_ecl(res::view(bytes));
        for (std::size_t id = 0; id < ecl.timelines.size(); ++id) {
            const tl::Program program(res::view(bytes), ecl, id);
            ++timelines;
            for (std::uint32_t pc = 0; pc + 1 < program.code.size(); ++pc) {
                const auto payload = program.payload(pc);
                check(!payload.size ||
                          std::memcmp(payload.data, bytes.data() + program.code[pc].offset + 8,
                                      payload.size) == 0,
                      "DAT compiled payload identity");
                ++records;
            }
            if (entry.name != "ecldata1sp.ecl" || id != 0)
                continue;
            tl::State state;
            tl::Context world;
            world.gui_boss_present = tl::KnownBool::no;
            world.spawns_suppressed = tl::KnownBool::no;
            world.boss_active[0] = tl::KnownBool::no;
            unsigned effects = 0, holds = 0;
            for (unsigned calls = 0; calls < 100; ++calls) {
                auto result = tl::advance(program, state, world, 1);
                if (result.status == tl::Status::external_effect) {
                    const auto *op = tl::pending_operation(program, state);
                    const std::uint32_t offsets[] = {39684, 39716, 39760};
                    check(effects < 3 && op->offset == offsets[effects],
                          "practice timeline event order");
                    if (effects < 2) {
                        check(op->opcode == 0 && op->words[0] == (effects ? 42 : 0),
                              "practice timeline exact spawn targets");
                    } else
                        check(op->opcode == 16, "practice timeline retry handoff");
                    if (effects == 1)
                        world.boss_active[0] = tl::KnownBool::yes;
                    ++effects;
                    check(tl::acknowledge_effect(program, state, result.effect_token),
                          "practice handoff ack");
                    --calls; // Resume the same source frame after external work.
                    continue;
                }
                check(result.status == tl::Status::frame_complete,
                      "practice timeline resolved control");
                if (state.pc == 2) {
                    check(state.time.current == 30, "practice boss gate holds local time30");
                    if (++holds == 5)
                        world.boss_active[0] = tl::KnownBool::no;
                }
                if (result.at_end)
                    break;
            }
            check(effects == 3 && holds == 5 && state.time.current == 61,
                  "practice timeline complete handoff fixture, not a complete world");
            practice_checked = true;
        }
    }
    check(timelines == 32 && records == 2003 && practice_checked,
          "all native DAT timeline records");
    std::cout << "DAT timeline payloads: " << records << ", timelines: " << timelines << '\n';
}
} // namespace
int main(int argc, char **argv) try {
    check(argc <= 2, "usage: timeline_tests [th08.dat]");
    gates_and_effects();
    event_broadcast_and_rollback();
    timing_and_validation();
    ownership();
    if (argc == 2)
        dat(argv[1]);
    std::cout << "timeline tests passed\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
