#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace th08::plan {

struct BeamNode {
    float x, y;
    int parent;
    std::uint16_t act; // Movement/focus bits on the transition into this node.
    std::uint16_t viol;
    float score;
};

struct BeamSelection {
    std::size_t candidates = 0, bins = 0, retained_bins = 0;
};

namespace detail {
// Keep the original uint32_t(lround(coordinate * 2)) & 2047 proposal key.
// On the tested glibc/x86-64 binary32 profile, round the unsigned significand
// directly. Preserve the original multiplication and delegate large/special
// values and other layouts to lround, including its domain/error behavior.
inline std::uint32_t half_pixel_bin(float coordinate) {
    const float scaled = coordinate * 2;
#if defined(__GLIBC__) && defined(__x86_64__)
    if constexpr (sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559) {
        const float one = 1.f;
        std::uint32_t layout, bits;
        std::memcpy(&layout, &one, sizeof(layout));
        std::memcpy(&bits, &scaled, sizeof(bits));
        if (layout == 0x3f800000u) {
            const auto exponent = (bits >> 23) & 255u;
            if (exponent < 126)
                return 0;
            if (exponent <= 148) {
                const auto shift = 150 - exponent;
                const auto rounded =
                    (((bits & 0x7fffffu) | 0x800000u) + (std::uint32_t(1) << (shift - 1))) >> shift;
                const auto sign = bits >> 31;
                return ((rounded ^ (0u - sign)) + sign) & 2047u;
            }
        }
    }
#endif
    return std::uint32_t(std::lround(scaled)) & 2047u;
}

struct SpatialBin {
    int x, y;
    bool focus;

    bool operator==(const SpatialBin &other) const {
        return x == other.x && y == other.y && focus == other.focus;
    }
};

struct SpatialHash {
    std::size_t operator()(const SpatialBin &bin) const {
        const std::uint64_t xy = std::uint64_t(std::uint32_t(bin.x)) << 32 | std::uint32_t(bin.y);
        return std::size_t((xy ^ (xy >> 29)) * 0x9e3779b97f4a7c15ULL) ^ std::size_t(bin.focus);
    }
};

inline int bin_coordinate(float coordinate, int cell_size) {
    const double value = std::floor(double(coordinate) / cell_size);
    if (!std::isfinite(value) || value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max())
        throw std::invalid_argument("invalid beam coordinate");
    return int(value);
}
} // namespace detail

// Proposal pruning only: bins never stand in for a simulated state. Preserve
// the best existing node in each spatial/focus bin, then use score rank to fill
// the remaining beam. More bins than slots keeps the best ranked bin winners.
// Equal scores retain their source order, and hash iteration never determines
// a selection. Coordinates, actions and exact parent indices are copied intact.
template <class Node>
inline BeamSelection retain_spatial_beam(std::vector<Node> &nodes, std::size_t width,
                                         int cell_size) {
    if (width == 0 || cell_size <= 0)
        throw std::invalid_argument("spatial beam requires positive width and cell size");
    BeamSelection stats{nodes.size(), 0, 0};
    const auto limit = std::min(width, nodes.size());
    // Source indices make equal-score rank explicit without sorting all candidates.
    // Select each bin's original winner before applying the unchanged global rank.
    const auto rank = [&](std::size_t a, std::size_t b) {
        if (nodes[a].score < nodes[b].score)
            return true;
        if (nodes[b].score < nodes[a].score)
            return false;
        return a < b;
    };
    std::unordered_map<detail::SpatialBin, std::size_t, detail::SpatialHash> bins;
    bins.reserve(nodes.size());
    std::vector<std::size_t> winners;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto &node = nodes[i];
        const detail::SpatialBin bin{detail::bin_coordinate(node.x, cell_size),
                                     detail::bin_coordinate(node.y, cell_size),
                                     (node.act & 4) != 0};
        const auto entry = bins.emplace(bin, winners.size());
        if (entry.second)
            winners.push_back(i);
        else if (node.score < nodes[winners[entry.first->second]].score)
            winners[entry.first->second] = i;
    }
    stats.bins = bins.size();
    stats.retained_bins = std::min(limit, winners.size());
    const auto retain_best = [&](std::vector<std::size_t> &indices, std::size_t count) {
        if (indices.size() > count) {
            std::nth_element(indices.begin(), indices.begin() + count, indices.end(), rank);
            indices.resize(count);
        }
    };
    if (winners.size() < limit) {
        std::vector<unsigned char> selected(nodes.size(), 0);
        for (const auto i : winners)
            selected[i] = 1;
        std::vector<std::size_t> rest;
        rest.reserve(nodes.size() - winners.size());
        for (std::size_t i = 0; i < nodes.size(); ++i)
            if (!selected[i])
                rest.push_back(i);
        retain_best(rest, limit - winners.size());
        winners.insert(winners.end(), rest.begin(), rest.end());
    } else {
        retain_best(winners, limit);
    }
    std::sort(winners.begin(), winners.end(), rank);
    // Preserve source storage until every selected parent/action has been copied.
    std::vector<Node> retained;
    retained.reserve(limit);
    for (const auto i : winners)
        retained.push_back(nodes[i]);
    std::copy(retained.begin(), retained.end(), nodes.begin());
    nodes.resize(retained.size());
    return stats;
}

} // namespace th08::plan
