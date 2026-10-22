// Compose the bounded campaign ledger from completed process artifacts. Unknown
// fields and private paths never become public evidence; unfinished jobs supply
// only a pending ID. Verification checks execution agreement, not retail physics.
#include "headless_report.hpp"
#include "th08/plan_matrix.hpp"
#include "th08/plan_prefix.hpp"
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
constexpr const char *source_revision = "02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415";

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

std::string read(const fs::path &path, std::size_t limit = 2 * 1024 * 1024) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "input_unreadable");
    std::string result;
    std::array<char, 8192> buffer{};
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        require(result.size() + std::size_t(input.gcount()) <= limit, "input_too_large");
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
        if (key == "prefix_sha256" && value == "null")
            continue; // Non-prefix episodes have no input-prefix identity.
        if (key == "replay_target_version") {
            require(hex(string_value(value), 5), "invalid_replay_target_version");
            result.emplace(key, value);
            continue;
        }
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
    "stage",          "spell",       "difficulty", "seed",        "rollout",    "beam",
    "commit",         "checkpoint",  "keep",       "attempts",    "cegar",      "frames",
    "seconds",        "shoot",       "story",      "until_stage", "shot",       "measure",
    "diversity_cell", "boss_follow", "prefix",     "verify",      "final_route"};

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
                                            "prefix_native_update_attempts",
                                            "prefix_native_updates",
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
                                            "final_spell",
                                            "final_spell_active",
                                            "prefix_requested_frames",
                                            "prefix_complete",
                                            "prefix_sha256",
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

const std::vector<std::string> calculation_keys = {
    "calculation_updates",        "control_updates",
    "stage_transition_count",     "actions_sha256",
    "native_calculation_updates", "native_control_updates",
    "native_stage_transitions",   "final_spell",
    "final_spell_active"};

void add_calculations(Fields &output, const Fields &input) {
    const auto counters = scalars(input, calculation_keys);
    output.insert(counters.begin(), counters.end());
    if (input.count("stage_transitions")) {
        std::ostringstream encoded;
        encoded << '[';
        bool first = true;
        for (const auto &event : objects(field(input, "stage_transitions"))) {
            if (!first)
                encoded << ',';
            emit(encoded,
                 scalars(event, {"frame", "from_stage", "to_stage", "calculation_updates_before",
                                 "calculation_updates_after"}));
            first = false;
        }
        encoded << ']';
        output.emplace("stage_transitions", encoded.str());
    }
}

bool has_calculations(const Fields &input) {
    for (const auto *key :
         {"calculation_updates", "control_updates", "stage_transition_count", "stage_transitions"})
        if (input.count(key))
            return true;
    return false;
}

std::uint64_t calculation_proof(const Fields &input, const Fields &plan, std::uint64_t frames) {
    const auto calculations = integer(field(input, "calculation_updates"));
    const auto controls = integer(field(input, "control_updates"));
    require(calculations <= frames && controls == frames - calculations,
            "calculation_update_count_mismatch");
    const auto events = objects(field(input, "stage_transitions"));
    require(integer(field(input, "stage_transition_count")) == events.size(),
            "stage_transition_count_mismatch");
    const auto initial_stage = integer(field(plan, "stage"));
    const auto terminal_stage = integer(field(plan, "final_stage"));
    require(initial_stage < 9 && terminal_stage < 9 &&
                (events.empty() ? initial_stage == terminal_stage
                                : integer(field(events.front(), "from_stage")) == initial_stage &&
                                      integer(field(events.back(), "to_stage")) == terminal_stage),
            "stage_transition_boundary_mismatch");
    std::uint64_t previous_frame = 0, zero_calculation_events = 0, previous_stage = 0;
    bool first = true;
    for (const auto &event : events) {
        const auto frame = integer(field(event, "frame"));
        const auto from = integer(field(event, "from_stage"));
        const auto to = integer(field(event, "to_stage"));
        const auto before = integer(field(event, "calculation_updates_before"));
        const auto after = integer(field(event, "calculation_updates_after"));
        require(frame > previous_frame && frame <= frames && from < 9 && to < 9 && from != to &&
                    (first || from == previous_stage) && after >= before && after - before <= 1 &&
                    frame > zero_calculation_events &&
                    before == frame - 1 - zero_calculation_events,
                "invalid_stage_transition_accounting");
        if (after == before)
            ++zero_calculation_events;
        previous_frame = frame;
        previous_stage = to;
        first = false;
    }
    require(zero_calculation_events == controls, "unaccounted_control_updates");
    return calculations;
}

void bind_actions(const Fields &input, const fs::path &actions) {
    // Hash the exact retained bytes, including whitespace, rather than a
    // reserialized action sequence. The verifier hashes and parses the same stream.
    require(field(input, "actions_sha256") == quote(digest(read(actions, 64 * 1024 * 1024))),
            "accounting_actions_hash_mismatch");
}

