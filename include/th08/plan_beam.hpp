#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
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

// An optional coordinate domain, not a different quantization or merge rule.
struct SpatialBeamBounds {
    float minx, miny, maxx, maxy;
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

// Only caller-owned capacity is reused; logical contents reset before reading.
// No input node pointers or simulated world state are retained.
// Private buffers prevent the input vector from aliasing retained-node storage.
template <class Node> class SpatialBeamScratch {
    template <class Other>
    friend BeamSelection retain_spatial_beam(std::vector<Other> &, std::size_t, int,
                                             SpatialBeamScratch<Other> &);
    template <class Other>
    friend BeamSelection retain_spatial_beam_bounded(std::vector<Other> &, std::size_t, int,
                                                     SpatialBeamBounds,
                                                     SpatialBeamScratch<Other> &);
    struct Entry {
        detail::SpatialBin bin;
        std::size_t winner_slot;
    };
    std::vector<Entry> table;
    std::vector<unsigned char> occupied, selected;
    std::vector<std::size_t> winners, rest;
    std::vector<Node> retained;
    std::vector<std::uint32_t> dense_slots;
    std::vector<std::uint64_t> winner_keys, rest_keys;

  public:
    SpatialBeamScratch() = default;
};

// Proposal pruning only: bins never stand in for a simulated state. Preserve
// the best existing node in each spatial/focus bin, then use score rank to fill
// the remaining beam. More bins than slots keeps the best ranked bin winners.
// Equal scores retain their source order, and hash iteration never determines
// a selection. Coordinates, actions and exact parent indices are copied intact.
template <class Node>
inline BeamSelection retain_spatial_beam(std::vector<Node> &nodes, std::size_t width, int cell_size,
                                         SpatialBeamScratch<Node> &scratch) {
    if (width == 0 || cell_size <= 0)
        throw std::invalid_argument("spatial beam requires positive width and cell size");
    BeamSelection stats{nodes.size(), 0, 0};
    const auto limit = std::min(width, nodes.size());
    scratch.winners.clear();
    scratch.rest.clear();
    scratch.retained.clear();
    scratch.selected.clear();
    if (nodes.empty()) {
        scratch.occupied.clear();
        return stats;
    }
    if (nodes.size() > std::numeric_limits<std::size_t>::max() / 2)
        throw std::length_error("spatial beam table too large");
    std::size_t table_size = 1;
    while (table_size < nodes.size() * 2) {
        if (table_size > std::numeric_limits<std::size_t>::max() / 2)
            throw std::length_error("spatial beam table too large");
        table_size *= 2;
    }
    scratch.table.resize(table_size);
    scratch.occupied.assign(table_size, 0);
    // Source indices make equal-score rank explicit without sorting all candidates.
    // Select each bin's original winner before applying the unchanged global rank.
    const auto rank = [&](std::size_t a, std::size_t b) {
        if (nodes[a].score < nodes[b].score)
            return true;
        if (nodes[b].score < nodes[a].score)
            return false;
        return a < b;
    };
    auto &winners = scratch.winners;
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const auto &node = nodes[i];
        const detail::SpatialBin bin{detail::bin_coordinate(node.x, cell_size),
                                     detail::bin_coordinate(node.y, cell_size),
                                     (node.act & 4) != 0};
        // Fold high hash bits before applying a power-of-two mask. Full bin
        // equality still resolves collisions; at most half the slots can fill.
        std::uint64_t hash = detail::SpatialHash{}(bin);
        hash ^= hash >> 32;
        auto slot = std::size_t(hash) & (table_size - 1);
        while (scratch.occupied[slot] && !(scratch.table[slot].bin == bin))
            slot = (slot + 1) & (table_size - 1);
        if (!scratch.occupied[slot]) {
            scratch.occupied[slot] = 1;
            scratch.table[slot] = {bin, winners.size()};
            winners.push_back(i);
        } else {
            auto &winner = winners[scratch.table[slot].winner_slot];
            if (node.score < nodes[winner].score)
                winner = i;
        }
    }
    stats.bins = winners.size();
    stats.retained_bins = std::min(limit, winners.size());
    const auto retain_best = [&](std::vector<std::size_t> &indices, std::size_t count) {
        if (indices.size() > count) {
            std::nth_element(indices.begin(), indices.begin() + count, indices.end(), rank);
            indices.resize(count);
        }
    };
    if (winners.size() < limit) {
        auto &selected = scratch.selected;
        selected.assign(nodes.size(), 0);
        for (const auto i : winners)
            selected[i] = 1;
        auto &rest = scratch.rest;
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
    auto &retained = scratch.retained;
    retained.reserve(limit);
    for (const auto i : winners)
        retained.push_back(nodes[i]);
    std::copy(retained.begin(), retained.end(), nodes.begin());
    nodes.resize(retained.size());
    return stats;
}

template <class Node>
inline BeamSelection retain_spatial_beam(std::vector<Node> &nodes, std::size_t width,
                                         int cell_size) {
    SpatialBeamScratch<Node> scratch;
    return retain_spatial_beam(nodes, width, cell_size, scratch);
}

// Use a small dense lookup only when the supplied domain and every node fit.
// Binary32 score keys order finite values and infinities like float comparison;
// signed zeros share a rank and source indices resolve all ties. The original
// node bits are copied untouched. Unsupported domains/layouts delegate to the
// generic selector, including its validation and error behavior.
template <class Node>
inline BeamSelection retain_spatial_beam_bounded(std::vector<Node> &nodes, std::size_t width,
                                                 int cell_size, SpatialBeamBounds bounds,
                                                 SpatialBeamScratch<Node> &scratch) {
    const auto fallback = [&] { return retain_spatial_beam(nodes, width, cell_size, scratch); };
    if constexpr (!std::is_same_v<decltype(Node::score), float> || sizeof(float) != 4 ||
                  !std::numeric_limits<float>::is_iec559) {
        return fallback();
    } else {
        if (width == 0 || cell_size <= 0 || nodes.size() >= UINT32_MAX)
            return fallback();
        const float one = 1.f;
        std::uint32_t layout;
        std::memcpy(&layout, &one, sizeof(layout));
        if (layout != 0x3f800000u || !(bounds.minx <= bounds.maxx) || !(bounds.miny <= bounds.maxy))
            return fallback();
        const double coordinates[] = {std::floor(double(bounds.minx) / cell_size),
                                      std::floor(double(bounds.miny) / cell_size),
                                      std::floor(double(bounds.maxx) / cell_size),
                                      std::floor(double(bounds.maxy) / cell_size)};
        for (const auto value : coordinates)
            if (!std::isfinite(value) || value < std::numeric_limits<int>::min() ||
                value > std::numeric_limits<int>::max())
                return fallback();
        const int minx = int(coordinates[0]), miny = int(coordinates[1]);
        const auto columns = std::uint64_t(std::int64_t(coordinates[2]) - minx + 1);
        const auto rows = std::uint64_t(std::int64_t(coordinates[3]) - miny + 1);
        // Bound scratch space and reset cost even for unusually small cells.
        constexpr std::size_t max_slots = 1 << 16;
        if (columns > max_slots / 2 || rows > max_slots / 2 || columns * rows > max_slots / 2)
            return fallback();
        auto &slots = scratch.dense_slots;
        auto &winners = scratch.winner_keys;
        auto &rest = scratch.rest_keys;
        auto &retained = scratch.retained;
        slots.assign(std::size_t(columns * rows * 2), UINT32_MAX);
        winners.clear();
        rest.clear();
        retained.clear();
        const auto key = [](float score, std::uint32_t index) {
            std::uint32_t bits;
            std::memcpy(&bits, &score, sizeof(bits));
            if ((bits << 1) == 0)
                bits = 0;
            const auto rank = bits & 0x80000000u ? ~bits : bits ^ 0x80000000u;
            return (std::uint64_t(rank) << 32) | index;
        };
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            const auto &node = nodes[i];
            std::uint32_t score_bits;
            std::memcpy(&score_bits, &node.score, sizeof(score_bits));
            if (!(node.x >= bounds.minx && node.x <= bounds.maxx && node.y >= bounds.miny &&
                  node.y <= bounds.maxy) ||
                (score_bits & 0x7fffffffu) > 0x7f800000u)
                return fallback();
            const auto x =
                std::size_t(std::int64_t(detail::bin_coordinate(node.x, cell_size)) - minx);
            const auto y =
                std::size_t(std::int64_t(detail::bin_coordinate(node.y, cell_size)) - miny);
            auto &slot = slots[(y * columns + x) * 2 + ((node.act & 4) != 0)];
            const auto ranked = key(node.score, std::uint32_t(i));
            if (slot == UINT32_MAX) {
                slot = std::uint32_t(winners.size());
                winners.push_back(ranked);
            } else if (ranked < winners[slot]) {
                winners[slot] = ranked;
            }
        }
        const auto limit = std::min(width, nodes.size());
        const BeamSelection stats{nodes.size(), winners.size(), std::min(limit, winners.size())};
        const auto retain_best = [](std::vector<std::uint64_t> &keys, std::size_t count) {
            if (keys.size() > count) {
                std::nth_element(keys.begin(), keys.begin() + count, keys.end());
                keys.resize(count);
            }
        };
        if (winners.size() < limit) {
            scratch.selected.assign(nodes.size(), 0);
            for (const auto ranked : winners)
                scratch.selected[std::uint32_t(ranked)] = 1;
            rest.reserve(nodes.size() - winners.size());
            for (std::size_t i = 0; i < nodes.size(); ++i)
                if (!scratch.selected[i])
                    rest.push_back(key(nodes[i].score, std::uint32_t(i)));
            retain_best(rest, limit - winners.size());
            winners.insert(winners.end(), rest.begin(), rest.end());
        } else {
            retain_best(winners, limit);
        }
        std::sort(winners.begin(), winners.end());
        retained.reserve(limit);
        for (const auto ranked : winners)
            retained.push_back(nodes[std::uint32_t(ranked)]);
        std::copy(retained.begin(), retained.end(), nodes.begin());
        nodes.resize(retained.size());
        return stats;
    }
}

} // namespace th08::plan
