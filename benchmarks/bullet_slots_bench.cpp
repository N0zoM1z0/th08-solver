#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <th08/bullet_slots.hpp>
#include <vector>

using th08::bullet::Slots;
struct Case {
    Slots::Occupancy occupied;
    Slots slots;
    unsigned cursor;
};
int main(int argc, char **argv) {
    std::mt19937 random(20260917);
    std::array<Case, 32> cases;
    for (auto &test : cases) {
        test.occupied.fill(true);
        test.cursor = random() % Slots::capacity;
        test.occupied[(test.cursor + Slots::capacity - 1) % Slots::capacity] = false;
        test.slots = Slots(test.occupied, test.cursor);
        if (test.slots.next() != (test.cursor + Slots::capacity - 1) % Slots::capacity)
            return 1;
    }
    constexpr unsigned queries = 20000;
    std::vector<double> scan_ns, indexed_ns;
    std::uint64_t checksum = 0;
    for (unsigned batch = 0; batch < 21; ++batch) {
        std::uint64_t sums[2]{};
        for (unsigned order = 0; order < 2; ++order) {
            const bool indexed = ((batch + order) & 1) != 0;
            const auto started = std::chrono::steady_clock::now();
            for (unsigned query = 0; query < queries; ++query) {
                const auto &test = cases[(query * 13) % cases.size()];
                unsigned selected = Slots::capacity;
                if (indexed) {
                    const auto value = test.slots.next();
                    if (value)
                        selected = *value;
                } else {
                    unsigned cursor = test.cursor;
                    for (unsigned probe = 0; probe < Slots::capacity; ++probe) {
                        if (!test.occupied[cursor]) {
                            selected = cursor;
                            break;
                        }
                        if (++cursor == Slots::capacity)
                            cursor = 0;
                    }
                }
                sums[indexed] += selected;
            }
            const auto elapsed =
                std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - started)
                    .count() /
                queries;
            (indexed ? indexed_ns : scan_ns).push_back(elapsed);
        }
        if (sums[0] != sums[1])
            return 1;
        checksum += sums[1];
    }
    std::sort(scan_ns.begin(), scan_ns.end());
    std::sort(indexed_ns.begin(), indexed_ns.end());
    std::ofstream file;
    if (argc == 2) {
        file.open(argv[1]);
        if (!file)
            return 2;
    }
    auto &out = argc == 2 ? file : std::cout;
    out << "{\"scope\":\"selection-only near-full pools; compact bool scan baseline; excludes "
           "lifecycle and game execution\","
        << "\"queries_per_batch\":" << queries
        << ",\"batches\":21,\"scan_median_ns\":" << scan_ns[10]
        << ",\"indexed_median_ns\":" << indexed_ns[10]
        << ",\"identical_selections\":true,\"checksum\":" << checksum << "}\n";
}
