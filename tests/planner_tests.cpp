#include "planner_reference.hpp"
#include "test_support.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
#include <th08/planner.hpp>
using namespace th08::solver;
using namespace th08::geometry;
using th08::test::check;
namespace {
bool same_bits(float a, float b) {
    return std::memcmp(&a, &b, sizeof(a)) == 0;
}
Result check_reference(const Model &model, Vec2 start, Movement speed, const Options &options) {
    const auto actual = plan(model, start, speed, options);
    const auto expected = reference_plan(model, start, speed, options);
    check(actual.status == expected.status && actual.expansions == expected.expansions &&
              actual.actions.size() == expected.actions.size(),
          "optimized search changed status, budget or route length");
    check(actual.expansions == actual.collision_queries + actual.duplicate_successors,
          "search metrics lost an attempted action");
    for (std::size_t i = 0; i < actual.actions.size(); ++i)
        check(actual.actions[i].x == expected.actions[i].x &&
                  actual.actions[i].y == expected.actions[i].y &&
                  same_bits(actual.positions[i].x, expected.positions[i].x) &&
                  same_bits(actual.positions[i].y, expected.positions[i].y),
              "optimized search changed deterministic tie-break or exact position bits");
    return actual;
}
} // namespace
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
        check_reference(model, start, {}, options);
    }
    Model boundary;
    boundary.dependency = Dependency::independent;
    boundary.frames.emplace_back(std::vector<Hazard>{}, Vec2{0, 0});
    Options edge_options;
    edge_options.terminal = {{8, 16}, {0, 0}};
    auto corner = check_reference(boundary, {8, 16}, {2, 2}, edge_options);
    check(corner.status == Status::found && corner.expansions == 9 &&
              corner.collision_queries == 4 && corner.duplicate_successors == 5 &&
              corner.actions[0].x == -1 && corner.actions[0].y == -1,
          "clamped successor merge changed generation-order winner or query count");
    boundary.frames[0] = Snapshot({Hazard::bullet({{8, 16}, {0, 0}})}, {0, 0});
    edge_options.terminal = {{192, 224}, {368, 416}};
    auto blocked_corner = check_reference(boundary, {8, 16}, {2, 2}, edge_options);
    check(blocked_corner.collision_queries == 4 && blocked_corner.duplicate_successors == 5,
          "blocked duplicate successors were queried again");
    boundary.frames[0] = Snapshot({}, {0, 0});
    const float tiny = std::numeric_limits<float>::denorm_min();
    auto rounded = check_reference(boundary, {192, 224}, {tiny, tiny}, edge_options);
    check(rounded.collision_queries == 1 && rounded.duplicate_successors == 8 &&
              rounded.actions[0].x == -1 && rounded.actions[0].y == -1,
          "rounded equal successors lost the earliest action");
    const float ulp = std::nextafter(192.f, 193.f) - 192.f;
    auto adjacent = check_reference(boundary, {192, 224}, {ulp, ulp}, edge_options);
    check(adjacent.collision_queries == 9 && adjacent.duplicate_successors == 0,
          "distinct adjacent float positions were merged");
    for (int tick = 1; tick < 8; ++tick)
        boundary.frames.emplace_back(std::vector<Hazard>{}, Vec2{0, 0});
    for (const auto start :
         std::array<Vec2, 5>{{{8, 16}, {376, 16}, {8, 432}, {376, 432}, {192, 224}}})
        for (const auto speed : std::array<Movement, 4>{{{}, {2, 2}, {ulp, ulp}, {32, 32}}})
            for (const std::size_t beam : {1, 17}) {
                edge_options.beam = beam;
                for (const std::uint64_t budget : {0, 8, 9, 10, 161, 10000}) {
                    edge_options.expansions = budget;
                    check_reference(boundary, start, speed, edge_options);
                }
            }
    // Signed zero cannot be a valid player coordinate or movement magnitude.
    // It is permitted in terminal geometry, which does not participate in keys.
    edge_options.expansions = 10000;
    edge_options.terminal = {{-0.f, +0.f}, {1000, 1000}};
    check_reference(boundary, {8, 16}, {}, edge_options);
    check(plan(boundary, {-0.f, 16}, {}, edge_options).status == Status::invalid_argument &&
              plan(boundary, {8, -0.f}, {}, edge_options).status == Status::invalid_argument &&
              plan(boundary, {8, 16}, {-0.f, 1}, edge_options).status == Status::invalid_argument,
          "signed-zero position or movement bypassed the domain contract");
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
              << ",\"collision_queries\":" << r.collision_queries
              << ",\"duplicate_successors\":" << r.duplicate_successors
              << ",\"contract_failures\":0}\n";
}
