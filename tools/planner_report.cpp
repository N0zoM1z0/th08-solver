// Compose the bounded campaign ledger from completed process artifacts. Unknown
// fields and private paths never become public evidence; unfinished jobs supply
// only a pending ID. Verification checks execution agreement, not retail physics.
#include "headless_report.hpp"
#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace fs = std::filesystem;
using Fields = std::map<std::string, std::string>;

void require(bool condition, const char *reason) {
    if (!condition)
        throw std::runtime_error(reason);
}

bool identifier(const std::string &text) {
    return !text.empty() && text.size() <= 128 &&
           std::all_of(text.begin(), text.end(), [](unsigned char c) {
               return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                      c == '_' || c == '-';
           });
}

bool hex(const std::string &text, std::size_t size) {
    return text.size() == size && std::all_of(text.begin(), text.end(), [](char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

std::string quote(const std::string &safe) {
    return '"' + safe + '"';
}

std::string string_value(const std::string &raw) {
    require(raw.size() >= 2 && raw.front() == '"' && raw.back() == '"', "expected_string");
    return raw.substr(1, raw.size() - 2);
}

std::string read(const fs::path &path) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "input_unreadable");
    std::string result;
    std::array<char, 8192> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        require(result.size() + std::size_t(input.gcount()) <= 2 * 1024 * 1024, "input_too_large");
        result.append(buffer.data(), std::size_t(input.gcount()));
    }
    require(input.eof(), "input_read_failed");
    return result;
}

Fields parse(const std::string &text) {
    return th08::headless_report::Document(text).parse();
}

std::string digest(const std::string &text) {
    std::array<unsigned char, EVP_MAX_MD_SIZE> bytes{};
    unsigned length = 0;
    require(EVP_Digest(text.data(), text.size(), bytes.data(), &length, EVP_sha256(), nullptr) ==
                    1 &&
                length == 32,
            "sha256_failed");
    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (unsigned i = 0; i < length; ++i)
        output << std::setw(2) << unsigned(bytes[i]);
    return output.str();
}

std::uint64_t integer(const std::string &raw) {
    std::uint64_t result = 0;
    const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), result);
    require(!raw.empty() && converted.ec == std::errc{} && converted.ptr == raw.data() + raw.size(),
            "expected_unsigned_integer");
    return result;
}

double numeric(const std::string &raw) {
    require(!raw.empty() && (raw.front() == '-' || (raw.front() >= '0' && raw.front() <= '9')),
            "expected_number");
    char *end = nullptr;
    const double result = std::strtod(raw.c_str(), &end);
    require(end == raw.c_str() + raw.size() && std::isfinite(result), "invalid_number");
    return result;
}

const std::string &field(const Fields &object, const std::string &key) {
    const auto found = object.find(key);
    require(found != object.end(), "missing_required_field");
    return found->second;
}

void add(std::uint64_t &total, std::uint64_t value) {
    require(value <= std::numeric_limits<std::uint64_t>::max() - total, "counter_overflow");
    total += value;
}

void emit(std::ostream &out, const Fields &fields) {
    out << '{';
    bool first = true;
    for (const auto &entry : fields) {
        out << (first ? "" : ",") << quote(entry.first) << ':' << entry.second;
        first = false;
    }
    out << '}';
}

// The input document has already been validated, including all nested values.
// Split only an array of objects needed for fidelity cases/stage summaries.
std::vector<Fields> objects(const std::string &raw) {
    const auto begin = raw.find_first_not_of(" \t\r\n");
    const auto end = raw.find_last_not_of(" \t\r\n");
    require(begin != std::string::npos && raw[begin] == '[' && raw[end] == ']', "expected_array");
    std::vector<Fields> result;
    std::size_t start = begin + 1;
    bool quoted = false, escaped = false;
    int depth = 0;
    for (std::size_t i = start; i < end; ++i) {
        const char c = raw[i];
        if (escaped)
            escaped = false;
        else if (quoted && c == '\\')
            escaped = true;
        else if (c == '"')
            quoted = !quoted;
        else if (!quoted) {
            if (c == '{' || c == '[')
                ++depth;
            else if (c == '}' || c == ']')
                --depth;
            else if (c == ',' && depth == 0) {
                result.push_back(parse(raw.substr(start, i - start)));
                start = i + 1;
            }
        }
    }
    if (raw.find_first_not_of(" \t\r\n", start) < end)
        result.push_back(parse(raw.substr(start, end - start)));
    return result;
}

