#pragma once
#include <cstdint>

// Remaining-call bound for the single ANM instruction an owned VM currently
// points at. The online proposal adapter uses it to bound bullet spawn
// animations without stepping the native VM.
namespace th08::animation {
enum class Cursor { unknown, null_instruction, present };
struct TerminalClock {
    Cursor cursor = Cursor::unknown;
    bool context_supported = false, blocked = false;
    std::int32_t current_time = 0;
    float subframe = 0;
    std::int16_t opcode = 0, scheduled_time = 0;
    std::uint16_t size = 0, mask = 0;
};
// One call per unit-rate update, without external writes, freezes or interrupts.
// Only the currently pointed terminal header is accepted; no future byte scan or
// operand evaluation. Zero means unsupported, never immediate completion.
inline unsigned remaining_terminal_calls(const TerminalClock &clock) {
    if (!clock.context_supported)
        return 0;
    // ExecuteScript tests its null cursor before the VM's blocking flags/clock.
    if (clock.cursor == Cursor::null_instruction)
        return 1;
    if (clock.cursor != Cursor::present || clock.blocked || clock.current_time < 0 ||
        clock.subframe != 0 || clock.size != 8 || clock.mask != 0 ||
        (clock.opcode != -1 && clock.opcode != 1 && clock.opcode != 2))
        return 0;
    const auto calls = std::int64_t(clock.scheduled_time) - clock.current_time + 1;
    return calls > 1 ? unsigned(calls) : 1;
}
} // namespace th08::animation
