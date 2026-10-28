#include "th08/phase_archive.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>

namespace th08::phase {
namespace {
bool hash(const std::string &s) {
    return s.size() == 64 && std::all_of(s.begin(), s.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}
bool label(const std::string &s) {
    return !s.empty() && s.size() <= 128 && std::all_of(s.begin(), s.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               c == '_' || c == '-' || c == '.';
    });
}
} // namespace
std::vector<Candidate> frontier(std::vector<Candidate> input, unsigned width,
                                std::uint64_t generation) {
    if (!width || width > 64 || input.size() > 4096)
        throw std::runtime_error("invalid_phase_frontier_budget");
    std::set<std::string> ids;
    std::string boundary;
    std::map<std::string, std::vector<Candidate>> families;
    std::map<std::pair<std::string, std::string>, Candidate> seen;
    std::sort(input.begin(), input.end(), [](const auto &a, const auto &b) { return a.id < b.id; });
    for (auto &c : input) {
        if (!label(c.id) || !label(c.boundary) || !label(c.family) || !hash(c.request_key) ||
            !hash(c.checkpoint_key) || !ids.insert(c.id).second)
            throw std::runtime_error("invalid_phase_candidate_identity");
        if (!c.completed || !c.no_miss || c.bombs)
            continue;
        if (boundary.empty())
            boundary = c.boundary;
        if (boundary != c.boundary)
            throw std::runtime_error("mixed_phase_archive_boundaries");
        const auto key = std::make_pair(c.request_key, c.checkpoint_key);
        const auto it = seen.find(key);
        if (it != seen.end()) {
            if (it->second.score != c.score || it->second.family != c.family)
                throw std::runtime_error("conflicting_phase_candidate_result");
            continue;
        }
        seen.emplace(key, c);
        families[c.family].push_back(std::move(c));
    }
    std::vector<std::vector<Candidate>> groups;
    for (auto &pair : families) {
        auto &v = pair.second;
        std::sort(v.begin(), v.end(), [](const auto &a, const auto &b) {
            return a.score != b.score ? a.score > b.score : a.id < b.id;
        });
        groups.push_back(std::move(v));
    }
    std::vector<Candidate> result;
    if (groups.empty())
        return result;
    const auto first = std::size_t(generation % groups.size());
    for (std::size_t depth = 0; result.size() < width; ++depth) {
        bool found = false;
        for (std::size_t i = 0; i < groups.size() && result.size() < width; ++i) {
            const auto &v = groups[(first + i) % groups.size()];
            if (depth < v.size()) {
                result.push_back(v[depth]);
                found = true;
            }
        }
        if (!found)
            break;
    }
    return result;
}
} // namespace th08::phase
