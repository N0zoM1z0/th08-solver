#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace th08::online {
struct TurnHint {
    bool supported = false;
    unsigned records = 0;
    float radians = 0;
};

// Read-only cue from one bounded, little-endian native ECL instruction batch.
// Sum absolute literal writes on both forward branch paths. This is not a
// general interpreter or a certificate that later loop iterations, positions,
// callbacks or rates remain unchanged. No operands or gameplay RNG are resolved.
inline TurnHint current_batch_turn_hint(const std::uint8_t *bytes, std::size_t size,
                                        std::uint32_t difficulty) {
    TurnHint result;
    if (!bytes || size < 12)
        return result;
    const auto read = [&](auto zero, std::size_t offset) {
        auto value = zero;
        std::memcpy(&value, bytes + offset, sizeof(value));
        return value;
    };
    constexpr float main_anm_rotation_z = 10094.f;
    const auto time = read(std::int32_t{}, 0);
    std::size_t cursor = 0;
    double radians = 0;
    for (; result.records < 64; ++result.records) {
        if (size - cursor < 12)
            return result;
        if (read(std::int32_t{}, cursor) != time) {
            result.supported = true;
            result.radians = float(std::min(radians, std::acos(-1.0)));
            return result;
        }
        const int opcode = read(std::int16_t{}, cursor + 4);
        const int length = read(std::int16_t{}, cursor + 6);
        if (length < 12 || std::size_t(length) > size - cursor)
            return result;
        const auto flags = read(std::uint16_t{}, cursor + 10);
        if ((bytes[cursor + 9] & difficulty) == difficulty) {
            const auto has = [&](int words) { return length >= 12 + words * 4; };
            const auto real = [&](int word) { return read(float{}, cursor + 12 + word * 4); };
            const auto integer = [&](int word) {
                return read(std::int32_t{}, cursor + 12 + word * 4);
            };
            if (opcode == 0) {
                // No-op.
            } else if (opcode == 4) {
                if (!has(2) || integer(1) <= 0 || integer(0) != time)
                    return result;
            } else if (opcode >= 48 && opcode <= 53) {
                if (!has(4) || integer(3) <= 0 || integer(2) != time)
                    return result;
            } else if (opcode == 6) {
                if (!has(2) || integer(0) == int(main_anm_rotation_z))
                    return result;
            } else if (opcode == 137) {
                if (!has(1) || (flags & 1))
                    return result;
            } else if (opcode == 165) {
                if (!has(1) || !(flags & 1) || real(0) != main_anm_rotation_z)
                    return result;
            } else if (opcode == 63) {
                if (!has(2))
                    return result; // Position remains a separate geometry hypothesis.
            } else if (opcode == 7 || (opcode >= 15 && opcode <= 19) ||
                       (opcode >= 25 && opcode <= 29) || opcode == 32 || opcode == 33) {
                const int words = opcode >= 25 && opcode <= 29 ? 3 : 2;
                if (!has(words) || !(flags & 1))
                    return result;
                if (real(0) == main_anm_rotation_z) {
                    if ((opcode != 15 && opcode != 16) || (flags & 2) || !std::isfinite(real(1)))
                        return result;
                    radians += std::abs(double(real(1)));
                }
            } else {
                return result;
            }
        }
        cursor += std::size_t(length);
    }
    return result;
}
} // namespace th08::online
