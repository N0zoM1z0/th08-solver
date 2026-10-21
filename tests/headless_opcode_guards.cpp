#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "Supervisor.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
void check(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(message);
}

template <class Call> void rejects(Call call, const char *diagnostic) {
    try {
        call();
    } catch (const std::runtime_error &error) {
        check(std::string(error.what()).find(diagnostic) != std::string::npos,
              "native guard returned the wrong diagnostic");
        return;
    }
    throw std::runtime_error("unsupported native instruction was silently advanced");
}

th08::AnmRawInstr instruction(std::int16_t opcode) {
    th08::AnmRawInstr result{};
    result.opcode = opcode;
    result.instructionSize = sizeof(result);
    return result;
}

void check_anm() {
    using namespace th08;
    auto manager = std::make_unique<AnmManager>();
    std::array<AnmRawInstr, 4> program{
        {instruction(AnmOpcode_Nop), instruction(AnmOpcode_InterruptLabel),
         instruction(AnmOpcode_Alpha), instruction(AnmOpcode_Static)}};
    program[1].intArgs[0] = 7;
    program[2].intArgs[0] = 91;
    AnmVm vm{};
    vm.Initialize();
    vm.beginningOfScript = vm.currentInstruction = program.data();
    vm.currentTimeInScript = 0;
    check(manager->ExecuteScript(&vm) && !vm.currentInstruction && vm.color1.a == 91,
          "ANM no-op/label changed native instruction ordering");

    // A due unknown instruction must stop before the following known effect.
    program[0] = instruction(32000);
    vm.Initialize();
    vm.beginningOfScript = vm.currentInstruction = program.data();
    vm.currentTimeInScript = 0;
    rejects([&] { manager->ExecuteScript(&vm); }, "unsupported ANM opcode 32000");
    check(vm.currentInstruction == program.data() && vm.color1.a == 255,
          "ANM guard advanced the cursor or executed the suffix");
}

std::unique_ptr<th08::Bullet> bullet(std::uint32_t kind, std::uint32_t enabled) {
    using namespace th08;
    auto result = std::make_unique<Bullet>();
    result->state = BULLET_STATE_FIRED;
    result->position = {12, 34, 0};
    result->activeTransformFlags = 0;
    result->transformFlags = enabled;
    result->transformIndex = 0;
    result->transformSound = -1;
    for (auto &record : result->transforms)
        record = {};
    result->transforms[0].kind = kind;
    result->transforms[1].kind = BULLET_TRANSFORM_WAIT;
    result->transforms[1].payload.timed.frames = 3;
    return result;
}

void check_transforms() {
    using namespace th08;
    constexpr std::array<u32, 9> passive{{
        BULLET_TRANSFORM_SPAWN_FAST,
        BULLET_TRANSFORM_SPAWN_NORMAL,
        BULLET_TRANSFORM_SPAWN_SLOW,
        BULLET_TRANSFORM_PLAY_SPAWN_SOUND,
        BULLET_TRANSFORM_CANCEL_IMMUNE,
        BULLET_TRANSFORM_ONLY_WHEN_PLAYER_YOUKAI,
        BULLET_TRANSFORM_ONLY_WHEN_PLAYER_HUMAN,
        BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER,
        BULLET_TRANSFORM_CANCEL_IMMUNE | BULLET_TRANSFORM_ECL_EX_TRIGGER_MARKER,
    }};
    for (const auto kind : passive) {
        auto test = bullet(kind, kind | BULLET_TRANSFORM_WAIT);
        test->AdvanceTransformProgram();
        check(test->transformIndex == 1 && test->activeTransformFlags == 0,
              "passive transform flag changed native cursor timing");
        test->AdvanceTransformProgram();
        check(test->transformIndex == 2 && test->activeTransformFlags == BULLET_TRANSFORM_WAIT &&
                  test->exStates[BULLET_TRANSFORM_STATE_WAIT].timer == 3,
              "known WAIT after a passive flag changed behavior");
    }

    auto stopped = bullet(BULLET_TRANSFORM_NONE, BULLET_TRANSFORM_WAIT);
    stopped->AdvanceTransformProgram();
    check(stopped->transformIndex == 0 && stopped->activeTransformFlags == 0,
          "NONE transform advanced past the native terminator");

    constexpr u32 unknown = 0x200000;
    auto invalid = bullet(unknown, unknown | BULLET_TRANSFORM_WAIT);
    rejects([&] { invalid->AdvanceTransformProgram(); }, "unsupported bullet-transform kind");
    check(invalid->transformIndex == 0 && invalid->activeTransformFlags == 0 &&
              invalid->state == BULLET_STATE_FIRED && invalid->position.x == 12 &&
              invalid->position.y == 34,
          "unknown transform changed native state before rejection");

    // An unselected record is not executed, just as a difficulty-filtered ECL
    // instruction is skipped. Preserve the source's same-call WAIT handoff.
    auto disabled = bullet(unknown, BULLET_TRANSFORM_WAIT);
    disabled->AdvanceTransformProgram();
    check(disabled->transformIndex == 2 && disabled->activeTransformFlags == BULLET_TRANSFORM_WAIT,
          "disabled record no longer skips into its known successor");
}
} // namespace

int main() {
    try {
        th08::g_Supervisor.framerateMultiplier = 1;
        check_anm();
        check_transforms();
        std::cout << "Native unknown-opcode guards and intentional no-ops passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