Fields scalars(const Fields &input, const std::vector<std::string> &keys) {
    static const std::set<std::string> text_keys = {"id",
                                                    "outcome",
                                                    "failure",
                                                    "profile",
                                                    "beam_retention",
                                                    "movement_model",
                                                    "proposal_pruning",
                                                    "acceptance",
                                                    "hazard_probe_geometry"};
    Fields result;
    for (const auto &key : keys) {
        const auto found = input.find(key);
        if (found == input.end())
            continue;
        const auto &value = found->second;
        // String-valued allowlisted fields must be identifiers or actual hashes.
        // Numeric/bool producer fields cannot smuggle private strings/objects.
        if (!value.empty() && value.front() == '"') {
            const auto text = string_value(value);
            if (key.find("sha256") != std::string::npos)
                require((key == "sha256" && text.empty()) || hex(text, 64), "invalid_hash");
            else if (key == "source_revision")
                require(hex(text, 40), "invalid_source_revision");
            else {
                require(text_keys.count(key), "unexpected_string_field");
                require(identifier(text) || text.empty(), "unsafe_scalar_string");
            }
        } else if (value != "true" && value != "false")
            numeric(value);
        result.emplace(key, value);
    }
    return result;
}

const std::vector<std::string> option_keys = {
    "stage",          "spell",       "difficulty", "seed",        "rollout", "beam",
    "commit",         "checkpoint",  "keep",       "attempts",    "cegar",   "frames",
    "seconds",        "shoot",       "story",      "until_stage", "shot",    "measure",
    "diversity_cell", "boss_follow", "verify"};

const std::vector<std::string> cost_keys = {"rollouts",
                                            "searches",
                                            "cegar_replans",
                                            "backtracks",
                                            "retries",
                                            "releases",
                                            "quiet_windows",
                                            "model_breaks",
                                            "native_update_attempts",
                                            "native_updates",
                                            "incomplete_native_updates",
                                            "rollout_update_attempts",
                                            "rollout_updates",
                                            "commit_updates",
                                            "rollout_failures",
                                            "diversity_selections",
                                            "diversity_candidates",
                                            "diversity_bins",
                                            "diversity_retained_bins",
                                            "probes",
                                            "probe_rng_diverged"};

const std::vector<std::string> plan_keys = {"outcome",
                                            "source_revision",
                                            "profile",
                                            "shot",
                                            "stage",
                                            "spell",
                                            "difficulty",
                                            "seed",
                                            "frames",
                                            "wall_seconds",
                                            "max_frame",
                                            "score",
                                            "graze",
                                            "rng_seed",
                                            "rng_draws",
                                            "deaths",
                                            "rollout",
                                            "beam",
                                            "commit",
                                            "checkpoint",
                                            "story",
                                            "final_stage",
                                            "beam_retention",
                                            "diversity_selection_seconds",
                                            "native_seconds_excluding_file_io",
                                            "shared_capacity_bytes",
                                            "max_rollout",
                                            "movement_model",
                                            "proposal_pruning",
                                            "acceptance",
                                            "hazard_probe_geometry",
                                            "probe_rng_first_frame_mean",
                                            "same_hazards_1_16",
                                            "same_hazards_17_32",
                                            "same_hazards_33_48",
                                            "same_hazards_49_64",
                                            "binary_sha256",
                                            "dat_sha256"};

const std::vector<std::string> verify_keys = {"source_revision", "verified",
                                              "frames",          "complete_at",
                                              "death_updates",   "score",
                                              "stage",           "rng_seed",
                                              "rng_draws",       "native_update_attempts",
                                              "native_updates",  "native_seconds_excluding_file_io",
                                              "binary_sha256",   "dat_sha256"};

struct Case {
    std::string group, id, status = "pending", failure;
    Fields plan, options, verification, verification_options, rpy, identity;
    std::string plan_sha256, verify_sha256;
    int exit_code = -1;
};

struct Totals {
    std::uint64_t finalized = 0, pending = 0, complete = 0, failed = 0, unverified = 0;
    std::uint64_t setup_rejections = 0, missing_reports = 0;
    std::uint64_t solver_updates = 0, verification_updates = 0, rpy_rows = 0, rpy_steps = 0;
    double solver_wall_seconds = 0, solver_native_seconds = 0, verification_native_seconds = 0;
    std::map<std::string, std::uint64_t> cost_totals;
};

void account(const Fields &fields, const std::string &key, std::uint64_t &total) {
    const auto found = fields.find(key);
    if (found != fields.end())
        add(total, integer(found->second));
}

void seconds(const Fields &fields, const std::string &key, double &total) {
    const auto found = fields.find(key);
    if (found != fields.end()) {
        const auto value = numeric(found->second);
        require(value >= 0 && std::isfinite(total + value), "invalid_seconds");
        total += value;
    }
}