struct Case {
    std::string group, id, status = "pending", failure;
    Fields plan, options, verification, verification_options, accounting, accounting_options, rpy,
        archive_replay, identity, artifacts;
    std::string plan_sha256, verify_sha256, accounting_sha256, archive_replay_sha256;
    std::string expected_dat_sha256;
    int exit_code = -1;
};

struct Totals {
    std::uint64_t finalized = 0, pending = 0, complete = 0, failed = 0, unverified = 0;
    std::uint64_t setup_rejections = 0, missing_reports = 0;
    std::uint64_t solver_updates = 0, verification_updates = 0, rpy_rows = 0, rpy_steps = 0;
    std::uint64_t archive_replay_rows = 0, archive_replay_steps = 0;
    std::uint64_t accounting_updates = 0, accounting_receipts = 0;
    double solver_wall_seconds = 0, solver_native_seconds = 0, verification_native_seconds = 0;
    double accounting_native_seconds = 0;
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

std::uint64_t accounting_receipt(Case &result, const fs::path &root, std::uint64_t frames) {
    const auto path = root / result.group / (result.id + "-accounting") / "verify.json";
    require(fs::is_regular_file(path), "missing_calculation_accounting");
    const auto text = read(path);
    const auto receipt = parse(text);
    result.accounting_sha256 = digest(text);
    result.accounting = scalars(receipt, verify_keys);
    add_calculations(result.accounting, receipt);
    result.accounting_options = scalars(parse(field(receipt, "options")), option_keys);
    require(field(receipt, "verified") == "true" && integer(field(receipt, "frames")) == frames &&
                integer(field(receipt, "complete_at")) == frames &&
                integer(field(receipt, "death_updates")) == 0 &&
                integer(field(receipt, "native_update_attempts")) == frames &&
                integer(field(receipt, "native_updates")) == frames,
            "accounting_verification_failed");
    for (const auto *key : {"score", "rng_seed", "rng_draws"})
        require(integer(field(receipt, key)) == integer(field(result.plan, key)) &&
                    integer(field(receipt, key)) == integer(field(result.verification, key)),
                "accounting_state_mismatch");
    require(field(receipt, "source_revision") == quote(source_revision) &&
                hex(string_value(field(receipt, "dat_sha256")), 64),
            "accounting_identity_mismatch");
    if (!result.expected_dat_sha256.empty())
        require(field(receipt, "dat_sha256") == result.expected_dat_sha256,
                "accounting_identity_mismatch");
    bool legacy_identity = false;
    for (const auto *original : {&result.plan, &result.verification}) {
        for (const auto *key : {"source_revision", "dat_sha256"})
            if (original->count(key))
                require(field(receipt, key) == field(*original, key),
                        "accounting_identity_mismatch");
            else
                legacy_identity = true;
        legacy_identity = legacy_identity || !original->count("binary_sha256");
    }
    // Old practice reports predate producer identity fields. Their missing
    // identity is explicit and requires the separately retained campaign binary.
    if (legacy_identity)
        require(field(result.identity, "binary_origin") == quote("retained_campaign_binary") &&
                    hex(string_value(field(result.identity, "binary_sha256")), 64) &&
                    !result.expected_dat_sha256.empty(),
                "missing_legacy_accounting_identity");
    require(hex(string_value(field(receipt, "binary_sha256")), 64),
            "missing_accounting_binary_identity");
    require(integer(field(receipt, "stage")) == integer(field(result.plan, "final_stage")) &&
                integer(field(receipt, "stage")) == integer(field(result.verification, "stage")),
            "accounting_terminal_mismatch");
    for (const auto *key : {"final_spell", "final_spell_active"})
        if (result.plan.count(key))
            require(field(receipt, key) == field(result.plan, key), "accounting_terminal_mismatch");
    for (const auto *key : {"stage", "spell", "difficulty", "seed", "shot", "story", "until_stage"})
        require(field(result.accounting_options, key) == field(result.options, key) &&
                    field(result.accounting_options, key) ==
                        field(result.verification_options, key),
                "accounting_checkpoint_mismatch");
    if (result.options.count("final_route") || result.verification_options.count("final_route") ||
        result.accounting_options.count("final_route"))
        require(field(result.accounting_options, "final_route") ==
                        field(result.options, "final_route") &&
                    field(result.accounting_options, "final_route") ==
                        field(result.verification_options, "final_route"),
                "accounting_checkpoint_mismatch");
    bind_actions(receipt, root / result.group / result.id / "plan.actions");
    return calculation_proof(result.accounting, result.plan, frames);
}

Fields replay_summary(const Fields &input) {
    auto result = scalars(input, {"rows", "finished", "natural", "steps", "runtime_error",
                                  "io_error", "death_rows", "bomb_input_rows", "replay_sha256",
                                  "dat_sha256", "source_revision", "replay_target_exe_size",
                                  "replay_target_exe_checksum", "replay_target_version"});
    if (input.count("binary_sha256")) {
        const auto identity = scalars(input, {"binary_sha256"});
        result.emplace("checker_sha256", field(identity, "binary_sha256"));
    }
    return result;
}

void natural_replay(Fields &summary, const Fields &input, const Fields &plan,
                    std::uint64_t expected_rows, const Fields *checkpoint = nullptr) {
    require(field(input, "finished") == "true" && field(input, "natural") == "true",
            "natural_replay_mismatch");
    require(integer(field(input, "rows")) == expected_rows, "natural_replay_calculation_mismatch");
    for (const auto *key : {"runtime_error", "io_error"})
        require(!input.count(key) || field(input, key) == "false", "natural_replay_error");
    require(integer(field(input, "death_rows")) == 0 &&
                integer(field(input, "bomb_input_rows")) == 0,
            "natural_replay_not_nmnb");
    const auto stages = objects(field(input, "stages"));
    require(!stages.empty(), "natural_replay_missing_stages");
    // A continuous story's terminal score and stage belong to its final summary.
    const auto &last = stages.back();
    require(integer(field(last, "endScore")) == integer(field(plan, "score")),
            "natural_replay_score_mismatch");
    if (plan.count("final_stage"))
        require(integer(field(last, "stage")) == integer(field(plan, "final_stage")),
                "natural_replay_terminal_mismatch");
    if (checkpoint) {
        std::string path = "[";
        for (const auto &stage : stages)
            path += (path.size() == 1 ? "" : ",") + std::to_string(integer(field(stage, "stage")));
        path += ']';
        require(path == field(*checkpoint, "expected_stages"), "matrix_replay_stage_path_mismatch");
        summary.emplace("stage_path", path);
    }
    summary.emplace("endScore", field(last, "endScore"));
}

void replay_identity(const Fields &receipt, const Case &result, const std::string &replay_hash) {
    // A fresh checker may fill missing legacy identity, never contradict an
    // identity the original proof already recorded. Checker binaries may differ.
    if (receipt.count("replay_sha256") && !replay_hash.empty())
        require(field(receipt, "replay_sha256") == quote(replay_hash),
                "archive_replay_hash_mismatch");
    if (receipt.count("source_revision"))
        require(field(receipt, "source_revision") == quote(source_revision),
                "archive_replay_identity_mismatch");
    for (const auto *key : {"dat_sha256", "source_revision"})
        if (receipt.count(key) && result.plan.count(key))
            require(field(receipt, key) == field(result.plan, key),
                    "archive_replay_identity_mismatch");
    if (receipt.count("dat_sha256") && !result.expected_dat_sha256.empty())
        require(field(receipt, "dat_sha256") == result.expected_dat_sha256,
                "archive_replay_identity_mismatch");
}

void bind_replay(const Fields &receipt, const Case &result, const std::string &replay_hash) {
    require(field(receipt, "replay_sha256") == quote(replay_hash), "archive_replay_hash_mismatch");
    require(hex(string_value(field(receipt, "checker_sha256")), 64),
            "missing_replay_checker_identity");
    require(hex(string_value(field(receipt, "dat_sha256")), 64) &&
                field(receipt, "source_revision") == quote(source_revision),
            "missing_replay_input_identity");
    replay_identity(receipt, result, replay_hash);
}

void check_process_exit(const fs::path &path, Fields &summary, const char *failure, bool required) {
    // Older campaigns used checked shell commands without sidecar receipts.
    // If a receipt exists, successful JSON cannot override a failed process.
    require(!required || fs::exists(path), "missing_process_receipt");
    if (!fs::exists(path))
        return;
    require(fs::is_regular_file(path) && !fs::is_symlink(path), "invalid_process_receipt");
    auto text = read(path);
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
        text.pop_back();
    const auto code = integer(text);
    require(code <= 255, "invalid_process_exit_code");
    summary.emplace("process_exit_code", std::to_string(code));
    require(code == 0, failure);
}

void verify_complete(Case &result, const fs::path &root, bool require_process_receipts,
                     const Fields *checkpoint) {
    const auto directory = result.group == "spell92-diversity-v3"
                               ? root / "spell92-verified" / result.id
                               : root / result.group / (result.id + "-verify");
    require(fs::is_regular_file(directory / "verify.json"), "missing_verification");
    const auto text = read(directory / "verify.json");
    const auto verification = parse(text);
    result.verify_sha256 = digest(text);
    result.verification = scalars(verification, verify_keys);
    add_calculations(result.verification, verification);
    check_process_exit(root / result.group / (result.id + "-verify.exit"), result.verification,
                       "verification_process_failed", require_process_receipts);
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
    if (result.options.count("prefix") && integer(field(result.options, "prefix")) != 0) {
        require(field(result.plan, "prefix_complete") == "true" &&
                    result.plan.count("prefix_sha256") &&
                    integer(field(result.plan, "prefix_requested_frames")) ==
                        integer(field(result.plan, "prefix_native_updates")) &&
                    integer(field(result.plan, "prefix_native_update_attempts")) ==
                        integer(field(result.plan, "prefix_native_updates")),
                "incomplete_prefix_reconstruction");
    }
    for (const auto *key : {"score", "rng_seed", "rng_draws"})
        require(integer(field(result.plan, key)) == integer(field(verification, key)),
                "verification_state_mismatch");
    if (result.plan.count("final_stage"))
        require(integer(field(verification, "stage")) == integer(field(result.plan, "final_stage")),
                "verification_terminal_mismatch");
    for (const auto *key : {"final_spell", "final_spell_active"})
        if (result.plan.count(key) && verification.count(key))
            require(field(result.plan, key) == field(verification, key),
                    "verification_terminal_mismatch");
    for (const auto *key : {"source_revision", "binary_sha256", "dat_sha256"})
        if (result.plan.count(key) && verification.count(key))
            require(field(result.plan, key) == field(verification, key),
                    "verification_identity_mismatch");
    for (const auto *key : {"stage", "spell", "difficulty", "seed", "shot", "story", "until_stage"})
        if (result.options.count(key) && result.verification_options.count(key))
            require(field(result.options, key) == field(result.verification_options, key),
                    "verification_checkpoint_mismatch");
    if (result.options.count("final_route") || result.verification_options.count("final_route"))
        require(field(result.options, "final_route") ==
                    field(result.verification_options, "final_route"),
                "verification_checkpoint_mismatch");
    const bool new_plan = has_calculations(result.plan);
    const bool new_verification = has_calculations(result.verification);
    std::uint64_t expected_rows = frames;
    bool observed_calculations = false;
    if (new_plan || new_verification) {
        require(new_plan && new_verification, "incomplete_calculation_accounting");
        expected_rows = calculation_proof(result.plan, result.plan, frames);
        require(calculation_proof(result.verification, result.plan, frames) == expected_rows,
                "verification_calculation_mismatch");
        for (const auto *key : {"calculation_updates", "control_updates", "stage_transition_count",
                                "stage_transitions"})
            require(field(result.plan, key) == field(result.verification, key),
                    "verification_calculation_mismatch");
        bind_actions(verification, root / result.group / result.id / "plan.actions");
        if (result.plan.count("actions_sha256"))
            bind_actions(result.plan, root / result.group / result.id / "plan.actions");
        observed_calculations = true;
    } else if (fs::exists(root / result.group / (result.id + "-accounting") / "verify.json")) {
        // The original execution proof and its binary identities stay intact.
        // This separately hashed fresh run only supplies missing observations.
        expected_rows = accounting_receipt(result, root, frames);
        observed_calculations = true;
    }
    if (numeric(field(result.plan, "spell")) < 0) {
        const auto path = root / result.group / (result.id + "-rpy.json");
        require(fs::is_regular_file(path), "missing_natural_replay");
        const auto replay = parse(read(path));
        result.rpy = replay_summary(replay);
        check_process_exit(root / result.group / (result.id + "-rpy.exit"), result.rpy,
                           "natural_replay_process_failed", require_process_receipts);
        replay_identity(result.rpy, result, "");
        const auto rows = integer(field(replay, "rows"));
        require(rows == expected_rows || observed_calculations, "missing_calculation_accounting");
        natural_replay(result.rpy, replay, result.plan, expected_rows, checkpoint);
        const auto archive_path = root / result.group / (result.id + "-rpy-archive.json");
        if (fs::exists(archive_path)) {
            // Preserve the primary replay run and its work. This additional
            // receipt supplies current checker identity and byte binding only.
            const auto text = read(archive_path);
            const auto fresh = parse(text);
            result.archive_replay_sha256 = digest(text);
            result.archive_replay = replay_summary(fresh);
            natural_replay(result.archive_replay, fresh, result.plan, rows, checkpoint);
            const auto replay_hash =
                digest(read(root / result.group / result.id / "th8_solver.rpy", 64 * 1024 * 1024));
            bind_replay(result.archive_replay, result, replay_hash);
            replay_identity(result.rpy, result, replay_hash);
        }
    }
}

template <std::size_t N>
void publish_artifacts(const fs::path &destination, const std::array<std::string, N> &names,
                       const std::array<std::string, N> &contents,
                       const std::array<std::string, N> &hashes) {
    std::array<bool, N> existing{}, staged{}, created{};
    for (std::size_t i = 0; i < names.size(); ++i) {
        const auto path = destination / names[i];
        existing[i] = fs::exists(path);
        if (existing[i])
            require(!fs::is_symlink(path) && digest(read(path, 64 * 1024 * 1024)) == hashes[i],
                    "archive_artifact_conflict");
        else
            require(!fs::exists(destination / (names[i] + ".tmp")), "archive_temporary_conflict");
    }
    fs::create_directories(destination);
    try {
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (existing[i])
                continue;
            const auto temporary = destination / (names[i] + ".tmp");
            std::ofstream output(temporary, std::ios::binary);
            require(bool(output), "archive_write_failed");
            staged[i] = true;
            output.write(contents[i].data(), std::streamsize(contents[i].size()));
            output.close();
            require(bool(output), "archive_write_failed");
        }
        for (std::size_t i = 0; i < names.size(); ++i)
            if (!existing[i]) {
                fs::rename(destination / (names[i] + ".tmp"), destination / names[i]);
                created[i] = true;
            }
    } catch (...) {
        std::error_code ignored;
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (created[i])
                fs::remove(destination / names[i], ignored);
            if (staged[i])
                fs::remove(destination / (names[i] + ".tmp"), ignored);
        }
        throw;
    }
}

