#include <cstdlib>
#include <iostream>
#include <th08/emitter.hpp>
namespace vm = th08::emitter;
void check(bool valid, const char *why) {
    if (!valid) {
        std::cerr << why << '\n';
        std::exit(1);
    }
}
int main() {
    vm::Program program;
    vm::Workspace workspace;
    program.code = {{0, 63, 0, 8, 255, 76, 0, {}}};
    check(vm::run(program, workspace, 8).status == vm::Status::unsupported,
          "movement silently ignored");
    program.code = {{0, 6, 3, 8, 255, 76, 0, {10000, 10001}}};
    check(vm::run(program, workspace, 8).status == vm::Status::missing_context,
          "uninitialized register became zero");
    program.code = {{0, 4, 0, 8, 255, 76, 0, {0, 0}}};
    check(vm::run(program, workspace, 8, 4, 20).status == vm::Status::instruction_limit,
          "loop escaped instruction budget");
    program.code = {{0, 2, 0, 4, 255, 76, 0, {3}}, {0, 53, 0, 0, 255, 92, 0, {}}};
    const auto wait = vm::run(program, workspace, 8, 10);
    check(wait.status == vm::Status::returned && wait.tick == 3, "secondary wait timing mismatch");
    program.code = {{0, 63, 0, 8, 1, 76, 0, {}}, {0, 53, 0, 0, 8, 96, 0, {}}};
    check(vm::run(program, workspace, 8).status == vm::Status::returned,
          "masked instruction executed");
    check(vm::run(program, workspace, 0).status == vm::Status::invalid, "empty mask accepted");
    program.code = {{0, 6, 1, 8, 255, 76, 0, {10036, 2}},
                    {0, 5, 4, 12, 255, 96, 1, {0, 0, 10036}},
                    {0, 53, 0, 0, 255, 120, 0, {}}};
    check(vm::run(program, workspace, 8).status == vm::Status::returned,
          "explicit EXTRA_I0 initialization was rejected");
    std::cout << "emitter context, unsupported semantics, budgets, wait and masks: passed\n";
}
