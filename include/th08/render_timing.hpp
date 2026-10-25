#pragma once
#include <array>
#include <cstdint>

namespace th08::render_timing {
// Diagnostic file for the owned 32-bit x86 Wine process: little-endian words,
// a 16-byte header and 64-byte samples. No pointer or compiler struct crosses it.
constexpr std::uint32_t magic = 0x31544d52, version = 1;
using Header = std::array<std::uint32_t, 4>;
using Sample = std::array<std::uint64_t, 8>;
enum Field { Sequence, Stage, ElapsedNs, PeriodNs, DrawNs, PresentNs, DrawCalls, Flags };
enum Flag : std::uint64_t { NoDraw = 1, PresentFailed = 2, RepeatedReceipt = 4 };
constexpr Header header{magic, version, sizeof(Sample), 0};
} // namespace th08::render_timing
