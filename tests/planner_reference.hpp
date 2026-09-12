#pragma once
// Unoptimized checkpoint implementation retained only as a deterministic test oracle.
#include <set>
#include <th08/planner.hpp>
namespace th08::solver {
inline Result reference_plan(const Model &model, Vec2 start, Movement speed,
                             const Options &options = {}) {
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
    std::vector<std::vector<Node>> layers{{{start, 0, {}, 0}}};
    for (const auto &frame : model.frames) {
        std::vector<Node> next;
        for (std::size_t parent = 0; parent < layers.back().size(); ++parent)
            for (int y = -1; y <= 1; ++y)
                for (int x = -1; x <= 1; ++x) {
                    if (out.expansions >= options.expansions) {
                        out.status = Status::expansion_limit;
                        return out;
                    }
                    ++out.expansions;
                    Vec2 p = advance(layers.back()[parent].p, {x, y}, speed);
                    if (frame.query(p))
                        continue;
                    double dx = double(p.x) - options.terminal.center.x,
                           dy = double(p.y) - options.terminal.center.y;
                    next.push_back({p, parent, {x, y}, dx * dx + dy * dy});
                }
        std::stable_sort(next.begin(), next.end(),
                         [](const Node &a, const Node &b) { return a.score < b.score; });
        std::set<std::pair<float, float>> seen;
        std::vector<Node> kept;
        for (const auto &n : next)
            if (seen.emplace(n.p.x, n.p.y).second) {
                kept.push_back(n);
                if (kept.size() == options.beam)
                    break;
            }
        if (kept.empty()) {
            out.status = Status::search_exhausted;
            return out;
        }
        layers.push_back(std::move(kept));
    }
    const auto &last = layers.back();
    auto terminal = std::find_if(last.begin(), last.end(), [&](const Node &n) {
        return geometry::box_hit(n.p, {0, 0}, options.terminal);
    });
    if (terminal == last.end()) {
        out.status = Status::no_terminal_witness;
        return out;
    }
    std::size_t parent = std::size_t(terminal - last.begin());
    out.actions.resize(model.frames.size());
    out.positions.resize(model.frames.size());
    for (std::size_t t = model.frames.size(); t > 0; --t) {
        const auto &n = layers[t][parent];
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