void archive_complete(Case &result, const fs::path &root, const fs::path &destination) {
    if (destination.empty() || numeric(field(result.plan, "spell")) >= 0)
        return;
    const auto directory = root / result.group / result.id;
    const auto replay = read(directory / "th8_solver.rpy", 64 * 1024 * 1024);
    const auto actions = read(directory / "plan.actions", 64 * 1024 * 1024);
    const auto replay_hash = digest(replay), actions_hash = digest(actions);
    bind_replay(result.archive_replay.empty() ? result.rpy : result.archive_replay, result,
                replay_hash);
    replay_identity(result.rpy, result, replay_hash);
    bool bound_actions = false;
    for (const auto *proof : {&result.verification, &result.accounting})
        if (proof->count("actions_sha256")) {
            require(field(*proof, "actions_sha256") == quote(actions_hash),
                    "archive_actions_hash_mismatch");
            bound_actions = true;
        }
    require(bound_actions, "missing_archive_actions_binding");
    const auto basename = result.group + "--" + result.id;
    const std::array<std::string, 2> names{basename + ".rpy", basename + ".actions"};
    const std::array<std::string, 2> contents{replay, actions}, hashes{replay_hash, actions_hash};
    publish_artifacts(destination, names, contents, hashes);
    result.artifacts = {{"rpy_file", quote(names[0])},
                        {"rpy_sha256", quote(replay_hash)},
                        {"actions_file", quote(names[1])},
                        {"actions_sha256", quote(actions_hash)}};
}