void verify_complete(Case &result, const fs::path &root) {
    const auto directory = result.group == "spell92-diversity-v3"
                               ? root / "spell92-verified" / result.id
                               : root / result.group / (result.id + "-verify");
    require(fs::is_regular_file(directory / "verify.json"), "missing_verification");
    const auto text = read(directory / "verify.json");
    const auto verification = parse(text);
    result.verify_sha256 = digest(text);
    result.verification = scalars(verification, verify_keys);
    if (verification.count("options"))
        result.verification_options = scalars(parse(field(verification, "options")), option_keys);
    require(field(verification, "verified") == "true", "verification_failed");
    const auto frames = integer(field(result.plan, "frames"));
    require(frames > 0 && integer(field(verification, "frames")) == frames &&
                integer(field(verification, "complete_at")) == frames,
            "verification_frame_mismatch");
    require(integer(field(verification, "death_updates")) == 0 &&
                numeric(field(result.plan, "deaths")) == 0,
            "verification_deaths");
    for (const auto *key : {"score", "rng_seed", "rng_draws"})
        require(integer(field(result.plan, key)) == integer(field(verification, key)),
                "verification_state_mismatch");
    for (const auto *key : {"source_revision", "binary_sha256", "dat_sha256"})
        if (result.plan.count(key) && verification.count(key))
            require(field(result.plan, key) == field(verification, key),
                    "verification_identity_mismatch");
    for (const auto *key : {"stage", "spell", "difficulty", "seed", "shot", "story", "until_stage"})
        if (result.options.count(key) && result.verification_options.count(key))
            require(field(result.options, key) == field(result.verification_options, key),
                    "verification_checkpoint_mismatch");
    if (numeric(field(result.plan, "spell")) < 0) {
        const auto path = root / result.group / (result.id + "-rpy.json");
        require(fs::is_regular_file(path), "missing_natural_replay");
        const auto replay = parse(read(path));
        result.rpy = scalars(replay, {"rows", "finished", "natural", "steps", "runtime_error",
                                      "io_error", "death_rows", "bomb_input_rows"});
        require(field(replay, "finished") == "true" && field(replay, "natural") == "true" &&
                    integer(field(replay, "rows")) == frames,
                "natural_replay_mismatch");
        for (const auto *key : {"runtime_error", "io_error"})
            require(!replay.count(key) || field(replay, key) == "false", "natural_replay_error");
        require(integer(field(replay, "death_rows")) == 0 &&
                    integer(field(replay, "bomb_input_rows")) == 0,
                "natural_replay_not_nmnb");
        const auto stages = objects(field(replay, "stages"));
        require(!stages.empty(), "natural_replay_missing_stages");
        // A continuous story has several stage summaries. Its terminal score
        // belongs to the final stage, just as a practice run's only score does.
        const auto &final_stage = stages.back();
        require(integer(field(final_stage, "endScore")) == integer(field(result.plan, "score")),
                "natural_replay_score_mismatch");
        result.rpy.emplace("endScore", field(final_stage, "endScore"));
    }
}

