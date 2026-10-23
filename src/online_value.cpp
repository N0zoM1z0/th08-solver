#include "th08/online_value.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <tuple>

namespace th08::online {
namespace {
void require(bool value, const char *message = "invalid_branch_value") {
    if (!value)
        throw std::runtime_error(message);
}
constexpr const char *version = "th08_branch_value_v1\t24\t64\t24";
constexpr const char *dataset_version = "th08_branch_dataset_v1\t24\t64\t24";
double finite(const std::string &text) {
    std::size_t used = 0;
    const auto value = std::stod(text, &used);
    require(used == text.size() && std::isfinite(value));
    return value;
}
std::uint64_t integer(const std::string &text, std::uint64_t maximum) {
    std::uint64_t value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    require(!text.empty() && parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size() &&
            value <= maximum);
    return value;
}
std::vector<std::string> columns(const std::string &line) {
    std::vector<std::string> result;
    std::size_t start = 0;
    while (true) {
        const auto end = line.find('\t', start);
        result.push_back(line.substr(start, end - start));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return result;
}
bool digest(const std::string &value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}
} // namespace
double normalized_value_feature(float value, double mean, double scale) {
    require(std::isfinite(value) && std::isfinite(mean) && std::isfinite(scale) && scale > 0);
    return std::clamp((double(value) - mean) / scale, -8.0, 8.0);
}
double BranchValueModel::probability(const ValueFeatures &features) const {
    require(loaded && std::isfinite(bias));
    double z = bias;
    for (unsigned i = 0; i < kValueFeatures; ++i) {
        require(std::isfinite(weight[i]));
        z += weight[i] * normalized_value_feature(features[i], mean[i], scale[i]);
    }
    require(std::isfinite(z));
    if (z >= 0)
        return 1.0 / (1.0 + std::exp(-z));
    const auto exponential = std::exp(z);
    return exponential / (1.0 + exponential);
}
BranchValueModel load_branch_value(const std::string &path) {
    std::ifstream input(path);
    std::string line;
    require(bool(input) && bool(std::getline(input, line)) && line == version);
    require(bool(std::getline(input, line)) && line == "feature\tmean\tscale\tweight");
    BranchValueModel model;
    for (unsigned i = 0; i <= kValueFeatures; ++i) {
        require(bool(std::getline(input, line)));
        const auto values = columns(line);
        require(values.size() == 4 && values[0] == (i == 0 ? "bias" : kValueFeatureNames[i - 1]));
        const auto mean = finite(values[1]), scale = finite(values[2]), weight = finite(values[3]);
        require(scale > 0 && std::fabs(mean) <= 1e6 && scale <= 1e6 && std::fabs(weight) <= 1e6);
        if (i == 0) {
            require(mean == 0 && scale == 1);
            model.bias = weight;
        } else {
            model.mean[i - 1] = mean;
            model.scale[i - 1] = scale;
            model.weight[i - 1] = weight;
        }
    }
    require(!std::getline(input, line) && input.eof());
    model.loaded = true;
    return model;
}
void save_branch_value(const BranchValueModel &model, const std::string &path) {
    model.probability({}); // Validate before opening the output.
    require(std::fabs(model.bias) <= 1e6);
    for (unsigned i = 0; i < kValueFeatures; ++i)
        require(std::fabs(model.mean[i]) <= 1e6 && model.scale[i] <= 1e6 &&
                std::fabs(model.weight[i]) <= 1e6);
    require(!std::filesystem::exists(path), "branch_value_output_exists");
    std::ofstream output(path);
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output << std::setprecision(17) << version << "\nfeature\tmean\tscale\tweight\nbias\t0\t1\t"
           << model.bias << '\n';
    for (unsigned i = 0; i < kValueFeatures; ++i)
        output << kValueFeatureNames[i] << '\t' << model.mean[i] << '\t' << model.scale[i] << '\t'
               << model.weight[i] << '\n';
}
std::string branch_dataset_header() {
    std::string header = "episode\tseed\tstage\tdifficulty\tshot\tspell\tupdate\tproposal\tstatus\t"
                         "native_updates\tprefix_sha256";
    for (const auto *name : kValueFeatureNames)
        header += '\t' + std::string(name);
    return header;
}
std::vector<BranchExample> read_branch_examples(const std::string &path) {
    std::ifstream input(path);
    std::string line;
    require(bool(input) && bool(std::getline(input, line)) && line == dataset_version,
            "invalid_branch_dataset");
    require(bool(std::getline(input, line)) && line == branch_dataset_header());
    std::vector<BranchExample> rows;
    std::set<std::tuple<std::string, std::uint64_t, unsigned>> seen;
    std::map<std::string, std::array<unsigned, 5>> episodes;
    while (std::getline(input, line)) {
        require(line.size() <= 16384 && rows.size() < 100000);
        const auto fields = columns(line);
        require(fields.size() == 11 + kValueFeatures);
        BranchExample row;
        require(digest(fields[0]) && digest(fields[10]));
        row.episode = fields[0];
        row.prefix = fields[10];
        row.seed = unsigned(integer(fields[1], 65535));
        row.stage = unsigned(integer(fields[2], 8));
        row.difficulty = unsigned(integer(fields[3], 4));
        row.shot = unsigned(integer(fields[4], 11));
        row.spell = unsigned(integer(fields[5], 221));
        const std::array<unsigned, 5> identity{row.seed, row.stage, row.difficulty, row.shot,
                                               row.spell};
        const auto episode = episodes.emplace(row.episode, identity);
        require(episode.second || episode.first->second == identity,
                "inconsistent_branch_parent_identity");
        row.update = integer(fields[6], 2000000);
        row.proposal = unsigned(integer(fields[7], 3));
        row.native_updates = unsigned(integer(fields[9], kValueLabelUpdates));
        require(row.native_updates > 0 &&
                seen.emplace(row.episode, row.update, row.proposal).second);
        require(fields[8] == "survived" || fields[8] == "death" || fields[8] == "complete",
                "failed_or_incomplete_branch_cannot_label_training_data");
        require(fields[8] != "survived" || row.native_updates == kValueLabelUpdates);
        row.survived = fields[8] != "death";
        for (unsigned i = 0; i < kValueFeatures; ++i) {
            const double value = finite(fields[11 + i]);
            require(std::fabs(value) <= 1e6);
            row.features[i] = float(value);
        }
        rows.push_back(std::move(row));
    }
    require(input.eof() && !rows.empty());
    return rows;
}
} // namespace th08::online
