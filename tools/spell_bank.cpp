#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}
struct Input {
    std::string bytes, sha256;
};
Input read(const char *path) {
    std::ifstream file(path, std::ios::binary);
    require(bool(file), "cannot read input");
    Input result{std::string(std::istreambuf_iterator<char>(file), {}), {}};
    require(!file.bad() && !result.bytes.empty(), "empty or unreadable input");
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned size = 0;
    require(EVP_Digest(result.bytes.data(), result.bytes.size(), digest.data(), &size, EVP_sha256(),
                       nullptr) == 1 &&
                size == 32,
            "cannot hash input");
    std::ostringstream hash;
    for (unsigned i = 0; i < size; ++i)
        hash << std::hex << std::setw(2) << std::setfill('0') << unsigned(digest[i]);
    result.sha256 = hash.str();
    return result;
}
template <class T> T integer(const std::string &text) {
    T value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    require(result.ec == std::errc{} && result.ptr == text.data() + text.size(), "invalid integer");
    return value;
}
std::vector<std::string> fields(const std::string &line) {
    std::vector<std::string> result;
    std::size_t begin = 0;
    for (;;) {
        const auto end = line.find('\t', begin);
        result.push_back(line.substr(begin, end == std::string::npos ? end : end - begin));
        if (end == std::string::npos)
            return result;
        begin = end + 1;
    }
}
std::vector<std::string> lines(const Input &input) {
    std::istringstream stream(input.bytes);
    std::vector<std::string> result;
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        require(!line.empty(), "blank record");
        result.push_back(line);
    }
    return result;
}
struct SpellCase {
    int stage, difficulty, shot, spell;
    bool operator<(const SpellCase &other) const {
        return std::tie(stage, difficulty, shot, spell) <
               std::tie(other.stage, other.difficulty, other.shot, other.spell);
    }
};
SpellCase scenario(const std::vector<std::string> &row, std::size_t offset) {
    const SpellCase result{integer<int>(row[offset]), integer<int>(row[offset + 1]),
                           integer<int>(row[offset + 2]), integer<int>(row[offset + 3])};
    require(result.stage >= 0 && result.stage <= 8 && result.difficulty >= 0 &&
                result.difficulty <= 4 && result.shot >= 0 && result.shot <= 11 &&
                result.spell >= 0,
            "invalid checkpoint identity");
    return result;
}
std::vector<SpellCase> cases(const Input &input) {
    const auto records = lines(input);
    require(records.front() == "stage\tdifficulty\tshot\tspell", "invalid checkpoint header");
    std::vector<SpellCase> result;
    std::set<SpellCase> unique;
    for (std::size_t i = 1; i < records.size(); ++i) {
        const auto row = fields(records[i]);
        require(row.size() == 4, "invalid checkpoint record");
        const auto entry = scenario(row, 0);
        require(unique.insert(entry).second, "duplicate checkpoint");
        result.push_back(entry);
    }
    require(!result.empty(), "empty checkpoint protocol");
    return result;
}
std::set<int> seeds(const Input &input) {
    std::set<int> result;
    for (const auto &line : lines(input)) {
        const auto seed = integer<int>(line);
        require(seed >= 0 && seed <= 65535, "seed outside native domain");
        require(result.insert(seed).second, "duplicate protocol seed");
    }
    require(!result.empty(), "empty seed protocol");
    return result;
}
bool label(const std::string &text) {
    return !text.empty() && text.size() <= 64 &&
           std::all_of(text.begin(), text.end(), [](unsigned char c) {
               return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                      c == '-' || c == '_' || c == '.';
           });
}
bool hash(const std::string &text) {
    return text.size() == 64 && std::all_of(text.begin(), text.end(), [](unsigned char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}
enum class Outcome { complete, death, budget };
using TrialKey = std::tuple<SpellCase, int, bool>;
struct Trials {
    std::map<TrialKey, Outcome> entries;
    std::uint64_t frames = 0;
};
Trials trials(const Input &input, const std::vector<SpellCase> &expected_cases,
              const std::set<int> &expected_seeds, const std::string &baseline,
              const std::string &candidate) {
    const auto records = lines(input);
    require(records.front() == "profile\tstage\tdifficulty\tshot\tspell\tseed\toutcome\tframes\t"
                               "actions_sha256",
            "invalid trial header");
    const std::set<SpellCase> allowed(expected_cases.begin(), expected_cases.end());
    Trials result;
    for (std::size_t i = 1; i < records.size(); ++i) {
        const auto row = fields(records[i]);
        require(row.size() == 9, "truncated or extra trial fields");
        require(row[0] == baseline || row[0] == candidate, "unexpected trial profile");
        const auto entry = scenario(row, 1);
        const auto seed = integer<int>(row[5]);
        require(allowed.count(entry) && expected_seeds.count(seed),
                "trial outside frozen protocol");
        require(row[6] == "complete" || row[6] == "death" || row[6] == "budget",
                "invalid trial outcome; runtime/setup failures need investigation");
        const auto outcome = row[6] == "complete" ? Outcome::complete
                             : row[6] == "death"  ? Outcome::death
                                                  : Outcome::budget;
        const auto frames = integer<std::uint64_t>(row[7]);
        require(frames > 0 && frames <= std::numeric_limits<std::uint64_t>::max() - result.frames,
                "invalid trial frame count");
        require(hash(row[8]), "invalid action identity");
        const TrialKey key{entry, seed, row[0] == candidate};
        require(result.entries.emplace(key, outcome).second, "duplicate trial");
        result.frames += frames;
    }
    return result;
}
std::array<double, 2> wilson(std::size_t complete, std::size_t total) {
    constexpr double z = 1.959963984540054;
    const double n = double(total), p = double(complete) / n, divisor = 1 + z * z / n;
    const double center = (p + z * z / (2 * n)) / divisor;
    const double radius = z * std::sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / divisor;
    return {std::max(0.0, center - radius), std::min(1.0, center + radius)};
}
double mcnemar(std::size_t baseline_only, std::size_t candidate_only) {
    if (baseline_only == candidate_only)
        return 1;
    const auto n = baseline_only + candidate_only, m = std::min(baseline_only, candidate_only);
    // Sum the exact binomial tail backward from its largest term. Starting at
    // 2^-n instead would underflow even for moderate balanced seed banks.
    double term = std::exp(std::lgamma(double(n) + 1) - std::lgamma(double(m) + 1) -
                           std::lgamma(double(n - m) + 1) - double(n) * std::log(2.0));
    double sum = term;
    for (auto k = m; k > 0; --k) {
        term *= double(k) / double(n - k + 1);
        sum += term;
    }
    return std::min(1.0, 2 * sum);
}
struct Summary {
    SpellCase scenario;
    std::array<std::size_t, 4> paired{}; // neither, baseline only, candidate only, both
    std::array<std::size_t, 2> budgets{};
};
std::vector<Summary> summarize(const Trials &trials, const std::vector<SpellCase> &expected_cases,
                               const std::set<int> &expected_seeds) {
    std::vector<Summary> result;
    for (const auto &entry : expected_cases) {
        Summary summary{entry, {}, {}};
        for (const auto seed : expected_seeds) {
            std::array<Outcome, 2> outcomes;
            for (unsigned arm = 0; arm < 2; ++arm) {
                const auto found = trials.entries.find({entry, seed, arm != 0});
                require(found != trials.entries.end(), "missing pair from frozen denominator");
                outcomes[arm] = found->second;
                summary.budgets[arm] += found->second == Outcome::budget;
            }
            const auto pair = unsigned(outcomes[0] == Outcome::complete) |
                              (unsigned(outcomes[1] == Outcome::complete) << 1);
            ++summary.paired[pair];
        }
        result.push_back(summary);
    }
    return result;
}
} // namespace

int main(int argc, char **argv) {
    try {
        require(argc == 7, "usage: th08_spell_bank TRIALS SEEDS CASES PROTOCOL BASELINE CANDIDATE");
        const std::string baseline = argv[5], candidate = argv[6];
        require(label(baseline) && label(candidate) && baseline != candidate,
                "invalid profile labels");
        const auto rows = read(argv[1]), seed_input = read(argv[2]), case_input = read(argv[3]);
        const auto protocol = read(argv[4]);
        const auto expected_cases = cases(case_input);
        const auto expected_seeds = seeds(seed_input);
        const auto observed = trials(rows, expected_cases, expected_seeds, baseline, candidate);
        const auto summaries = summarize(observed, expected_cases, expected_seeds);
        std::cout << std::setprecision(17)
                  << "{\"schema_version\":1,\"scope\":\"paired-spell-practice-proxy\","
                     "\"equivalence_claim\":false,\"confidence_level\":0.95,\"baseline\":\""
                  << baseline << "\",\"candidate\":\"" << candidate << "\",\"trials_sha256\":\""
                  << rows.sha256 << "\",\"seeds_sha256\":\"" << seed_input.sha256
                  << "\",\"cases_sha256\":\"" << case_input.sha256 << "\",\"protocol_sha256\":\""
                  << protocol.sha256 << "\",\"trials\":" << observed.entries.size()
                  << ",\"native_updates\":" << observed.frames << ",\"cases\":[";
        for (std::size_t i = 0; i < summaries.size(); ++i) {
            const auto &s = summaries[i];
            const auto a = s.paired[1] + s.paired[3], b = s.paired[2] + s.paired[3];
            const auto aci = wilson(a, expected_seeds.size()),
                       bci = wilson(b, expected_seeds.size());
            std::cout << (i ? "," : "") << "{\"stage\":" << s.scenario.stage
                      << ",\"difficulty\":" << s.scenario.difficulty
                      << ",\"shot\":" << s.scenario.shot << ",\"spell\":" << s.scenario.spell
                      << ",\"paired_seeds\":" << expected_seeds.size()
                      << ",\"baseline_complete\":" << a << ",\"candidate_complete\":" << b
                      << ",\"both_complete\":" << s.paired[3]
                      << ",\"baseline_only\":" << s.paired[1]
                      << ",\"candidate_only\":" << s.paired[2]
                      << ",\"neither_complete\":" << s.paired[0]
                      << ",\"baseline_budget\":" << s.budgets[0]
                      << ",\"candidate_budget\":" << s.budgets[1] << ",\"baseline_wilson_95\":["
                      << aci[0] << ',' << aci[1] << "],\"candidate_wilson_95\":[" << bci[0] << ','
                      << bci[1]
                      << "],\"mcnemar_exact_two_sided\":" << mcnemar(s.paired[1], s.paired[2])
                      << '}';
        }
        std::cout << "]}\n";
        require(bool(std::cout), "cannot write summary");
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "th08_spell_bank: " << error.what() << '\n';
        return 2;
    }
}
