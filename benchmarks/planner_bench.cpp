#include "../tests/planner_reference.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

using namespace th08::solver;
using namespace th08::geometry;
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
    std::vector<double> reference_ms, optimized_ms;
    std::uint64_t checksum = 0;
    for (unsigned batch = 0; batch < 14; ++batch) {
        Result reference, optimized;
        auto measure = [&](bool baseline) {
            const auto start = std::chrono::steady_clock::now();
            auto result = baseline ? reference_plan(model, {192, 400}, {}, options)
                                   : plan(model, {192, 400}, {}, options);
            const auto elapsed =
                std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                    .count();
            (baseline ? reference_ms : optimized_ms).push_back(elapsed);
            (baseline ? reference : optimized) = std::move(result);
        };
        measure((batch & 1) != 0);
        measure((batch & 1) == 0);
        if (reference.status != Status::found || optimized.status != reference.status ||
            reference.expansions != optimized.expansions ||
            reference.actions.size() != optimized.actions.size())
            return 1;
        for (std::size_t i = 0; i < reference.actions.size(); ++i)
            if (reference.actions[i].x != optimized.actions[i].x ||
                reference.actions[i].y != optimized.actions[i].y)
                return 1;
        checksum += optimized.expansions;
    }
    std::sort(reference_ms.begin(), reference_ms.end());
    std::sort(optimized_ms.begin(), optimized_ms.end());
    std::ofstream file;
    if (argc == 2) {
        file.open(argv[1]);
        if (!file)
            return 2;
    }
    std::ostream &out = argc == 2 ? file : std::cout;
    out << "{\"scope\":\"360-frame synthetic moving-hazard search and replay; alternating order\","
           "\"batches\":14,\"reference_median_ms\":"
        << reference_ms[7] << ",\"optimized_median_ms\":" << optimized_ms[7]
        << ",\"identical_routes\":true,\"checksum\":" << checksum << "}\n";
}
