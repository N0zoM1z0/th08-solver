#include "planner_reference.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <th08/planner.hpp>
using namespace th08::solver;
using namespace th08::geometry;
void check(bool value, const char *message) {
    if (!value) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    std::mt19937 rng(20260912);
    for (unsigned test = 0; test < 80; ++test) {
        Model model;
        model.dependency = Dependency::fixture;
        const Vec2 start{float(8 + rng() % 369), float(16 + rng() % 417)};
        for (int tick = 0; tick < 24; ++tick) {
            std::vector<Hazard> hazards;
            for (int i = 0; i < 6; ++i)
                hazards.push_back(Hazard::bullet({{start.x + float(int(rng() % 101) - 50),
                                                   start.y + float(int(rng() % 101) - 50)},
                                                  {float(rng() % 16), float(rng() % 16)}}));
            model.frames.emplace_back(std::move(hazards), Vec2{.825f, .825f});
        }
        Options options;
        options.beam = 1 + rng() % 128;
        options.expansions = test % 5 ? 100000 : 30;
        options.terminal = {{192, 224}, {368, 416}};
        const auto actual = plan(model, start, {}, options);
        const auto expected = reference_plan(model, start, {}, options);
        check(actual.status == expected.status && actual.expansions == expected.expansions &&
                  actual.actions.size() == expected.actions.size(),
              "optimized search changed status, budget or route length");
        for (std::size_t i = 0; i < actual.actions.size(); ++i)
            check(actual.actions[i].x == expected.actions[i].x &&
                      actual.actions[i].y == expected.actions[i].y &&
                      actual.positions[i].x == expected.positions[i].x &&
                      actual.positions[i].y == expected.positions[i].y,
                  "optimized search changed deterministic tie-break or position merging");
    }
    Model m;
    for (int t = 0; t < 24; ++t) {
        std::vector<Hazard> h;
        // Synthetic reopening collision gate, not a complete Reisen spell.
        if (t >= 10)
            h.push_back(Hazard::bullet({{192, 352}, {24, 100}}));
        m.frames.emplace_back(h, Vec2{.825f, .825f});
    }
    check(plan(m, {192, 352}, {}).status == Status::unsupported_dependency,
          "unknown future accepted");
    m.dependency = Dependency::fixture;
    m.epoch = 7;
    check(plan(m, {192, 352}, {}).status == Status::invalidated, "stale epoch accepted");
    Options o;
    o.expected_epoch = 7;
    o.terminal = {{224, 352}, {4, 4}};
    Result r = plan(m, {192, 352}, {}, o);
    check(r.status == Status::found && r.actions.size() == 24, "gate route missing");
    Vec2 p{192, 352};
    for (std::size_t t = 0; t < r.actions.size(); ++t) {
        check(std::abs(r.actions[t].x) <= 1 && std::abs(r.actions[t].y) <= 1, "illegal action");
        p = advance(p, r.actions[t], {});
        check(!m.frames[t].scan(p), "unsafe result");
    }
    check(box_hit(p, {0, 0}, o.terminal), "terminal condition missing");
    auto again = plan(m, {192, 352}, {}, o);
    check(again.expansions == r.expansions, "nondeterministic expansion count");
    for (std::size_t i = 0; i < r.actions.size(); ++i)
        check(r.actions[i].x == again.actions[i].x && r.actions[i].y == again.actions[i].y,
              "nondeterministic actions");
    o.expansions = 1;
    auto limited = plan(m, {192, 352}, {}, o);
    check(limited.status == Status::expansion_limit && limited.actions.empty(),
          "budget failure returned actions");
    o.expansions = 200000;
    o.terminal = {{8, 16}, {0, 0}};
    check(plan(m, {192, 352}, {}, o).status == Status::no_terminal_witness,
          "unreachable terminal accepted");
    check(plan(m, {0, 0}, {}, o).status == Status::invalid_argument, "invalid start accepted");
    Model blocked;
    blocked.dependency = Dependency::fixture;
    blocked.frames.emplace_back(std::vector<Hazard>{Hazard::bullet({{192, 224}, {1000, 1000}})},
                                Vec2{1, 1});
    auto failure = plan(blocked, {192, 352}, {});
    check(failure.status == Status::search_exhausted && failure.actions.empty(),
          "blocked world returned route");
    std::cout << "{\"fixture_gate\":\"found_and_replayed\",\"expansions\":" << r.expansions
              << ",\"contract_failures\":0}\n";
}
