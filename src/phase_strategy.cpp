#include "th08/phase_strategy.hpp"
#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <tuple>

namespace th08::phase {
namespace {
int integer(const std::string &s) {
    int value = 0;
    const auto r = std::from_chars(s.data(), s.data() + s.size(), value);
    if (s.empty() ||
        !std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; }) ||
        r.ec != std::errc() || r.ptr != s.data() + s.size() || value < 0)
        throw std::runtime_error("invalid_phase_integer");
    return value;
}
std::vector<std::string> split(const std::string &text, char delimiter) {
    std::vector<std::string> parts;
    std::size_t first = 0;
    do {
        const auto end = text.find(delimiter, first);
        parts.push_back(text.substr(first, end == std::string::npos ? end : end - first));
        if (end == std::string::npos)
            break;
        first = end + 1;
    } while (true);
    return parts;
}
auto key(const Strategy &s) {
    return std::make_pair(s.stage, s.card);
}
} // namespace
Plan::Plan(std::vector<Strategy> strategies) : entries_(std::move(strategies)) {
    if (entries_.size() > 32)
        throw std::runtime_error("too_many_phase_strategies");
    for (const auto &s : entries_)
        if (s.stage < 0 || s.stage > 8 || s.card < 1 || s.card > 256 || s.top < 0 || s.top > 432 ||
            s.rows < 1 || s.rows > 14 || s.bottom() > 432 || s.reserve > 10000 ||
            (s.fire != Fire::Feedback && s.fire != Fire::Continuous) ||
            (s.capture != Capture::Human && s.capture != Capture::Youkai))
            throw std::runtime_error("invalid_phase_strategy");
    std::sort(entries_.begin(), entries_.end(),
              [](const auto &a, const auto &b) { return key(a) < key(b); });
    for (std::size_t i = 1; i < entries_.size(); ++i)
        if (key(entries_[i - 1]) == key(entries_[i]))
            throw std::runtime_error("duplicate_phase_strategy");
}
const Strategy *Plan::find(int stage, int spell) const {
    if (spell < 0 || spell >= 256)
        return nullptr;
    const auto wanted = std::make_pair(stage, spell + 1);
    const auto it = std::lower_bound(entries_.begin(), entries_.end(), wanted,
                                     [](const Strategy &s, const auto &k) { return key(s) < k; });
    return it != entries_.end() && key(*it) == wanted ? &*it : nullptr;
}
Plan parse(const std::string &text) {
    if (text == "none")
        return Plan({});
    if (text.empty() || text.size() > 4096)
        throw std::runtime_error("invalid_phase_plan_size");
    std::vector<Strategy> strategies;
    for (const auto &row : split(text, ',')) {
        const auto p = split(row, ':');
        if (p.size() != 7 || (p[5] != "feedback" && p[5] != "shoot") ||
            (p[6] != "human" && p[6] != "youkai"))
            throw std::runtime_error("invalid_phase_plan_row");
        strategies.push_back({integer(p[0]), integer(p[1]), integer(p[2]), integer(p[3]),
                              unsigned(integer(p[4])),
                              p[5] == "shoot" ? Fire::Continuous : Fire::Feedback,
                              p[6] == "human" ? Capture::Human : Capture::Youkai});
    }
    return Plan(std::move(strategies));
}
std::uint16_t propose(const Strategy &s, int remaining, float y, bool extreme,
                      std::uint16_t input) {
    // All movement/dialogue bits survive. Source input sampling, form changes,
    // shooting, option lifecycle and collision are executed by Native afterward.
    if (remaining <= int(s.reserve))
        return std::uint16_t((input & ~5u) | (s.capture == Capture::Youkai ? 5u : 1u));
    if (y > s.bottom())
        return std::uint16_t(input & ~5u);
    return std::uint16_t((input & ~5u) | 4u | ((s.fire == Fire::Continuous || !extreme) ? 1u : 0u));
}
} // namespace th08::phase
