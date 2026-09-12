#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace th08::bullet {
// Occupancy/cursor index only, not bullet storage or a complete allocation transaction.
// Every non-UNUSED lifecycle phase must occupy a slot until actual deactivation.
class Slots {
  public:
    static constexpr unsigned capacity = 1536;
    using Occupancy = std::array<bool, capacity>;

  private:
    std::array<std::uint64_t, capacity / 64> free_{};
    std::uint16_t cursor_ = 0, available_ = capacity;

    static unsigned first_bit(std::uint64_t bits) {
        // Called only for a nonzero word; no special CPU ISA is required.
#if defined(__GNUC__) || defined(__clang__)
        return unsigned(__builtin_ctzll(bits));
#else
        unsigned bit = 0;
        while ((bits & 1) == 0) {
            bits >>= 1;
            ++bit;
        }
        return bit;
#endif
    }

  public:
    Slots() {
        free_.fill(~std::uint64_t(0));
    }
    Slots(const Occupancy &occupied, unsigned cursor) : Slots() {
        if (cursor >= capacity)
            throw std::invalid_argument("bullet cursor outside pool");
        cursor_ = std::uint16_t(cursor);
        for (unsigned i = 0; i < capacity; ++i)
            if (occupied[i]) {
                free_[i / 64] &= ~(std::uint64_t(1) << (i % 64));
                --available_;
            }
    }
    unsigned cursor() const {
        return cursor_;
    }
    unsigned available() const {
        return available_;
    }
    bool occupied(unsigned index) const {
        if (index >= capacity)
            throw std::out_of_range("bullet slot outside pool");
        return (free_[index / 64] & (std::uint64_t(1) << (index % 64))) == 0;
    }
    std::optional<std::uint16_t> next() const {
        if (available_ == 0)
            return std::nullopt;
        const unsigned word = cursor_ / 64, offset = cursor_ % 64;
        const auto high = ~std::uint64_t(0) << offset;
        auto candidates = free_[word] & high;
        if (candidates)
            return std::uint16_t(word * 64 + first_bit(candidates));
        for (unsigned step = 1; step < free_.size(); ++step) {
            unsigned index = word + step;
            if (index >= free_.size())
                index -= unsigned(free_.size());
            candidates = free_[index];
            if (candidates)
                return std::uint16_t(index * 64 + first_bit(candidates));
        }
        candidates = free_[word] & ~high;
        if (candidates)
            return std::uint16_t(word * 64 + first_bit(candidates));
        return std::nullopt;
    }
    // Source selects a free slot before drawing launch randomness. Reserving marks
    // it occupied but does NOT move the cursor: initialization may spawn children.
    std::optional<std::uint16_t> reserve() {
        const auto selected = next();
        if (selected) {
            free_[*selected / 64] &= ~(std::uint64_t(1) << (*selected % 64));
            --available_;
        }
        return selected;
    }
    // Called after that spawn's initialization/child callbacks return. A parent's
    // completion can overwrite a cursor already moved by its nested children.
    void finish_spawn(unsigned selected) {
        if (selected >= capacity)
            throw std::out_of_range("completed bullet slot outside pool");
        cursor_ = std::uint16_t(selected + 1 == capacity ? 0 : selected + 1);
    }
    bool release(unsigned index) {
        if (index >= capacity)
            throw std::out_of_range("released bullet slot outside pool");
        auto &word = free_[index / 64];
        const auto bit = std::uint64_t(1) << (index % 64);
        if (word & bit)
            return false;
        word |= bit;
        ++available_;
        return true;
    }
};
} // namespace th08::bullet
