#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace th08::online {
constexpr unsigned kValueFeatures = 24, kValueHorizon = 24, kValueLabelUpdates = 64;
using ValueFeatures = std::array<float, kValueFeatures>;
inline constexpr std::array<const char *, kValueFeatures> kValueFeatureNames{"x",
                                                                             "y",
                                                                             "travel_x",
                                                                             "travel_y",
                                                                             "wall_left",
                                                                             "wall_right",
                                                                             "wall_top",
                                                                             "wall_bottom",
                                                                             "last_dx",
                                                                             "last_dy",
                                                                             "focused",
                                                                             "base_score",
                                                                             "local_real",
                                                                             "local_ghost",
                                                                             "local_spawning",
                                                                             "local_actors",
                                                                             "local_canonical_gap",
                                                                             "forecast_boxes",
                                                                             "forecast_lasers",
                                                                             "forecast_risk",
                                                                             "youkai",
                                                                             "global_bullets",
                                                                             "global_ghosts",
                                                                             "global_lasers"};

// Probability for one proposed H24 input prefix followed by its held final
// input, executed natively through update 64 or the declared episode terminal.
// This is a conditional branch heuristic, not optimal state value or a safety certificate.
struct BranchValueModel {
    std::array<double, kValueFeatures> mean{}, scale{}, weight{};
    double bias = 0;
    bool loaded = false;
    double probability(const ValueFeatures &) const;
};
BranchValueModel load_branch_value(const std::string &path);
void save_branch_value(const BranchValueModel &, const std::string &path);
double normalized_value_feature(float value, double mean, double scale);

struct ValueProposal {
    std::vector<std::uint16_t> inputs; // Full gameplay bits, always shooting; no bomb.
    ValueFeatures features;
};
struct BranchExample {
    std::string episode, prefix;
    unsigned seed = 0, stage = 0, difficulty = 0, shot = 0, spell = 0;
    std::uint64_t update = 0;
    unsigned proposal = 0, native_updates = 0;
    bool survived = false;
    ValueFeatures features;
};
std::string branch_dataset_header();
std::vector<BranchExample> read_branch_examples(const std::string &path);
} // namespace th08::online