Case collect(const fs::path &root, const std::string &group, const std::string &id,
             const std::string &known_binary, Totals &totals) {
    Case result;
    result.group = group;
    result.id = id;
    const auto exit = root / group / (id + ".exit");
    if (!fs::is_regular_file(exit)) {
        ++totals.pending;
        return result;
    }
    try {
        auto code = read(exit);
        while (!code.empty() && (code.back() == '\n' || code.back() == '\r'))
            code.pop_back();
        const auto parsed_exit = integer(code);
        require(parsed_exit <= 255, "invalid_exit_code");
        result.exit_code = int(parsed_exit);
        const auto plan_path = root / group / id / "plan.json";
        if (!fs::is_regular_file(plan_path) && group == "spell92-diversity-v2" &&
            result.exit_code == 2 && fs::is_regular_file(root / group / (id + ".log"))) {
            auto log = read(root / group / (id + ".log"));
            while (!log.empty() && (log.back() == '\n' || log.back() == '\r'))
                log.pop_back();
            if (log == "invalid planner arguments: unknown option: --diversity-cell") {
                result.status = "setup_rejected";
                result.failure = "unknown_diversity_cell_option";
                ++totals.setup_rejections;
                return result;
            }
        }
        ++totals.finalized;
        if (!fs::is_regular_file(plan_path)) {
            result.status = "unverified_missing_report";
            result.failure = "missing_plan";
            ++totals.missing_reports;
            return result;
        }
        const auto text = read(plan_path);
        const auto plan = parse(text);
        result.plan_sha256 = digest(text);
        result.plan = scalars(plan, plan_keys);
        const auto costs = scalars(plan, cost_keys);
        result.plan.insert(costs.begin(), costs.end());
        if (plan.count("options"))
            result.options = scalars(parse(field(plan, "options")), option_keys);
        const auto outcome = string_value(field(result.plan, "outcome"));
        require(identifier(outcome), "invalid_outcome");
        if (result.plan.count("binary_sha256")) {
            result.identity.emplace("binary_sha256", field(result.plan, "binary_sha256"));
            result.identity.emplace("binary_origin", quote("producer"));
        } else if (!known_binary.empty() &&
                   ((group == "spell92-diversity-v3" && outcome == "complete") ||
                    group == "spell92-training-v1" || group == "practice-v1" ||
                    group == "advance-v1")) {
            result.identity.emplace("binary_sha256", quote(known_binary));
            result.identity.emplace("binary_origin", quote("retained_campaign_binary"));
        } else
            result.identity.emplace(
                "binary_origin",
                quote(group == "spell92-v1" ? "unretained_preliminary_binary" : "unrecorded"));
        account(plan, "native_updates", totals.solver_updates);
        seconds(plan, "wall_seconds", totals.solver_wall_seconds);
        seconds(plan, "native_seconds_excluding_file_io", totals.solver_native_seconds);
        for (const auto &key : cost_keys)
            account(plan, key, totals.cost_totals[key]);
        if (outcome != "complete") {
            result.status = "failed";
            ++totals.failed;
            return result;
        }
        result.status = "unverified_complete";
        require(result.exit_code == 0, "complete_process_failed");
        verify_complete(result, root);
        result.status = "verified_complete";
        ++totals.complete;
    } catch (const std::exception &error) {
        result.failure = error.what();
        // Only stable, identifier-shaped error codes are public. Parser and
        // filesystem exception text may contain local input/output locations.
        if (!identifier(result.failure))
            result.failure = "invalid_artifact";
        if (result.status == "unverified_complete")
            ++totals.unverified;
        else
            result.status = "invalid";
    }
    account(result.verification, "native_updates", totals.verification_updates);
    seconds(result.verification, "native_seconds_excluding_file_io",
            totals.verification_native_seconds);
    account(result.rpy, "rows", totals.rpy_rows);
    account(result.rpy, "steps", totals.rpy_steps);
    return result;
}

struct Options {
    fs::path root, fidelity, output;
    std::string known_binary;
};

Options options(int argc, char **argv) {
    Options result;
    std::set<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        require(i + 1 < argc && seen.insert(key).second, "invalid_options");
        const std::string value = argv[++i];
        if (key == "--root")
            result.root = value;
        else if (key == "--fidelity")
            result.fidelity = value;
        else if (key == "--output")
            result.output = value;
        else if (key == "--known-binary-sha256") {
            require(hex(value, 64), "invalid_known_binary_hash");
            result.known_binary = value;
        } else
            throw std::runtime_error("unknown_option");
    }
    require(!result.root.empty() && !result.fidelity.empty() && !result.output.empty(),
            "required_options_missing");
    return result;
}