void archive_failed_actions(Case &result, const fs::path &root, const fs::path &destination) {
    if (destination.empty())
        return;
    const auto actions = read(root / result.group / result.id / "plan.actions", 64 * 1024 * 1024);
    const auto frames = integer(field(result.plan, "frames"));
    require(frames <= 64 * 1024 * 1024, "invalid_failed_prefix_frames");
    th08::plan::PrefixParser parser{std::size_t(frames)};
    parser.append(actions.data(), actions.size());
    require(parser.finish().size() == frames, "failed_prefix_count_mismatch");
    const auto hash = digest(actions);
    const std::array<std::string, 1> names{result.group + "--" + result.id + ".actions"};
    publish_artifacts(destination, names, std::array<std::string, 1>{actions},
                      std::array<std::string, 1>{hash});
    // This is the final emitted search prefix, not a successful demonstration
    // or an archive of every rejected internal rollout.
    result.artifacts = {{"actions_file", quote(names[0])},
                        {"actions_sha256", quote(hash)},
                        {"actions_frames", std::to_string(frames)},
                        {"artifact_role", quote("failed_search_prefix")},
                        {"execution_verified", "false"}};
}

// This immutable, report-owned manifest supplies checkpoints and the coverage
// denominator. Repeated attempts retain separate costs and share one route ID.
struct RouteMatrix {
    std::string sha256;
    std::map<std::string, Fields> checkpoints;
    bool sampled = false;
    std::string sample_phase;
};

