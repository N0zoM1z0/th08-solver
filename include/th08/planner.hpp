#pragma once
#include "geometry.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <string>

namespace th08::solver {
using geometry::Vec2;
struct Action {
    int x = 0, y = 0;
};
struct Movement {
    float axis = 2, diagonal = 1.414213538f;
};
inline Vec2 advance(Vec2 p, Action a, Movement speed) {
    float v = (a.x && a.y) ? speed.diagonal : speed.axis;
    return {std::clamp(p.x + float(a.x) * v, 8.f, 376.f),
            std::clamp(p.y + float(a.y) * v, 16.f, 432.f)};
}
enum class Dependency { unknown, fixture, independent };
struct Model {
    // Frame t is the lethal collision phase AFTER action t moves the player.
    // No focus transition, damage, gate or entity update is inferred here.
    std::vector<geometry::Snapshot> frames;
    Dependency dependency = Dependency::unknown;
    std::uint64_t epoch = 0;
};
enum class Status {
    found,
    unsupported_dependency,
    invalidated,
    invalid_argument,
    expansion_limit,
    search_exhausted,
    no_terminal_witness
};
struct Options {
    std::size_t beam = 128;
    std::uint64_t expansions = 200000, expected_epoch = 0;
    geometry::Box terminal{{192, 352}, {368, 416}};
};
struct Result {
    Status status = Status::invalid_argument;
    std::vector<Action> actions;
    std::vector<Vec2> positions;
    std::uint64_t expansions = 0;
    // Attempted actions still consume the budget, including duplicate successors.
    // These counters cover search only, not the independent final replay.
    std::uint64_t collision_queries = 0, duplicate_successors = 0;
};
// Deterministic, bounded beam proposal followed by unindexed replay. Failure is
// search failure, never a proof of impossibility or a safe fallback input.
inline Result plan(const Model &model, Vec2 start, Movement speed, const Options &options = {}) {
    Result out;
    if (model.dependency != Dependency::fixture && model.dependency != Dependency::independent) {
        out.status = Status::unsupported_dependency;
        return out;
    }
    if (model.epoch != options.expected_epoch) {
        out.status = Status::invalidated;
        return out;
    }
    if (!geometry::finite(start) || start.x < 8 || start.x > 376 || start.y < 16 || start.y > 432 ||
        !std::isfinite(speed.axis) || !std::isfinite(speed.diagonal) || speed.axis <= 0 ||
        speed.diagonal <= 0 || speed.axis > 32 || speed.diagonal > 32 || model.frames.empty() ||
        model.frames.size() > 4096 || options.beam == 0 || options.beam > 4096)
        return out;
    try {
        geometry::coordinate(options.terminal.center);
        geometry::size(options.terminal.size);
    } catch (const std::invalid_argument &) {
        return out;
    }
    struct Node {
        Vec2 p;
        std::size_t parent;
        Action action;
        double score;
    };
    const auto better = [](const Node &a, const Node &b) {
        if (a.score != b.score)
            return a.score < b.score;
        if (a.parent != b.parent)
            return a.parent < b.parent;
        if (a.action.y != b.action.y)
            return a.action.y < b.action.y;
        return a.action.x < b.action.x;
    };
    const auto worse = [&](const Node &a, const Node &b) { return better(b, a); };
    std::vector<Node> nodes, next;
    nodes.reserve(1 + model.frames.size() * options.beam);
    next.reserve(options.beam * 9);
    nodes.push_back({start, 0, {}, 0});
    std::size_t first = 0, count = 1;
    std::size_t table_size = 4;
    // At most nine successors per parent; keep the table below half occupancy.
    while (table_size < options.beam * 18)
        table_size *= 2;
    std::vector<std::uint64_t> seen(table_size);
    std::vector<std::size_t> candidate(table_size);
    constexpr auto blocked = std::numeric_limits<std::size_t>::max();
    auto key = [](Vec2 p) {
        std::uint32_t x, y;
        std::memcpy(&x, &p.x, sizeof(x));
        std::memcpy(&y, &p.y, sizeof(y));
        return (std::uint64_t(x) << 32) | y;
    };
    auto hash = [](std::uint64_t value) {
        value ^= value >> 30;
        value *= 0xbf58476d1ce4e5b9ULL;
        value ^= value >> 27;
        value *= 0x94d049bb133111ebULL;
        return value ^ (value >> 31);
    };
    for (const auto &frame : model.frames) {
        next.clear();
        std::fill(seen.begin(), seen.end(), 0);
        for (std::size_t parent = first; parent < first + count; ++parent)
            for (int y = -1; y <= 1; ++y)
                for (int x = -1; x <= 1; ++x) {
                    if (out.expansions >= options.expansions) {
                        out.status = Status::expansion_limit;
                        return out;
                    }
                    ++out.expansions;
                    Vec2 p = advance(nodes[parent].p, {x, y}, speed);
                    // This equivalence is valid only for the fixed Model above:
                    // identical positions share collision results and scores.
                    // Clamping produces positive finite coordinates, excluding
                    // signed zero, so bit keys preserve exact position equality.
                    const auto position = key(p);
                    auto slot = std::size_t(hash(position)) & (table_size - 1);
                    while (seen[slot] != 0 && seen[slot] != position)
                        slot = (slot + 1) & (table_size - 1);
                    const bool duplicate = seen[slot] == position;
                    if (duplicate) {
                        ++out.duplicate_successors;
                        if (candidate[slot] == blocked)
                            continue;
                    } else {
                        seen[slot] = position;
                        ++out.collision_queries;
                        if (frame.query(p)) {
                            candidate[slot] = blocked;
                            continue;
                        }
                    }
                    double dx = double(p.x) - options.terminal.center.x,
                           dy = double(p.y) - options.terminal.center.y;
                    const Node n{p, parent, {x, y}, dx * dx + dy * dy};
                    if (duplicate) {
                        // Keep the comparator winner, rather than relying on
                        // first arrival if candidate generation order changes.
                        auto &previous = next[candidate[slot]];
                        if (better(n, previous))
                            previous = n;
                    } else {
                        candidate[slot] = next.size();
                        next.push_back(n);
                    }
                }
        // Pop only enough candidates to fill the beam. Explicit generation-order
        // tie-breaks preserve the old stable_sort result without sorting the tail.
        std::make_heap(next.begin(), next.end(), worse);
        first = nodes.size();
        count = 0;
        while (!next.empty() && count < options.beam) {
            std::pop_heap(next.begin(), next.end(), worse);
            const auto n = next.back();
            next.pop_back();
            nodes.push_back(n);
            ++count;
        }
        if (count == 0) {
            out.status = Status::search_exhausted;
            return out;
        }
    }
    auto terminal = std::find_if(nodes.begin() + first, nodes.end(), [&](const Node &n) {
        return geometry::box_hit(n.p, {0, 0}, options.terminal);
    });
    if (terminal == nodes.end()) {
        out.status = Status::no_terminal_witness;
        return out;
    }
    std::size_t parent = std::size_t(terminal - nodes.begin());
    out.actions.resize(model.frames.size());
    out.positions.resize(model.frames.size());
    for (std::size_t t = model.frames.size(); t > 0; --t) {
        const auto &n = nodes[parent];
        out.actions[t - 1] = n.action;
        out.positions[t - 1] = n.p;
        parent = n.parent;
    }
    Vec2 p = start;
    for (std::size_t t = 0; t < model.frames.size(); ++t) {
        p = advance(p, out.actions[t], speed);
        if (model.frames[t].scan(p))
            throw std::logic_error("indexed path failed reference replay");
    }
    out.status = Status::found;
    return out;
}
} // namespace th08::solver
