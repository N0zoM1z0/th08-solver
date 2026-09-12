#include <cstdlib>
#include <cstring>
#include <iostream>
#include <th08/emitter.hpp>
namespace vm = th08::emitter;
void check(bool valid, const char *why) {
    if (!valid) {
        std::cerr << why << '\n';
        std::exit(1);
    }
}
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
vm::Operation op(int opcode, std::initializer_list<std::uint32_t> words, std::uint16_t flags = 0) {
    vm::Operation result{0, std::int16_t(opcode), flags, std::uint16_t(words.size() * 4), 255, 0, 0,
                         {}};
    std::copy(words.begin(), words.end(), result.words.begin());
    return result;
}
void scalar_tests(vm::Workspace &workspace) {
    vm::Program program;
    program.code = {op(6, {10000, 4}, 1),
                    op(30, {10000}, 1),
                    op(31, {10000}, 1),
                    op(32, {bits(10016.0f), bits(0.0f)}, 1),
                    op(33, {bits(10017.0f), bits(0.0f)}, 1),
                    op(39, {bits(10018.0f), bits(0.0f), bits(0.0f), bits(3.0f), bits(4.0f)}, 1),
                    op(34, {bits(10019.0f), bits(0.0f), bits(0.0f), bits(1.0f), bits(0.0f)}, 1),
                    op(38, {bits(10020.0f), bits(10021.0f), bits(0.0f), bits(2.0f)}, 3),
                    op(37, {bits(10019.0f)}, 1),
                    op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::returned &&
              workspace.registers[0] == 4 && workspace.registers[16] == 0 &&
              workspace.registers[17] == 1 && workspace.registers[18] == 5 &&
              workspace.registers[19] == 0 && workspace.registers[20] == 2 &&
              workspace.registers[21] == 0,
          "increment, trigonometry or geometric arithmetic mismatch");
    const int expected[] = {10, 4, 21, 2, 1};
    for (int opcode = 10; opcode <= 29; ++opcode) {
        const bool floating = (opcode >= 15 && opcode <= 19) || opcode >= 25;
        const auto dest = floating ? bits(10016.0f) : 10000U;
        const auto seven = floating ? bits(7.0f) : 7U;
        const auto three = floating ? bits(3.0f) : 3U;
        program.code = {op(floating ? 7 : 6, {dest, seven}, 1),
                        opcode < 20 ? op(opcode, {dest, three}, 1)
                                    : op(opcode, {dest, seven, three}, 1),
                        op(53, {})};
        check(vm::run(program, workspace, 8).status == vm::Status::returned,
              "arithmetic opcode rejected");
        const double value =
            floating && opcode % 5 == 3 ? double(7.0f / 3.0f) : expected[opcode % 5];
        check(workspace.registers[floating ? 16 : 0] == value, "arithmetic value mismatch");
    }
    program.code = {op(7, {bits(10016.0f), bits(-3.75f)}, 1), op(6, {10000, 10016}, 3),
                    op(7, {bits(10017.0f), bits(10016.5f)}, 3), op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::returned &&
              workspace.registers[0] == -3 && workspace.registers[17] == -3.75,
          "typed operand truncation mismatch");
    program.code = {op(6, {10000, 10079}, 3), op(7, {bits(10016.0f), bits(10098.5f)}, 3),
                    op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::returned &&
              workspace.registers[0] == 10079 && workspace.registers[16] == 10098.5,
          "raw selector default mismatch");
    program.code = {op(7, {bits(10000.0f), bits(1.0f)}, 1), op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::unsupported,
          "wrong typed lvalue silently changed an integer register");
    for (int opcode : {23, 24, 28, 29}) {
        const bool floating = opcode >= 25;
        program.code = {
            op(opcode, {floating ? bits(10016.0f) : 10000U, floating ? bits(1.0f) : 1U, 0}, 1),
            op(53, {})};
        check(vm::run(program, workspace, 8).status == vm::Status::invalid,
              "division by zero accepted");
    }
    program.code = {op(20, {10000, 0x7fffffffU, 1}, 1), op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::invalid,
          "unverified signed overflow accepted");
    for (int opcode = 40; opcode <= 51; ++opcode)
        for (int right : {2, 3, 4}) {
            const bool floating = (opcode & 1) != 0;
            const bool comparisons[] = {3 == right, 3 != right, 3 < right,
                                        3 <= right, 3 > right,  3 >= right};
            auto branch = op(opcode, {floating ? bits(3.0f) : 3U,
                                      floating ? bits(float(right)) : unsigned(right), 0, 0});
            branch.target = 2;
            program.code = {branch, op(63, {0, 0}), op(53, {})};
            check(vm::run(program, workspace, 8).status == (comparisons[(opcode - 40) / 2]
                                                                ? vm::Status::returned
                                                                : vm::Status::unsupported),
                  "conditional branch mismatch");
        }
}
void call_tests(vm::Workspace &workspace) {
    vm::Module module;
    module.subs.resize(2);
    module.subs[0].code = {op(6, {10000, 7}, 1), op(6, {10008, 1}, 1),     op(6, {10061, 9}, 1),
                           op(52, {1}),          op(6, {10001, 10008}, 3), op(53, {})};
    module.subs[1].code = {op(6, {10000, 10053}, 3), op(6, {10008, 10000}, 3),
                           op(6, {10061, 12}, 1), op(2, {3}), op(53, {})};
    const auto result = vm::run(module, 0, workspace, 8);
    check(result.status == vm::Status::returned && result.tick == 3,
          "call scheduling or caller clock restoration mismatch");
    check(workspace.registers[0] == 7 && workspace.registers[1] == 9 &&
              workspace.registers[8] == 9 && workspace.registers[61] == 12 &&
              !workspace.initialized[53],
          "call parameters, context restore or persistent entity/shared storage mismatch");
    check(vm::run(module.subs[0], workspace, 8).status == vm::Status::missing_context,
          "isolated call invented a module");
    module.subs[0].code = {op(52, {0})};
    check(vm::run(module, 0, workspace, 8).status == vm::Status::unsupported,
          "call stack overflow not stopped");
    check(vm::run(module, 2, workspace, 8).status == vm::Status::invalid,
          "invalid module entry accepted");
}
void random_tests(vm::Workspace &workspace) {
    using th08::random::Rng;
    vm::Program program;
    program.code = {op(6, {10000, 10032}, 3),
                    op(7, {bits(10016.0f), bits(10033.75f)}, 3),
                    op(6, {10001, 10034}, 3),
                    op(7, {bits(10017.0f), bits(10035.0f)}, 3),
                    op(7, {bits(10018.0f), bits(10082.0f)}, 3),
                    op(8, {10002, 13}, 1),
                    op(9, {bits(10019.0f), bits(2.5f)}, 1),
                    op(53, {})};
    check(vm::run(program, workspace, 8).status == vm::Status::missing_context,
          "random operands invented an initial seed");
    for (unsigned seed = 0; seed < 65536; ++seed) {
        Rng actual{std::uint16_t(seed)}, reference{std::uint16_t(seed)};
        const auto nonnegative = reference.next_u32() & 0x7fffffffU;
        const auto unit = reference.unit();
        const auto raw = reference.next_u32();
        std::int32_t signed_raw;
        std::memcpy(&signed_raw, &raw, sizeof(raw));
        const auto signed_unit = reference.signed_unit();
        const auto angle = reference.range_float(6.2831855f) - 3.1415927f;
        const int signed_integer = (reference.next_u16() & 1U ? 1 : -1) * 13;
        const float signed_float = (reference.next_u16() & 1U ? 1.0f : -1.0f) * 2.5f;
        check(vm::run(program, workspace, 8, 400, 100000, &actual).status == vm::Status::returned &&
                  workspace.registers[0] == nonnegative && workspace.registers[16] == unit &&
                  workspace.registers[1] == signed_raw && workspace.registers[17] == signed_unit &&
                  workspace.registers[18] == angle && workspace.registers[2] == signed_integer &&
                  workspace.registers[19] == signed_float && actual.seed() == reference.seed() &&
                  actual.generation_count() == 12,
              "random scalar values, typed selectors or draw ordering changed");
    }
    Rng stream(123);
    program.code = {op(6, {10000, 10082}, 3), op(53, {})};
    check(vm::run(program, workspace, 8, 400, 100000, &stream).status == vm::Status::returned &&
              workspace.registers[0] == 10082 && stream.generation_count() == 0,
          "integer selector hole incorrectly consumed random angle");
    program.code = {op(6, {10000, 10032}, 3), op(21, {10001, 10032, 10032}, 7), op(53, {})};
    Rng prefix(123);
    prefix.next_u32();
    check(vm::run(program, workspace, 8, 400, 100000, &stream).status == vm::Status::unsupported &&
              stream.seed() == prefix.seed() && stream.generation_count() == 2,
          "ambiguous RNG expression order accepted or failed instruction consumed draws");
    program.code = {op(8, {10000, 10032}, 3), op(53, {})};
    check(vm::run(program, workspace, 8, 400, 100000, &stream).status == vm::Status::unsupported &&
              stream.seed() == prefix.seed() && stream.generation_count() == 2,
          "random-sign and random-value expression order silently chosen");
    program.code = {op(7, {bits(10000.0f), bits(10033.0f)}, 3), op(53, {})};
    check(vm::run(program, workspace, 8, 400, 100000, &stream).status == vm::Status::unsupported &&
              stream.seed() == prefix.seed() && stream.generation_count() == 2,
          "invalid random destination corrupted RNG checkpoint");
    program.code = {op(96, {0, 1, 1, 0, 0, 0, 0, 0}), op(53, {})};
    check(vm::run(program, workspace, 8, 400, 100000, &stream).status ==
                  vm::Status::missing_context &&
              workspace.emissions.empty() && stream.seed() == prefix.seed(),
          "RNG-enabled execution crossed unresolved shot side effects");
    vm::Module module;
    module.subs.resize(2);
    module.subs[0].code = {op(6, {10008, 10032}, 3), op(52, {1}), op(6, {10010, 10032}, 3),
                           op(53, {})};
    module.subs[1].code = {op(6, {10009, 10032}, 3), op(53, {})};
    stream.set_seed(123);
    stream.reset_generation_count();
    Rng expected(123);
    const auto first = expected.next_u32() & 0x7fffffffU;
    const auto second = expected.next_u32() & 0x7fffffffU;
    const auto third = expected.next_u32() & 0x7fffffffU;
    check(vm::run(module, 0, workspace, 8, 400, 100000, &stream).status == vm::Status::returned &&
              workspace.registers[8] == first && workspace.registers[9] == second &&
              workspace.registers[10] == third && stream.seed() == expected.seed() &&
              stream.generation_count() == 6,
          "call/return restored or duplicated shared RNG state");
}
int main() {
    vm::Program program;
    vm::Workspace workspace;
    scalar_tests(workspace);
    call_tests(workspace);
    random_tests(workspace);
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