struct ManifestCase {
    th08::plan::MatrixCase route;
    std::uint64_t seed;
};

RouteMatrix route_matrix(const fs::path &path, bool sampled) {
    RouteMatrix result;
    if (path.empty())
        return result;
    const auto text = read(path);
    const auto manifest = parse(text);
    require(integer(field(manifest, "schema_version")) == 1, "matrix_schema_mismatch");
    result.sampled = sampled;
    const auto seed = sampled ? 0 : integer(field(manifest, "seed"));
    require(seed <= 65535, "matrix_seed_out_of_range");
    if (sampled) {
        require(string_value(field(manifest, "sampling_unit")) == "seed-case-pair",
                "sample_unit_mismatch");
        if (manifest.count("sample_phase")) {
            result.sample_phase = string_value(field(manifest, "sample_phase"));
            require(result.sample_phase == "diagnosis" || result.sample_phase == "holdout",
                    "sample_phase_invalid");
        }
    }
    std::map<std::string, Fields> entries;
    for (const auto &entry : objects(field(manifest, "cases"))) {
        const auto id = string_value(field(entry, "id"));
        require(identifier(id) && entries.emplace(id, entry).second,
                "matrix_duplicate_or_unsafe_id");
    }
    std::vector<ManifestCase> selected;
    if (sampled) {
        require(!entries.empty(), "sample_empty_manifest");
        for (const auto &item : entries) {
            const auto &input = item.second;
            const auto shot = integer(field(input, "shot"));
            const auto difficulty = integer(field(input, "difficulty"));
            const auto case_seed = integer(field(input, "seed"));
            require(shot < 12 && difficulty < 5 && case_seed <= 65535,
                    "sample_checkpoint_out_of_range");
            const auto route = string_value(field(input, "final_route"));
            require(difficulty == 4 ? route == "natural" : route == "a" || route == "b",
                    "sample_invalid_route");
            if (!result.sample_phase.empty())
                require(string_value(field(input, "sample_phase")) == result.sample_phase,
                        "sample_phase_mismatch");
            const auto final_route = route == "a"   ? th08::plan::FinalRoute::A
                                     : route == "b" ? th08::plan::FinalRoute::B
                                                    : th08::plan::FinalRoute::Natural;
            selected.push_back({{int(shot), int(difficulty), final_route}, case_seed});
        }
    } else {
        const auto catalog = th08::plan::full_run_matrix();
        require(entries.size() == catalog.size(), "matrix_case_count_mismatch");
        for (const auto &entry : catalog)
            selected.push_back({entry, seed});
    }
    for (const auto &selection : selected) {
        const auto &entry = selection.route;
        const auto case_seed = selection.seed;
        const std::string route = entry.route == th08::plan::FinalRoute::A   ? "a"
                                  : entry.route == th08::plan::FinalRoute::B ? "b"
                                                                             : "natural";
        const std::string difficulty = th08::plan::difficulty_names[entry.difficulty];
        const auto id = difficulty + "-t" + std::to_string(entry.shot) + '-' + route + "-s" +
                        std::to_string(case_seed);
        const auto found = entries.find(id);
        require(found != entries.end(), "matrix_catalog_mismatch");
        const auto &input = found->second;
        Fields checkpoint{{"shot", std::to_string(entry.shot)},
                          {"difficulty", std::to_string(entry.difficulty)},
                          {"seed", std::to_string(case_seed)},
                          {"story", "1"}};
        for (const auto &value : checkpoint)
            require(integer(field(input, value.first)) == integer(value.second),
                    "matrix_checkpoint_mismatch");
        require(
            string_value(field(input, "shot_name")) == th08::plan::shot_names[entry.shot] &&
                string_value(field(input, "difficulty_name")) == difficulty &&
                string_value(field(input, "stage")) == (entry.stage() == 8 ? "extra" : "1") &&
                string_value(field(input, "final_route")) == route &&
                string_value(field(input, "unlock_profile")) ==
                    (entry.difficulty == 4 ? "original-extra-entry" : "both-finals-clearState-2"),
            "matrix_profile_mismatch");
        std::string stages = "[";
        if (entry.difficulty != 4)
            stages += "0,1,2," + std::to_string(entry.fourth_stage()) + ",5,";
        stages += std::to_string(entry.final_stage()) + ']';
        auto supplied_stages = field(input, "expected_stages");
        supplied_stages.erase(
            std::remove_if(supplied_stages.begin(), supplied_stages.end(),
                           [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }),
            supplied_stages.end());
        require(supplied_stages == stages, "matrix_expected_stage_path_mismatch");
        checkpoint.emplace("stage", std::to_string(entry.stage()));
        checkpoint.emplace("spell", "-1");
        checkpoint.emplace("until_stage", "8");
        checkpoint.emplace("final_route", std::to_string(int(entry.route)));
        checkpoint.emplace("expected_stages", stages);
        require(result.checkpoints.emplace(id, std::move(checkpoint)).second,
                "matrix_duplicate_checkpoint");
    }
    result.sha256 = digest(text);
    return result;
}

