#include "th08/online.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>

namespace th08::online {
namespace {
void require(bool condition) {
    if (!condition)
        throw std::runtime_error("invalid_risk_model");
}
template <class Store>
void rows(const std::filesystem::path &file, const std::string &expected, Store store) {
    std::ifstream input(file);
    require(bool(input));
    std::string line;
    require(bool(std::getline(input, line)));
    if (!line.empty() && line.back() == '\r')
        line.pop_back();
    require(line == expected);
    const auto columns = 1 + std::count(expected.begin(), expected.end(), '\t');
    while (std::getline(input, line)) {
        std::istringstream record(line);
        std::vector<double> values;
        double value;
        while (record >> value) {
            require(std::isfinite(value));
            values.push_back(value);
        }
        require(record.eof() && values.size() == std::size_t(columns));
        store(values);
    }
    require(input.eof());
}
std::int64_t integer(double value, std::int64_t maximum) {
    require(value >= 0 && value <= double(maximum) && value == std::floor(value));
    return std::int64_t(value);
}
int horizon_index(double value) {
    for (int i = 0; i < 4; ++i)
        if (value == std::array<int, 4>{4, 8, 16, 32}[i])
            return i;
    throw std::runtime_error("invalid_risk_horizon");
}
} // namespace

RiskTables load_risk_tables(const std::string &directory) {
    const std::filesystem::path root(directory);
    RiskTables tables;
    const auto load = [&](const char *name, const char *header, int keys, auto &destination) {
        std::set<std::pair<std::uint64_t, int>> seen;
        rows(root / name, header, [&](const std::vector<double> &v) {
            std::int64_t segment = 0, sprite = 0, size = 0, speed, age;
            if (keys == 6) {
                segment = integer(v[0], 65535);
                sprite = integer(v[1], 65535);
                size = integer(v[2], 65535);
                speed = integer(v[3], 255);
                age = integer(v[4], 255);
            } else if (keys == 4) {
                segment = integer(v[0], 65535);
                speed = integer(v[1], 255);
                age = integer(v[2], 255);
            } else {
                speed = integer(v[0], 255);
                age = integer(v[1], 255);
            }
            const int horizon = horizon_index(v[keys - 1]);
            const auto key = risk_key(segment, sprite, size, speed, age);
            require(seen.emplace(key, horizon).second);
            const auto count = integer(v[keys], 9007199254740991LL);
            require(v[keys + 1] >= 0 && v[keys + 1] <= std::numeric_limits<float>::max());
            if (keys == 6) {
                require(v[8] >= 0 && v[9] >= 0 && v[9] <= 1);
            }
            if (keys == 3 || count >= 200)
                destination[key][horizon] = float(v[keys + 1]);
        });
    };
    load("risk_table_merged.tsv", "seg\tsprite\tsize10\tsb\tab\th\tn\tq99\tq999\tp_gt2", 6,
         tables.fine);
    load("risk_backoff_seg.tsv", "seg\tsb\tab\th\tn\tq99", 4, tables.segment);
    load("risk_backoff_global.tsv", "sb\tab\th\tn\tq99", 3, tables.global);
    std::set<std::array<std::int64_t, 3>> seen;
    rows(root / "nowarn_merged.tsv", "seg\tcx\tcy\tcount\trate", [&](const auto &v) {
        const auto segment = integer(v[0], 65535), x = integer(v[1], 23), y = integer(v[2], 27);
        require(seen.insert({segment, x, y}).second);
        integer(v[3], 9007199254740991LL);
        require(v[4] >= 0 && v[4] <= std::numeric_limits<float>::max());
        tables.spawn[int(segment)][y * 24 + x] = float(v[4]);
    });
    tables.loaded = true;
    return tables;
}
} // namespace th08::online
