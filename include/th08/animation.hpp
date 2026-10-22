#pragma once
#include "formats.hpp"

namespace th08::animation {
enum class Status { certified, unsupported, invalid };
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
struct Timing {
    Status status = Status::unsupported;
    std::uint32_t stopping_offset = 0, inspected = 0;
    std::int16_t stopping_opcode = 0;
    std::int32_t sprite = -1, completion_time = -1;
    bool hides_on_completion = false;
    // SetAndExecuteScript performs time zero during template initialization.
    // With a unit-rate clock, a copied template needs this many subsequent calls.
    std::uint32_t calls_after_template = 0;
};
// Certify completion timing and the single immutable sprite of a straight-line
// script. This is a dataflow projection, not a renderer or a general ANM VM.
// Preconditions: initialized template, unit clock, no interrupts or external freeze.
// Every accepted non-control instruction writes only visual fields that cannot
// affect completion or sprite identity. Unknown instructions/masks fail closed.
Timing certify_timing(resources::View resource, const resources::Anm &anm, std::size_t script);
} // namespace th08::animation