Case collect(const fs::path &root, const std::string &group, const std::string &id,
             const std::string &known_binary, const std::string &known_dat,
             const fs::path &replay_directory, const fs::path &failed_actions_directory,
             bool require_process_receipts, const RouteMatrix &matrix, Totals &totals) {
    Case result;
    result.group = group;
    result.id = id;
    result.expected_dat_sha256 = known_dat;
    const auto exit = root / group / (id + ".exit");
    if (!fs::is_regular_file(exit)) {
        if (!matrix.sha256.empty() && !matrix.checkpoints.count(id)) {
            result.status = "invalid";
            result.failure = "matrix_unknown_case";
            return result;
        }
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
        add_calculations(result.plan, plan);
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
        const Fields *checkpoint = nullptr;
        if (!matrix.sha256.empty()) {
            const auto found = matrix.checkpoints.find(id);
            require(found != matrix.checkpoints.end(), "matrix_unknown_case");
            checkpoint = &found->second;
            for (const auto &value : *checkpoint) {
                if (value.first == "expected_stages")
                    continue;
                require(field(result.options, value.first) == value.second,
                        "matrix_options_mismatch");
                if (value.first != "final_route" && value.first != "until_stage")
                    require(field(result.plan, value.first) == value.second,
                            "matrix_plan_checkpoint_mismatch");
            }
        }
        if (outcome != "complete") {
            result.status = "failed";
            archive_failed_actions(result, root, failed_actions_directory);
            ++totals.failed;
            return result;
        }
        result.status = "unverified_complete";
        require(result.exit_code == 0, "complete_process_failed");
        verify_complete(result, root, require_process_receipts, checkpoint);
        if (checkpoint) {
            auto path = '[' + field(result.plan, "stage");
            for (const auto &transition : objects(field(result.plan, "stage_transitions")))
                path += ',' + std::to_string(integer(field(transition, "to_stage")));
            path += ']';
            require(path == field(*checkpoint, "expected_stages"),
                    "matrix_native_stage_path_mismatch");
        }
        archive_complete(result, root, replay_directory);
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
    if (!result.accounting.empty()) {
        ++totals.accounting_receipts;
        account(result.accounting, "native_updates", totals.accounting_updates);
        seconds(result.accounting, "native_seconds_excluding_file_io",
                totals.accounting_native_seconds);
    }
    account(result.rpy, "rows", totals.rpy_rows);
    account(result.rpy, "steps", totals.rpy_steps);
    account(result.archive_replay, "rows", totals.archive_replay_rows);
    account(result.archive_replay, "steps", totals.archive_replay_steps);
    return result;
}

struct Options {
    fs::path root, fidelity, output, replay_directory, failed_actions_directory, matrix, sample;
    std::string known_binary;
    std::vector<std::string> groups;
    bool explicit_groups = false;
};

Options options(int argc, char **argv) {
    Options result;
    std::set<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        require(i + 1 < argc && (key == "--group" || seen.insert(key).second), "invalid_options");
        const std::string value = argv[++i];
        if (key == "--root")
            result.root = value;
        else if (key == "--fidelity")
            result.fidelity = value;
        else if (key == "--output")
            result.output = value;
        else if (key == "--replay-dir")
            result.replay_directory = value;
        else if (key == "--failed-actions-dir")
            result.failed_actions_directory = value;
        else if (key == "--matrix")
            result.matrix = value;
        else if (key == "--sample")
            result.sample = value;
        else if (key == "--group") {
            require(identifier(value) && std::find(result.groups.begin(), result.groups.end(),
                                                   value) == result.groups.end(),
                    "invalid_group");
            result.groups.push_back(value);
            result.explicit_groups = true;
        } else if (key == "--known-binary-sha256") {
            require(hex(value, 64), "invalid_known_binary_hash");
            result.known_binary = value;
        } else
            throw std::runtime_error("unknown_option");
    }
    require(!result.root.empty() && !result.fidelity.empty() && !result.output.empty(),
            "required_options_missing");
    require(result.matrix.empty() || result.sample.empty(), "conflicting_case_manifests");
    if (result.groups.empty())
        result.groups = {"spell92-v1",
                         "spell92-diversity-v2",
                         "spell92-diversity-v3",
                         "spell92-training-v1",
                         "spell92-efficiency-v1",
                         "spell92-holdout-v1",
                         "practice-v1",
                         "advance-v1",
                         "story-v2",
                         "story-prefix-v1"};
    return result;
}

