#include "../tests/planner_reference.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

using namespace th08::solver;
using namespace th08::geometry;
namespace {
struct Measurements {
    double reference_ms = 0, optimized_ms = 0;
    std::uint64_t expansions = 0, collision_queries = 0, duplicate_successors = 0, checksum = 0;
};
Measurements measure_search(const Model &model, Vec2 start, Movement speed,
                            const Options &options) {
    std::vector<double> reference_ms, optimized_ms;
    Measurements out;
    for (unsigned batch = 0; batch < 14; ++batch) {
        Result reference, optimized;
        auto measure = [&](bool baseline) {
            const auto begin = std::chrono::steady_clock::now();
            auto result = baseline ? reference_plan(model, start, speed, options)
                                   : plan(model, start, speed, options);
            const auto elapsed =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin)
                    .count();
            (baseline ? reference_ms : optimized_ms).push_back(elapsed);
            (baseline ? reference : optimized) = std::move(result);
        };
        measure((batch & 1) != 0);
        measure((batch & 1) == 0);
        if (reference.status != Status::found || optimized.status != reference.status ||
            reference.expansions != optimized.expansions ||
            reference.actions.size() != optimized.actions.size())
            throw std::logic_error("planner benchmark status, budget or route length mismatch");
        for (std::size_t i = 0; i < reference.actions.size(); ++i)
            if (reference.actions[i].x != optimized.actions[i].x ||
                reference.actions[i].y != optimized.actions[i].y ||
                std::memcmp(&reference.positions[i].x, &optimized.positions[i].x, sizeof(float)) ||
                std::memcmp(&reference.positions[i].y, &optimized.positions[i].y, sizeof(float)))
                throw std::logic_error("planner benchmark action or exact position bits mismatch");
        out.expansions = optimized.expansions;
        out.collision_queries = optimized.collision_queries;
        out.duplicate_successors = optimized.duplicate_successors;
        out.checksum += optimized.expansions;
    }
    std::sort(reference_ms.begin(), reference_ms.end());
    std::sort(optimized_ms.begin(), optimized_ms.end());
    out.reference_ms = reference_ms[7];
    out.optimized_ms = optimized_ms[7];
    return out;
}
void write_measurements(std::ostream &out, const Measurements &result) {
    out << "\"batches\":14,\"reference_median_ms\":" << result.reference_ms
        << ",\"optimized_median_ms\":" << result.optimized_ms
        << ",\"identical_routes\":true,\"expansions_per_run\":" << result.expansions
        << ",\"collision_queries_per_run\":" << result.collision_queries
        << ",\"duplicate_successors_per_run\":" << result.duplicate_successors
        << ",\"checksum\":" << result.checksum;
}
} // namespace
int main(int argc, char **argv) {
    Model model;
    model.dependency = Dependency::fixture;
    for (int tick = 0; tick < 360; ++tick) {
        std::vector<Hazard> hazards;
        for (int i = 0; i < 256; ++i) {
            const float x = float((i * 47) % 384);
            const float y = float((i * 31 + tick * 2) % 560 - 56);
            // Preserve a corridor while retaining moving broad-phase workloads.
            if (x > 170 && x < 214)
                continue;
            hazards.push_back(Hazard::bullet({{x, y}, {4, 4}}));
        }
        model.frames.emplace_back(std::move(hazards), Vec2{.825f, .825f});
    }
    Options options;
    options.expansions = 1000000;
    options.terminal = {{192, 400}, {368, 416}};
    const auto moving = measure_search(model, {192, 400}, {}, options);
    Model boundary;
    boundary.dependency = Dependency::fixture;
    for (int tick = 0; tick < 360; ++tick)
        boundary.frames.emplace_back(std::vector<Hazard>{}, Vec2{0, 0});
    options.terminal = {{8, 16}, {0, 0}};
    const auto clamped = measure_search(boundary, {8, 16}, {2, 2}, options);
    std::ofstream file;
    if (argc == 2) {
        file.open(argv[1]);
        if (!file)
            return 2;
    }
    std::ostream &out = argc == 2 ? file : std::cout;
    out << "{\"scope\":\"360-frame synthetic moving-hazard search and replay; alternating order\",";
    write_measurements(out, moving);
    out << ",\"clamped_boundary\":{\"scope\":"
           "\"360-frame empty corner fixture; axis equals diagonal\",";
    write_measurements(out, clamped);
    out << "}}\n";
}
