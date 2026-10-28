#pragma once
#include "th08/shadow_protocol.hpp"
#include <algorithm>
#include <array>
#include <cstdint>

namespace th08::shadow::bridge {
// Linux mmap and Wine CreateFileMapping share this little-endian, 32-bit-word
// file. Every access uses lock-free GCC atomics on aligned words on x86. Neither
// pointers, std::atomic objects nor compiler-specific native structs cross it.
constexpr std::uint32_t magic = 0x31534254, version = 1, capacity = 128;
enum Header : unsigned {
    Magic,
    Version,
    Published,
    Accepted,
    Rejected,
    Sequence,
    Epoch,
    Planned,
    Fallback,
    Outcome,
    StopRequested,
    RejectionReason
};
enum class OutcomeCode : std::uint32_t { Running, Complete, Death, Budget, Failure, Stopped };
constexpr unsigned slot_words = 8 + capacity, map_words = 32 + 2 * slot_words;
constexpr unsigned map_bytes = 4 * map_words;
enum class UpdateEvent { None, Control, Calculation, Unsupported };
struct CalculationCursor {
    std::uint32_t stage = 0, frame = 0;
    UpdateEvent observe(std::uint32_t next_stage, std::uint32_t next_frame, bool ready) {
        // Stage control happens before asynchronous retail initialization. It
        // owns a receipt even while gameplay is not ready; the new counter then
        // starts at zero. Waiting/loading callbacks consume no gameplay receipt.
        if (next_stage != stage) {
            stage = next_stage;
            frame = 0;
            return UpdateEvent::Control;
        }
        if (!ready || next_frame == frame)
            return UpdateEvent::None;
        if (next_frame != frame + 1)
            return UpdateEvent::Unsupported;
        frame = next_frame;
        return UpdateEvent::Calculation;
    }
};
static_assert(__atomic_always_lock_free(4, nullptr), "shared words must be lock-free");
inline std::uint32_t load(const std::uint32_t *word) {
    return __atomic_load_n(word, __ATOMIC_ACQUIRE);
}
inline void save(std::uint32_t *word, std::uint32_t value) {
    __atomic_store_n(word, value, __ATOMIC_RELEASE);
}
struct Plan {
    std::uint32_t generation = 0, origin = 0, epoch = 0, first_change = 0;
    std::uint32_t count = 0;
    std::array<std::uint16_t, capacity> inputs{};
};
inline bool valid(const Plan &p) {
    if (!p.generation || (p.generation & 1) || !p.count || p.count > capacity ||
        p.origin > UINT32_MAX - p.count || p.first_change <= p.origin ||
        p.first_change > p.origin + p.count)
        return false;
    return std::all_of(p.inputs.begin(), p.inputs.begin() + p.count,
                       [](auto value) { return !(value & ~std::uint16_t(0x10f5)); });
}
inline unsigned slot(std::uint32_t generation) {
    return 32 + ((generation / 2) % 2) * slot_words;
}
inline void publish(std::uint32_t *map, const Plan &p) {
    if (!valid(p) || p.generation <= load(map + Published))
        throw std::runtime_error("invalid_shadow_bridge_plan");
    auto *target = map + slot(p.generation);
    save(target, p.generation | 1);
    save(target + 1, p.origin);
    save(target + 2, p.epoch);
    save(target + 3, p.first_change);
    save(target + 4, p.count);
    for (unsigned i = 0; i < p.count; ++i)
        save(target + 8 + i, p.inputs[i]);
    save(target, p.generation);
    save(map + Published, p.generation);
}
// The owner copies an accepted plan to private storage. A writer can never edit
// that copy or an already sampled input. A torn/newly superseded slot is retried.
inline bool read(const std::uint32_t *map, Plan &out) {
    const auto generation = load(map + Published);
    if (!generation || (generation & 1))
        return false;
    const auto *source = map + slot(generation);
    if (load(source) != generation)
        return false;
    Plan p;
    p.generation = generation;
    p.origin = load(source + 1);
    p.epoch = load(source + 2);
    p.first_change = load(source + 3);
    p.count = load(source + 4);
    if (!p.count || p.count > capacity)
        return false;
    for (unsigned i = 0; i < p.count; ++i) {
        const auto value = load(source + 8 + i);
        if (value > UINT16_MAX)
            return false;
        p.inputs[i] = std::uint16_t(value);
    }
    if (load(source) != generation || load(map + Published) != generation || !valid(p))
        return false;
    out = p;
    return true;
}
inline bool timely(const Plan &p, std::uint32_t sequence) {
    return valid(p) && p.origin <= sequence && sequence < UINT32_MAX &&
           sequence + 1 < p.first_change;
}
inline std::uint16_t fallback(std::uint16_t previous, std::uint64_t next, bool spell) {
    return std::uint16_t((previous & 0xf4) | 1 | ((!spell && (next & 1)) ? 0x1000 : 0));
}
inline bool compatible_lead(const Plan &proposal, const Plan &active, std::uint16_t previous,
                            std::uint32_t sequence, bool spell) {
    if (!timely(proposal, sequence))
        return false;
    for (auto next = sequence + 1; next < proposal.first_change; ++next) {
        auto expected = fallback(previous, next, spell);
        if (active.count && next > active.origin && next <= active.origin + active.count)
            expected = active.inputs[next - active.origin - 1];
        if (proposal.inputs[next - proposal.origin - 1] != expected)
            return false;
        previous = expected;
    }
    return true;
}
constexpr unsigned trace_bytes = Bytes{}.size() + 24;
using Trace = std::array<std::uint8_t, trace_bytes>;
inline Trace trace(const Packet &packet, std::uint64_t elapsed_ns, std::uint64_t calc_ns,
                   std::uint64_t observer_ns) {
    Trace result{};
    const auto bytes = encode(packet);
    std::copy(bytes.begin(), bytes.end(), result.begin());
    unsigned offset = Bytes{}.size();
    for (auto value : {elapsed_ns, calc_ns, observer_ns})
        for (unsigned i = 0; i < 8; ++i)
            result[offset++] = std::uint8_t(value >> (i * 8));
    return result;
}
struct TraceRecord {
    Packet packet;
    std::uint64_t elapsed_ns = 0, calc_ns = 0, observer_ns = 0;
};
inline TraceRecord decode_trace(const void *data, std::size_t size) {
    if (size != trace_bytes)
        throw std::runtime_error("incomplete_shadow_trace");
    // A trace appends timing words AFTER the complete wire packet. Decode that
    // packet through its versioned interface; treating the extra 24 bytes as a
    // longer header shifts every gameplay field into an unrelated value.
    TraceRecord result;
    result.packet = decode(data, Bytes{}.size());
    const auto *bytes = static_cast<const std::uint8_t *>(data);
    std::size_t offset = Bytes{}.size();
    for (auto *value : {&result.elapsed_ns, &result.calc_ns, &result.observer_ns})
        for (unsigned i = 0; i < 8; ++i)
            *value |= std::uint64_t(bytes[offset++]) << (i * 8);
    return result;
}
} // namespace th08::shadow::bridge