int report(const Options &opt) {
    const auto fidelity_text = read(opt.fidelity);
    const auto source_fidelity = parse(fidelity_text);
    const auto fidelity =
        scalars(source_fidelity,
                {"outcome", "failure", "manifest_sha256", "schema_sha256", "checker_sha256",
                 "dat_sha256", "expected_fixture_count", "manifest_fixtures", "selected_fixtures",
                 "full_suite", "passed", "verified_rows", "jobs", "timeout_seconds"});
    require(integer(field(fidelity, "passed")) <= integer(field(fidelity, "selected_fixtures")),
            "invalid_fidelity_counts");
    std::vector<Fields> failures;
    if (source_fidelity.count("cases"))
        for (const auto &entry : objects(field(source_fidelity, "cases")))
            if (field(entry, "outcome") != "\"passed\"")
                failures.push_back(scalars(entry, {"id", "outcome", "failure", "rows", "sha256"}));
    Totals totals;
    std::vector<Case> cases;
    for (const auto *group :
         {"spell92-v1", "spell92-diversity-v2", "spell92-diversity-v3", "spell92-training-v1",
          "spell92-efficiency-v1", "spell92-holdout-v1", "practice-v1", "advance-v1", "story-v2"}) {
        const auto directory = opt.root / group;
        if (!fs::is_directory(directory))
            continue;
        std::set<std::string> ids;
        for (const auto &entry : fs::directory_iterator(directory)) {
            auto id = entry.path().filename().string();
            if (entry.is_directory()) {
                if (id.size() >= 7 && id.compare(id.size() - 7, 7, "-verify") == 0)
                    continue;
            } else if (entry.path().extension() == ".exit")
                id = entry.path().stem().string();
            else
                continue;
            require(!entry.is_symlink() && identifier(id), "unsafe_case_id");
            ids.insert(id);
        }
        for (const auto &id : ids)
            cases.push_back(collect(opt.root, group, id, opt.known_binary, totals));
    }
    require(!cases.empty(), "no_campaign_cases");
    const bool valid = std::none_of(cases.begin(), cases.end(), [](const Case &entry) {
        return entry.status == "invalid" || entry.status == "unverified_complete" ||
               entry.status == "unverified_missing_report";
    });
    auto accounted_updates = totals.solver_updates;
    add(accounted_updates, totals.verification_updates);
    if (!opt.output.parent_path().empty())
        fs::create_directories(opt.output.parent_path());
    const auto temporary = fs::path(opt.output.string() + ".tmp");
    std::ofstream out(temporary);
    require(bool(out), "output_unwritable");
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "{\"producer\":\"th08_planner_report\",\"evidence_profile\":\"native-headless-"
           "projection\","
           "\"retail_equivalence_established\":false,\"ledger_valid\":"
        << (valid ? "true" : "false")
        << ",\"fidelity_input_sha256\":" << quote(digest(fidelity_text)) << ",\"fidelity\":";
    emit(out, fidelity);
    out << ",\"fidelity_failures\":[";
    for (std::size_t i = 0; i < failures.size(); ++i) {
        if (i)
            out << ',';
        emit(out, failures[i]);
    }
    out << "],\"totals\":{\"finalized_cases\":" << totals.finalized
        << ",\"pending_cases\":" << totals.pending
        << ",\"setup_rejected_cases\":" << totals.setup_rejections
        << ",\"setup_rejection_native_costs_recorded\":false"
        << ",\"missing_report_cases\":" << totals.missing_reports
        << ",\"verified_complete_cases\":" << totals.complete
        << ",\"failed_cases\":" << totals.failed
        << ",\"unverified_complete_cases\":" << totals.unverified
        << ",\"solver_native_updates\":" << totals.solver_updates
        << ",\"verification_native_updates\":" << totals.verification_updates
        << ",\"native_updates\":" << accounted_updates
        << ",\"rpy_calculation_rows\":" << totals.rpy_rows
        << ",\"rpy_session_steps\":" << totals.rpy_steps
        << ",\"sum_solver_wall_seconds\":" << totals.solver_wall_seconds
        << ",\"solver_native_seconds_excluding_file_io\":" << totals.solver_native_seconds
        << ",\"verification_native_seconds_excluding_file_io\":"
        << totals.verification_native_seconds << ",\"solver_costs\":{";
    bool first = true;
    for (const auto &entry : totals.cost_totals) {
        out << (first ? "" : ",") << quote(entry.first) << ':' << entry.second;
        first = false;
    }
    out << "}},\"cases\":[";
    first = true;
    for (const auto &entry : cases) {
        out << (first ? "" : ",") << "{\"group\":" << quote(entry.group)
            << ",\"id\":" << quote(entry.id) << ",\"status\":" << quote(entry.status)
            << ",\"failure\":" << quote(entry.failure);
        if (entry.status != "pending") {
            out << ",\"exit_code\":" << entry.exit_code
                << ",\"plan_sha256\":" << quote(entry.plan_sha256)
                << ",\"verify_sha256\":" << quote(entry.verify_sha256) << ",\"identity\":";
            emit(out, entry.identity);
            out << ",\"plan\":";
            emit(out, entry.plan);
            out << ",\"options\":";
            emit(out, entry.options);
            out << ",\"verification\":";
            emit(out, entry.verification);
            out << ",\"verification_options\":";
            emit(out, entry.verification_options);
            out << ",\"rpy\":";
            emit(out, entry.rpy);
        }
        out << '}';
        first = false;
    }
    out << "]}\n";
    out.close();
    require(bool(out), "output_write_failed");
    fs::rename(temporary, opt.output);
    std::cout << "finalized=" << totals.finalized << " verified_complete=" << totals.complete
              << " failed=" << totals.failed << " pending=" << totals.pending
              << " setup_rejected=" << totals.setup_rejections
              << " unverified_complete=" << totals.unverified << '\n';
    return valid ? 0 : 1;
}
} // namespace

int main(int argc, char **argv) {
    try {
        return report(options(argc, argv));
    } catch (const std::exception &) {
        std::cerr << "th08_planner_report: invalid_input_or_output\n";
        return 1;
    }
}
