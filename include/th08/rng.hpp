#pragma once
#include <cstdint>

namespace th08::random {
struct State {
    std::uint16_t seed, saved_seed;
    std::uint32_t generation_count;
    bool saved_seed_valid;
};
// Explicit seed, shared draw order. The first U16 draw is the high U32 word;
// the native source oracle checks this against the pinned reconstructed functions.
// Original binary/compiler evaluation order is not inferred from C++ syntax alone.
class Rng {
    State state_;

  public:
    explicit Rng(std::uint16_t seed, std::uint32_t generation_count = 0)
        : state_{seed, 0, generation_count, false} {}
    explicit Rng(State state) : state_(state) {}
    State state() const {
        return state_;
    }
    std::uint16_t seed() const {
        return state_.seed;
    }
    std::uint32_t generation_count() const {
        return state_.generation_count;
    }
    void set_seed(std::uint16_t seed) {
        state_.seed = seed;
    }
    void reset_generation_count() {
        state_.generation_count = 0;
    }
    void save_seed() {
        state_.saved_seed = state_.seed;
        state_.saved_seed_valid = true;
    }
    // Retail RestoreSavedSeed restores only the seed, not the draw count.
    // An unknown entry backup is never silently treated as zero.
    bool restore_saved_seed() {
        if (!state_.saved_seed_valid)
            return false;
        state_.seed = state_.saved_seed;
        return true;
    }
    std::uint16_t next_u16() {
        const auto temporary = std::uint16_t((state_.seed ^ 0x9630U) - 0x6553U);
        state_.seed = std::uint16_t(((temporary & 0xc000U) >> 14) + std::uint32_t(temporary) * 4);
        ++state_.generation_count;
        return state_.seed;
    }
    std::uint32_t next_u32() {
        const auto high = std::uint32_t(next_u16());
        const auto low = std::uint32_t(next_u16());
        return (high << 16) | low;
    }
    std::uint16_t range_u16(std::uint16_t range) {
        return range ? std::uint16_t(next_u16() % range) : 0;
    }
    std::uint32_t range_u32(std::uint32_t range) {
        return range ? next_u32() % range : 0;
    }
    float unit() {
        return float(next_u32()) / 0x1p32f;
    }
    float signed_unit() {
        return float(next_u32()) / 0x1p31f - 1.0f;
    }
    // Floating zero ranges still consume draws, unlike integer zero ranges.
    float range_float(float range) {
        return unit() * range;
    }
    float range_signed_float(float range) {
        return signed_unit() * range;
    }
};
} // namespace th08::random
