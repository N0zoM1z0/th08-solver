#pragma once
#include "formats.hpp"

namespace th08::animation {
enum class Status { certified, unsupported, invalid };
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