int report(const Options &opt) {
    const auto matrix =
        route_matrix(opt.sample.empty() ? opt.matrix : opt.sample, !opt.sample.empty());
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
    for (const auto &group : opt.groups) {
        const auto directory = opt.root / group;
        require(!opt.explicit_groups || fs::is_directory(directory), "missing_group");
        if (!fs::is_directory(directory))
            continue;
        require(!fs::is_symlink(directory), "unsafe_group");
        std::set<std::string> ids;
        for (const auto &entry : fs::directory_iterator(directory)) {
            auto id = entry.path().filename().string();
            if (!entry.is_directory() && entry.path().extension() == ".exit")
                id = entry.path().stem().string();
            else if (!entry.is_directory())
                continue;
            // Both verification directories and their process receipts are
            // sidecars, not additional solver attempts.
            if ((id.size() >= 7 && id.compare(id.size() - 7, 7, "-verify") == 0) ||
                (id.size() >= 4 && id.compare(id.size() - 4, 4, "-rpy") == 0) ||
                (id.size() >= 11 && id.compare(id.size() - 11, 11, "-accounting") == 0))
                continue;
            require(!entry.is_symlink() && identifier(id), "unsafe_case_id");
            ids.insert(id);
        }
        for (const auto &id : ids)
            cases.push_back(collect(
                opt.root, group, id, opt.known_binary,
                fidelity.count("dat_sha256") ? field(fidelity, "dat_sha256") : "",
                opt.replay_directory, opt.failed_actions_directory,
                opt.explicit_groups || !opt.matrix.empty() || !opt.sample.empty(), matrix, totals));
    }
    require(!cases.empty(), "no_campaign_cases");
    const bool valid = std::none_of(cases.begin(), cases.end(), [](const Case &entry) {
        return entry.status == "invalid" || entry.status == "unverified_complete" ||
               entry.status == "unverified_missing_report";
    });
    auto accounted_updates = totals.solver_updates;
    add(accounted_updates, totals.verification_updates);
    add(accounted_updates, totals.accounting_updates);
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
    if (!matrix.sha256.empty()) {
        std::set<std::string> observed, verified;
        for (const auto &entry : cases) {
            if (matrix.checkpoints.count(entry.id))
                observed.insert(entry.id);
            if (entry.status == "verified_complete")
                verified.insert(entry.id);
        }
        if (matrix.sampled) {
            std::set<std::uint64_t> expected_seeds, observed_seeds, verified_seeds;
            for (const auto &entry : matrix.checkpoints) {
                const auto seed = integer(field(entry.second, "seed"));
                expected_seeds.insert(seed);
                if (observed.count(entry.first))
                    observed_seeds.insert(seed);
                if (verified.count(entry.first))
                    verified_seeds.insert(seed);
            }
            out << ",\"coverage\":{\"sample_sha256\":" << quote(matrix.sha256)
                << ",\"sampling_unit\":\"seed-case-pair\""
                << ",\"expected_seed_case_pairs\":" << matrix.checkpoints.size()
                << ",\"observed_seed_case_pairs\":" << observed.size()
                << ",\"verified_seed_case_pairs\":" << verified.size()
                << ",\"not_yet_verified_seed_case_pairs\":"
                << matrix.checkpoints.size() - verified.size()
                << ",\"expected_unique_seeds\":" << expected_seeds.size()
                << ",\"observed_unique_seeds\":" << observed_seeds.size()
                << ",\"verified_unique_seeds\":" << verified_seeds.size();
            if (!matrix.sample_phase.empty())
                out << ",\"sample_phase\":" << quote(matrix.sample_phase);
            out << '}';
        } else {
            out << ",\"coverage\":{\"matrix_sha256\":" << quote(matrix.sha256)
                << ",\"expected_combinations\":" << matrix.checkpoints.size()
                << ",\"observed_combinations\":" << observed.size()
                << ",\"verified_combinations\":" << verified.size()
                << ",\"not_yet_verified_combinations\":"
                << matrix.checkpoints.size() - verified.size() << '}';
        }
    }
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
        << ",\"accounting_receipts\":" << totals.accounting_receipts
        << ",\"accounting_native_updates\":" << totals.accounting_updates
        << ",\"native_updates\":" << accounted_updates
        << ",\"rpy_calculation_rows\":" << totals.rpy_rows
        << ",\"rpy_session_steps\":" << totals.rpy_steps
        << ",\"archive_replay_rows\":" << totals.archive_replay_rows
        << ",\"archive_replay_steps\":" << totals.archive_replay_steps
        << ",\"sum_solver_wall_seconds\":" << totals.solver_wall_seconds
        << ",\"solver_native_seconds_excluding_file_io\":" << totals.solver_native_seconds
        << ",\"verification_native_seconds_excluding_file_io\":"
        << totals.verification_native_seconds
        << ",\"accounting_native_seconds_excluding_file_io\":" << totals.accounting_native_seconds
        << ",\"solver_costs\":{";
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
            if (!entry.accounting.empty()) {
                out << ",\"accounting_sha256\":" << quote(entry.accounting_sha256)
                    << ",\"accounting\":";
                emit(out, entry.accounting);
                out << ",\"accounting_options\":";
                emit(out, entry.accounting_options);
            }
            out << ",\"rpy\":";
            emit(out, entry.rpy);
            if (!entry.archive_replay.empty()) {
                out << ",\"archive_replay_sha256\":" << quote(entry.archive_replay_sha256)
                    << ",\"archive_replay\":";
                emit(out, entry.archive_replay);
            }
            if (!entry.artifacts.empty()) {
                out << ",\"artifacts\":";
                emit(out, entry.artifacts);
            }
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
