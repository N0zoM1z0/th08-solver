#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <th08/geometry.hpp>

using namespace th08::geometry;
int main(int argc, char **argv) {
    if (argc > 2) {
        std::cerr << "Usage: geometry_bench [output.json]\n";
        return 2;
    }
    std::ofstream file;
    if (argc == 2) {
        file.open(argv[1]);
        if (!file)
            return 2;
    }
    std::ostream &output = argc == 2 ? file : std::cout;
    std::mt19937 rng(20260912);
    auto u = [&](float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); };
    std::vector<Hazard> hazards;
    for (int i = 0; i < 1536; ++i)
        hazards.push_back(Hazard::bullet({{u(0, 384), u(0, 448)}, {4, 4}}));
    auto start = std::chrono::steady_clock::now();
    Snapshot snapshot(hazards, {.825f, .825f});
    const double build_us =
        std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - start).count();
    std::vector<Vec2> points;
    for (int i = 0; i < 20000; ++i)
        points.push_back({u(8, 376), u(16, 432)});
    std::array<std::uint64_t, 2> checks{}, hits{};
    std::array<double, 2> elapsed{};
    for (int mode = 0; mode < 2; ++mode) {
        start = std::chrono::steady_clock::now();
        for (auto p : points)
            hits[mode] += mode ? snapshot.query(p, &checks[mode]) : snapshot.scan(p, &checks[mode]);
        elapsed[mode] =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
    }
    output << "{\"scope\":\"synthetic geometry queries, no world or "
              "solver\",\"queries\":20000,\"hazards\":1536,\"build_us\":"
           << build_us << ",\"scan_ms\":" << elapsed[0] << ",\"grid_ms\":" << elapsed[1]
           << ",\"scan_checks\":" << checks[0] << ",\"grid_checks\":" << checks[1]
           << ",\"scan_hits\":" << hits[0] << ",\"grid_hits\":" << hits[1] << "}\n";
    return hits[0] == hits[1] ? 0 : 1;
}
