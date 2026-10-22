#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_set>
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
    std::stable_sort(nodes.begin(), nodes.end(),
                     [](const Node &a, const Node &b) { return a.score < b.score; });
    const auto limit = std::min(width, nodes.size());
    std::vector<unsigned char> selected(nodes.size(), 0);
    std::unordered_set<detail::SpatialBin, detail::SpatialHash> bins;
    bins.reserve(nodes.size());
    std::size_t count = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto &node = nodes[i];
        const detail::SpatialBin bin{detail::bin_coordinate(node.x, cell_size),
                                     detail::bin_coordinate(node.y, cell_size),
                                     (node.act & 4) != 0};
        if (bins.insert(bin).second && count < limit) {
            selected[i] = 1;
            ++count;
        }
    }
    stats.bins = bins.size();
    stats.retained_bins = count;
    for (std::size_t i = 0; i < nodes.size() && count < limit; ++i)
        if (!selected[i]) {
            selected[i] = 1;
            ++count;
        }
    std::size_t output = 0;
    for (std::size_t i = 0; i < nodes.size(); ++i)
        if (selected[i])
            nodes[output++] = nodes[i];
    nodes.resize(output);
    return stats;
}

} // namespace th08::plan
