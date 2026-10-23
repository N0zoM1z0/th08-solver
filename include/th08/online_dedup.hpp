#pragma once

#include "th08/online.hpp"
#include "th08/plan_beam.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace th08::online::detail {
// The legacy key has cyclic half-pixel coordinates and one focus bit. Optional
// direction channels distinguish incoming actions on selected layers, while
// position merging remains approximate. No Native branch state is retained.
class ProposalSet {
    std::vector<std::uint64_t> words, spare;
    std::size_t columns = 2048, rows = 2048;
    unsigned channels = 2;
    std::uint32_t base_x = 0, base_y = 0;
    std::uint64_t expanded = 0;
    static constexpr std::uint32_t mask = 2047;

    void expand() {
        // Bounds are an acceleration hint. Preserve every marked key when an
        // unexpected successor needs the full cyclic domain, including wraps.
        spare.assign((2048 * 2048 * channels) / 64, 0);
        ++expanded;
        for (std::size_t y = 0; y < rows; ++y)
            for (std::size_t x = 0; x < columns; ++x)
                for (unsigned state = 0; state < channels; ++state) {
                    const auto old = (y * columns + x) * channels + state;
                    if (words[old / 64] & (std::uint64_t(1) << (old % 64))) {
                        const auto bx = (x + base_x) & mask, by = (y + base_y) & mask;
                        const auto bit = (by * 2048 + bx) * channels + state;
                        spare[bit / 64] |= std::uint64_t(1) << (bit % 64);
                    }
                }
        words.swap(spare);
        columns = rows = 2048;
        base_x = base_y = 0;
    }

  public:
    // Coordinates use the Observation finite +/-2^20 profile. Invalid hints
    // choose the full domain without changing key validation or membership.
    // Each call starts a fresh layer; only buffer capacity survives a reset.
    void reset(const Box &bounds, bool retain_headings = false) {
        expanded = 0;
        channels = retain_headings ? 18 : 2;
        bool valid = bounds.x0 <= bounds.x1 && bounds.y0 <= bounds.y1;
        for (const float v : {bounds.x0, bounds.y0, bounds.x1, bounds.y1})
            valid = valid && std::isfinite(v) && std::fabs(v) <= 1048576.f;
        if (valid) {
            const auto x0 = std::lround(bounds.x0 * 2), x1 = std::lround(bounds.x1 * 2);
            const auto y0 = std::lround(bounds.y0 * 2), y1 = std::lround(bounds.y1 * 2);
            columns = std::size_t(std::min(2048L, x1 - x0 + 1));
            rows = std::size_t(std::min(2048L, y1 - y0 + 1));
            base_x = std::uint32_t(x0) & mask;
            base_y = std::uint32_t(y0) & mask;
        } else {
            columns = rows = 2048;
            base_x = base_y = 0;
        }
        words.assign((columns * rows * channels + 63) / 64, 0);
    }
    std::uint64_t expansions() const {
        return expanded;
    }

    // Called serially in the original generation order, before any lethal
    // probe. A failed proposal still owns its key, just as in the old hash set.
    bool first(float x, float y, bool focus, unsigned direction = 0) {
        if (channels == 18 && direction >= 9)
            throw std::runtime_error("invalid_online_proposal_direction");
        const auto bx = plan::detail::half_pixel_bin(x), by = plan::detail::half_pixel_bin(y);
        auto ix = (bx - base_x) & mask, iy = (by - base_y) & mask;
        if (ix >= columns || iy >= rows) {
            expand();
            ix = bx;
            iy = by;
        }
        const unsigned state = channels == 18 ? direction * 2 + unsigned(focus) : unsigned(focus);
        const auto bit = (iy * columns + ix) * channels + state;
        auto &word = words[bit / 64];
        const auto flag = std::uint64_t(1) << (bit % 64);
        if (word & flag)
            return false;
        word |= flag;
        return true;
    }
};
} // namespace th08::online::detail
