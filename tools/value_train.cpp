#include "th08/online_value.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <openssl/evp.h>
#include <set>
#include <sstream>
#include <stdexcept>

namespace on = th08::online;
namespace {
std::string hash(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    auto context =
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!input || !context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("cannot_hash_training_input");
    std::array<char, 65536> buffer;
    while (input) {
        input.read(buffer.data(), buffer.size());
        if (EVP_DigestUpdate(context.get(), buffer.data(), std::size_t(input.gcount())) != 1)
            throw std::runtime_error("training_digest_failure");
    }
    std::array<unsigned char, 32> digest;
    unsigned length = 0;
    if (!input.eof() || EVP_DigestFinal_ex(context.get(), digest.data(), &length) != 1 ||
        length != 32)
        throw std::runtime_error("training_digest_failure");
    std::ostringstream output;
    for (auto value : digest)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(value);
    return output.str();
}
std::vector<double> masses(const std::vector<on::BranchExample> &rows) {
    std::map<std::string, std::size_t> counts;
    for (const auto &row : rows)
        ++counts[row.episode];
    std::vector<double> result;
    for (const auto &row : rows)
        result.push_back(1.0 / (double(counts.size()) * counts.at(row.episode)));
    return result;
}
std::set<unsigned> seeds(const std::vector<on::BranchExample> &rows) {
    std::set<unsigned> result;
    for (const auto &row : rows)
        result.insert(row.seed);
    return result;
}
void metrics(const char *name, const on::BranchValueModel &model,
             const std::vector<on::BranchExample> &rows) {
    const auto mass = masses(rows);
    double loss = 0, brier = 0;
    std::size_t positive = 0;
    std::set<std::string> parents;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto &row = rows[i];
        const auto p = std::clamp(model.probability(row.features), 1e-12, 1 - 1e-12);
        const double y = row.survived ? 1 : 0;
        loss -= mass[i] * (y * std::log(p) + (1 - y) * std::log(1 - p));
        brier += mass[i] * (p - y) * (p - y);
        positive += row.survived;
        parents.insert(row.episode);
    }
    std::cout << '"' << name << "\":{\"rows\":" << rows.size() << ",\"positive\":" << positive
              << ",\"episodes\":" << parents.size() << ",\"seeds\":" << seeds(rows).size()
              << ",\"episode_weighted_bce\":" << loss << ",\"episode_weighted_brier\":" << brier
              << '}';
}
} // namespace
int main(int argc, char **argv) {
    try {
        if (argc != 4)
            throw std::runtime_error("usage: th08_value_train TRAIN.tsv TUNE.tsv WEIGHTS.tsv");
        const auto train = on::read_branch_examples(argv[1]);
        const auto tune = on::read_branch_examples(argv[2]);
        const auto training_seeds = seeds(train);
        for (const auto seed : seeds(tune))
            if (training_seeds.count(seed))
                throw std::runtime_error("training_tuning_seed_overlap");
        std::set<std::string> parents;
        for (const auto &row : train)
            parents.insert(row.episode);
        for (const auto &row : tune)
            if (parents.count(row.episode))
                throw std::runtime_error("training_tuning_parent_overlap");
        const auto positives =
            std::count_if(train.begin(), train.end(), [](const auto &r) { return r.survived; });
        if (positives == 0 || std::size_t(positives) == train.size())
            throw std::runtime_error("degenerate_training_labels");
        const auto mass = masses(train);
        on::BranchValueModel model;
        model.loaded = true;
        for (unsigned feature = 0; feature < on::kValueFeatures; ++feature) {
            for (std::size_t i = 0; i < train.size(); ++i)
                model.mean[feature] += mass[i] * train[i].features[feature];
            double variance = 0;
            for (std::size_t i = 0; i < train.size(); ++i) {
                const double d = train[i].features[feature] - model.mean[feature];
                variance += mass[i] * d * d;
            }
            model.scale[feature] = variance > 1e-10 ? std::sqrt(variance) : 1;
        }
        std::vector<std::array<double, on::kValueFeatures>> normalized(train.size());
        for (std::size_t i = 0; i < train.size(); ++i)
            for (unsigned j = 0; j < on::kValueFeatures; ++j)
                normalized[i][j] = on::normalized_value_feature(train[i].features[j], model.mean[j],
                                                                model.scale[j]);
        constexpr unsigned epochs = 400;
        constexpr double rate = .05, l2 = .01;
        for (unsigned epoch = 0; epoch < epochs; ++epoch) {
            double bias_gradient = 0;
            std::array<double, on::kValueFeatures> gradient{};
            for (std::size_t i = 0; i < train.size(); ++i) {
                double z = model.bias;
                for (unsigned j = 0; j < on::kValueFeatures; ++j)
                    z += model.weight[j] * normalized[i][j];
                const double p = z >= 0 ? 1 / (1 + std::exp(-z)) : std::exp(z) / (1 + std::exp(z));
                const double delta = mass[i] * (p - double(train[i].survived));
                bias_gradient += delta;
                for (unsigned j = 0; j < on::kValueFeatures; ++j)
                    gradient[j] += delta * normalized[i][j];
            }
            model.bias -= rate * bias_gradient;
            for (unsigned j = 0; j < on::kValueFeatures; ++j)
                model.weight[j] -= rate * (gradient[j] + l2 * model.weight[j]);
        }
        on::save_branch_value(model, argv[3]);
        const auto saved = on::load_branch_value(argv[3]);
        std::cout << std::setprecision(17)
                  << "{\"schema_version\":1,\"label\":\"conditional-held-last-branch-64\","
                  << "\"input_horizon\":24,\"label_updates\":64,\"features\":24,\"epochs\":"
                  << epochs << ",\"learning_rate\":" << rate << ",\"l2\":" << l2
                  << ",\"weighting\":\"equal-parent-episode\",\"normalization\":\"training-only-z-"
                     "clipped-at-8\","
                  << "\"train_sha256\":\"" << hash(argv[1]) << "\",\"tune_sha256\":\""
                  << hash(argv[2]) << "\",\"weights_sha256\":\"" << hash(argv[3]) << "\",";
        metrics("train", saved, train);
        std::cout << ',';
        metrics("tune", saved, tune);
        std::cout << "}\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
