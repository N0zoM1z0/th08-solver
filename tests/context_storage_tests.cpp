#include <cmath>
#include <iostream>
#include <stdexcept>
#include <th08/emitter.hpp>
#include <utility>

namespace vm = th08::emitter;
namespace {
void check(bool valid, const char *why) {
    if (!valid)
        throw std::runtime_error(why);
}
// Independent selector-domain classification, not the implementation's table.
bool is_context(std::size_t slot) {
    return slot < 8 || (slot >= 16 && slot < 24) || (slot >= 36 && slot < 40) ||
           (slot >= 53 && slot < 61) || slot == 94 || slot == 95;
}
void snapshots() {
    vm::ScalarStorage caller, callee;
    for (std::size_t slot = 0; slot < 101; ++slot) {
        caller.registers[slot] = double(slot) + .25;
        caller.initialized[slot] = (slot % 3 == 0);
        callee.registers[slot] = -double(slot) - .5;
        callee.initialized[slot] = !caller.initialized[slot];
    }
    caller.registers[94] = -0.0;
    const auto expected_caller = caller;
    const auto expected_callee = callee;
    const auto snapshot = vm::capture_context(caller);
    // Verify source field order independently, including the non-monotonic
    // extra-float selectors before the call-parameter region.
    std::size_t word = 0;
    for (const auto &range :
         {std::pair<unsigned, unsigned>{0, 8}, {16, 24}, {36, 40}, {94, 96}, {53, 57}, {57, 61}})
        for (auto slot = range.first; slot < range.second; ++slot, ++word)
            check(snapshot.values[word] == caller.registers[slot] &&
                      snapshot.initialized[word] == caller.initialized[slot],
                  "snapshot does not follow source scalar field order");
    caller = {}; // The snapshot owns its data, independent of the original storage.
    vm::restore_context(snapshot, callee);
    for (std::size_t slot = 0; slot < 101; ++slot) {
        const auto &expected = is_context(slot) ? expected_caller : expected_callee;
        check(callee.registers[slot] == expected.registers[slot], "wrong ownership on restore");
        check(callee.initialized[slot] == expected.initialized[slot], "lost unknown state");
    }
    check(std::signbit(callee.registers[94]), "signed zero lost in context snapshot");
    // Restore into the same source is also well-defined; no alias-dependent memcpy.
    vm::restore_context(vm::capture_context(callee), callee);
    check(std::signbit(callee.registers[94]), "self-restore lost signed zero");
}
void template_zeros() {
    vm::ScalarStorage storage;
    for (std::size_t slot = 0; slot < 101; ++slot) {
        storage.registers[slot] = double(slot) + 1;
        storage.initialized[slot] = (slot % 2 == 0);
    }
    const auto before = storage;
    vm::initialize_spawn_scalars(storage);
    unsigned initialized = 0;
    for (std::size_t slot = 0; slot < 101; ++slot) {
        const bool template_owned = is_context(slot) || (slot >= 8 && slot < 32);
        if (template_owned) {
            ++initialized;
            check(storage.registers[slot] == 0 && !std::signbit(storage.registers[slot]) &&
                      storage.initialized[slot],
                  "template scalar is not a known positive zero");
        } else {
            check(storage.registers[slot] == before.registers[slot] &&
                      storage.initialized[slot] == before.initialized[slot],
                  "spawn fabricated an external scalar");
        }
    }
    check(initialized == 46, "template must own thirty context and sixteen entity scalars");
    vm::Workspace workspace;
    vm::Program program;
    auto execution = vm::begin(program, workspace, 1);
    (void)execution;
    vm::initialize_spawn_scalars(workspace);
    for (std::size_t slot = 61; slot < 69; ++slot)
        check(!workspace.initialized[slot], "fresh spawn invented global call parameters");
}
void nested_call_ownership() {
    vm::Module module;
    module.subs.resize(3);
    const vm::Operation returned{0, 53, 0, 0, 255, 0, 0, {}};
    module.subs[0].code = {{0, 52, 0, 4, 255, 0, 0, {1}}, returned};
    module.subs[1].code = {{0, 52, 0, 4, 255, 0, 0, {2}}, returned};
    module.subs[2].code = {{0, 80, 0, 4, 255, 0, 0, {0}}, returned};
    vm::Workspace storage;
    auto execution = vm::begin(module, 0, storage, 1);
    for (std::size_t slot = 0; slot < 101; ++slot) {
        storage.registers[slot] = double(slot) + .5;
        storage.initialized[slot] = slot % 2 == 0;
    }
    const vm::ScalarStorage caller = storage;
    auto result = vm::advance(execution, storage, nullptr, 100, vm::Effects::yield_to_world);
    check(result.status == vm::Status::external_effect && execution.depth == 2,
          "nested-call fixture did not reach its world boundary");
    // A synthetic external owner mutates every slot; the two ordinary returns
    // must restore all context slots and preserve every non-context slot.
    for (std::size_t slot = 0; slot < 101; ++slot) {
        storage.registers[slot] = -double(slot) - .25;
        storage.initialized[slot] = !caller.initialized[slot];
    }
    const vm::ScalarStorage after_effect = storage;
    check(vm::acknowledge_effect(execution, result.executed), "fixture effect ack failed");
    result = vm::advance(execution, storage, nullptr, 100, vm::Effects::yield_to_world);
    check(result.status == vm::Status::returned && execution.depth == 0,
          "nested calls did not unwind");
    for (std::size_t slot = 0; slot < 101; ++slot) {
        const auto &expected = is_context(slot) ? caller : after_effect;
        check(storage.registers[slot] == expected.registers[slot] &&
                  storage.initialized[slot] == expected.initialized[slot],
              "compact call frames changed context/entity/global ownership");
    }
}
} // namespace
int main() try {
    snapshots();
    template_zeros();
    nested_call_ownership();
    struct PreviousCallFrame {
        const vm::Program *program;
        std::uint32_t pc;
        std::int64_t time, wait;
        std::array<double, 101> registers;
        std::array<bool, 101> initialized;
    };
    static_assert(sizeof(vm::CallFrame) < sizeof(PreviousCallFrame) / 2,
                  "context-only stack must remove redundant scalar storage");
    std::cout << "context scalars=30 template scalars=46 call_frame_bytes=" << sizeof(vm::CallFrame)
              << " previous_call_frame_bytes=" << sizeof(PreviousCallFrame)
              << " workspace_bytes=" << sizeof(vm::Workspace) << '\n';
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
