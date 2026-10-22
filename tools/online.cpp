#include "modern/headless/replay_check.hpp"
#include "modern/headless/runtime.hpp"
#include "th08/online_native.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/plan_route.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

namespace fs = std::filesystem;
namespace hl = th08::headless;
namespace on = th08::online;
using Clock = std::chrono::steady_clock;
namespace {
void require(bool condition, const char *reason) {
    if (!condition)
        throw std::runtime_error(reason);
}
struct Options {
    fs::path dat, out, prefix, risk;
    on::Configuration policy;
    int stage = 0, difficulty = 3, shot = 0, seed = 0, spell = -1, log_every = 0;
    bool story = false;
    th08::plan::FinalRoute route = th08::plan::FinalRoute::Natural;
    std::size_t frames = 400000;
    double seconds = 3600, deadline_ms = 1000.0 / 60;
};
int integer(const std::string &text) {
    std::size_t end;
    const auto value = std::stol(text, &end);
    require(end == text.size() && value >= std::numeric_limits<int>::min() &&
                value <= std::numeric_limits<int>::max(),
            "invalid_integer");
    return int(value);
}
double number(const std::string &text) {
    std::size_t end;
    const auto value = std::stod(text, &end);
    require(end == text.size() && std::isfinite(value), "invalid_number");
    return value;
}
Options parse(int argc, char **argv) {
    Options o;
    for (int i = 1; i < argc; i += 2) {
        require(i + 1 < argc, "missing_option_value");
        const std::string key = argv[i], value = argv[i + 1];
        auto boolean = [&] {
            const auto n = integer(value);
            require(n == 0 || n == 1, "invalid_boolean");
            return n != 0;
        };
        if (key == "--dat")
            o.dat = fs::absolute(value);
        else if (key == "--out")
            o.out = fs::absolute(value);
        else if (key == "--prefix")
            o.prefix = fs::absolute(value);
        else if (key == "--risk-dir")
            o.risk = fs::absolute(value);
        else if (key == "--stage") {
            const std::array<std::string, 9> names{"1", "2",  "3",  "4a",   "4b",
                                                   "5", "6a", "6b", "extra"};
            const auto found = std::find(names.begin(), names.end(), value);
            require(found != names.end(), "invalid_stage");
            o.stage = int(found - names.begin());
        } else if (key == "--difficulty")
            o.difficulty = int(integer(value));
        else if (key == "--shot")
            o.shot = int(integer(value));
        else if (key == "--seed")
            o.seed = int(integer(value));
        else if (key == "--spell")
            o.spell = int(integer(value));
        else if (key == "--story")
            o.story = boolean();
        else if (key == "--frames") {
            const auto n = integer(value);
            require(n > 0 && n <= 2000000, "invalid_frames");
            o.frames = std::size_t(n);
        } else if (key == "--seconds")
            o.seconds = number(value);
        else if (key == "--deadline-ms")
            o.deadline_ms = number(value);
        else if (key == "--horizon")
            o.policy.horizon = int(integer(value));
        else if (key == "--beam")
            o.policy.beam = int(integer(value));
        else if (key == "--diversity-cell")
            o.policy.diversity_cell = integer(value);
        else if (key == "--grow")
            o.policy.grow = float(number(value));
        else if (key == "--boss-follow")
            o.policy.boss_follow = float(number(value));
        else if (key == "--risk-weight")
            o.policy.risk_weight = float(number(value));
        else if (key == "--nowarn-weight")
            o.policy.nowarn_weight = float(number(value));
        else if (key == "--focus-bias")
            o.policy.focus_bias = float(number(value));
        else if (key == "--spawn-guard")
            o.policy.spawn_guard = float(number(value));
        else if (key == "--risk-hard")
            o.policy.risk_hard = boolean();
        else if (key == "--reach-filter")
            o.policy.reach_filter = boolean();
        else if (key == "--risk-stop-age")
            o.policy.risk_stop_age = boolean();
        else if (key == "--risk-phantoms")
            o.policy.risk_phantoms = boolean();
        else if (key == "--queued-shots")
            o.policy.queued_shots = boolean();
        else if (key == "--queued-lasers")
            o.policy.queued_lasers = boolean();
        else if (key == "--held-fallback")
            o.policy.held_fallback = boolean();
        else if (key == "--direct-turn-envelope")
            o.policy.direct_turn_envelope = boolean();
        else if (key == "--log-every")
            o.log_every = int(integer(value));
        else if (key == "--commit")
            require(integer(value) == 1, "only_one_update_commits_supported");
        else if (key == "--forecast")
            require(value == "kinematic", "unsupported_live_forecast");
        else if (key == "--final-route") {
            require(value == "a" || value == "b" || value == "natural", "invalid_final_route");
            o.route = value == "a"   ? th08::plan::FinalRoute::A
                      : value == "b" ? th08::plan::FinalRoute::B
                                     : th08::plan::FinalRoute::Natural;
        } else
            throw std::runtime_error("unknown_option");
    }
    require(!o.dat.empty() && !o.out.empty(), "required_options_missing");
    require(o.seed >= 0 && o.seed <= 65535 && o.shot >= 0 && o.shot < 12 && o.difficulty >= 0 &&
                o.difficulty < 5 && o.spell >= -1 && o.spell <= 222 && o.seconds > 0 &&
                o.deadline_ms > 0 && o.log_every >= 0,
            "invalid_checkpoint_or_budget");
    require(o.stage != 8 || o.difficulty == 4, "extra_requires_difficulty_four");
    require(o.stage != 8 || o.route == th08::plan::FinalRoute::Natural, "extra_has_no_final_route");
    return o;
}
std::string hash(const fs::path &path, th08::plan::PrefixParser *parser = nullptr) {
    std::ifstream input(path, std::ios::binary);
    require(bool(input), "hash_input_unreadable");
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    require(ctx && EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) == 1, "hash_init_failed");
    std::array<char, 65536> buffer;
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        if (parser)
            parser->append(buffer.data(), std::size_t(input.gcount()));
        require(EVP_DigestUpdate(ctx.get(), buffer.data(), std::size_t(input.gcount())) == 1,
                "hash_update_failed");
    }
    require(input.eof(), "hash_read_failed");
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned length;
    require(EVP_DigestFinal_ex(ctx.get(), digest, &length) == 1 && length == 32,
            "hash_finish_failed");
    std::ostringstream result;
    for (unsigned i = 0; i < length; ++i)
        result << std::hex << std::setw(2) << std::setfill('0') << unsigned(digest[i]);
    return result.str();
}
struct Scratch {
    fs::path previous = fs::current_path(), directory;
    Scratch() {
        char name[] = "/tmp/th08-online-XXXXXX";
        require(mkdtemp(name) != nullptr, "scratch_creation_failed");
        directory = name;
        fs::current_path(directory);
    }
    ~Scratch() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(directory, error);
    }
};
struct Transition {
    std::uint64_t update, before, after;
    int from, to;
};
struct DecisionLog {
    std::uint64_t update;
    int stage, spell, depth;
    float x, y;
    bool youkai;
    std::uint16_t action;
    double milliseconds;
};
int run(const Options &o) {
    require(!fs::exists(o.out) || fs::is_empty(o.out), "output_already_exists");
    fs::create_directories(o.out);
    const auto setup_begin = Clock::now();
    const auto dat_hash = hash(o.dat), binary_hash = hash("/proc/self/exe");
    std::vector<std::pair<std::string, std::string>> model_hashes;
    if (!o.risk.empty())
        for (const char *name : {"risk_table_merged.tsv", "risk_backoff_seg.tsv",
                                 "risk_backoff_global.tsv", "nowarn_merged.tsv"})
            model_hashes.emplace_back(name, hash(o.risk / name));
    on::Policy policy(o.policy,
                      o.risk.empty() ? on::RiskTables{} : on::load_risk_tables(o.risk.string()));
    th08::plan::PrefixParser parser(o.frames);
    const auto prefix_hash = o.prefix.empty() ? std::string{} : hash(o.prefix, &parser);
    const auto prefix = parser.finish();
    Scratch scratch;
    hl::Config config;
    const auto dat = o.dat.string();
    config.dat_path = dat.c_str();
    config.stage = o.stage;
    config.difficulty = o.difficulty;
    config.seed = std::uint16_t(o.seed);
    config.shot = o.shot;
    config.spell = o.spell;
    config.story = o.story;
    config.unlock_final_routes = o.route != th08::plan::FinalRoute::Natural;
    hl::Session session(config);
    auto state = session.state();
    const auto initial_calculations = state.calculation_updates;
    const auto started = Clock::now();
    const double setup_seconds = std::chrono::duration<double>(started - setup_begin).count();
    std::vector<std::uint16_t> actions;
    actions.reserve(o.frames);
    std::vector<double> timings;
    timings.reserve(o.frames);
    std::vector<DecisionLog> logs;
    if (o.log_every)
        logs.reserve(o.frames / std::size_t(o.log_every) + 1);
    std::vector<Transition> transitions;
    on::Observation observation;
    std::uint64_t attempts = 0, controls = 0, native_ns = 0, deadline_misses = 0, full_depth = 0;
    on::SearchStatistics search_stats;
    std::uint64_t queued_records = 0, unsupported_queued = 0;
    std::uint64_t queued_laser_records = 0, unsupported_queued_lasers = 0;
    std::uint64_t turn_hint_records = 0, unsupported_turn_hints = 0;
    int failed_input = -1;
    bool started_spell = false;
    std::string outcome = "budget", failure;
    auto terminal = [&] {
        if (o.spell >= 0) {
            started_spell = started_spell || state.spell_active;
            return started_spell && !state.spell_active;
        }
        return o.story ? th08::plan::story_terminal(state.stage, state.stage_complete, 8)
                       : state.stage_complete;
    };
    auto step = [&](std::uint16_t input) {
        const auto before = state;
        failed_input = input;
        ++attempts;
        const auto native_begin = Clock::now();
        const auto io_begin = session.file_io_time_ns();
        auto account = [&] {
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - native_begin)
                    .count();
            const auto io = session.file_io_time_ns() - io_begin;
            native_ns += std::uint64_t(elapsed) > io ? std::uint64_t(elapsed) - io : 0;
        };
        try {
            state = session.step(input);
        } catch (...) {
            account();
            throw;
        }
        account();
        actions.push_back(input);
        failed_input = -1;
        const auto calculations = state.calculation_updates - before.calculation_updates;
        const bool changed = before.stage != state.stage;
        if (calculations == 0)
            ++controls;
        if (changed)
            transitions.push_back(
                {actions.size(), before.calculation_updates - initial_calculations,
                 state.calculation_updates - initial_calculations, before.stage, state.stage});
        require(th08::plan::supported_recording_step(calculations, changed),
                "unsupported_calculation_boundary");
        require(o.spell < 0 || !state.spell_active || state.spell == o.spell,
                "active_spell_mismatch");
        require(o.route == th08::plan::FinalRoute::Natural || state.stage < 6 || state.stage > 7 ||
                    state.stage == (o.route == th08::plan::FinalRoute::A ? 6 : 7),
                "final_route_mismatch");
        require(state.deaths == 0 && state.player_state != 2, "death");
    };
    try {
        for (std::size_t i = 0; i < prefix.size(); ++i) {
            step(prefix[i]);
            // Warming needs motion history only; do not compute unused forecasts.
            on::capture_memory(session, actions.size(), observation);
            policy.observe(observation);
            if (terminal()) {
                require(i + 1 == prefix.size(), "prefix_after_terminal");
                outcome = "complete";
            }
        }
        while (outcome != "complete" && actions.size() < o.frames &&
               std::chrono::duration<double>(Clock::now() - started).count() < o.seconds) {
            const auto begin = Clock::now();
            on::capture_memory(session, actions.size(), observation, o.policy.spawn_guard > 0,
                               o.policy.queued_shots ? unsigned(o.policy.horizon) : 0,
                               o.policy.queued_lasers ? unsigned(o.policy.horizon) : 0,
                               o.policy.direct_turn_envelope);
            queued_records += observation.queued_shots.size();
            unsupported_queued += observation.unsupported_queued_shots;
            queued_laser_records += observation.queued_lasers.size();
            unsupported_queued_lasers += observation.unsupported_queued_lasers;
            turn_hint_records += observation.turn_hint_records;
            unsupported_turn_hints += observation.unsupported_turn_hints;
            const auto decision = policy.decide(observation);
            search_stats.selections += decision.search.selections;
            search_stats.candidates += decision.search.candidates;
            search_stats.bins += decision.search.bins;
            search_stats.retained_bins += decision.search.retained_bins;
            search_stats.selection_ms += decision.search.selection_ms;
            search_stats.held_paths += decision.search.held_paths;
            search_stats.held_updates += decision.search.held_updates;
            search_stats.held_selected += decision.search.held_selected;
            search_stats.turn_envelope_frames += decision.search.turn_envelope_frames;
            auto input = decision.action;
            if (o.spell < 0 && ((actions.size() + 1) % 2))
                input |= 4096;
            const auto route = th08::plan::final_route_input(o.route, state, actions.size() + 1);
            if (route)
                input = *route;
            const auto ms = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
            timings.push_back(ms);
            deadline_misses += ms > o.deadline_ms;
            full_depth += decision.depth >= o.policy.horizon;
            if (o.log_every && ((timings.size() - 1) % std::size_t(o.log_every)) == 0)
                logs.push_back({actions.size(), state.stage, state.spell_active ? state.spell : -1,
                                decision.depth, state.x, state.y, state.youkai, input, ms});
            step(input);
            if (terminal())
                outcome = "complete";
            if (actions.size() % 1200 == 0) {
                std::ofstream progress(o.out / "progress.json");
                progress << "{\"frame\":" << actions.size() << ",\"stage\":" << state.stage
                         << ",\"spell\":" << (state.spell_active ? state.spell : -1) << "}\n";
            }
        }
    } catch (const std::exception &error) {
        failure = error.what();
        outcome = failure == "death" ? "death" : "runtime_error";
        if (failure.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789") != std::string::npos)
            failure = "execution_failed";
    }
    const double wall_seconds = std::chrono::duration<double>(Clock::now() - started).count();
    {
        std::ofstream tape(o.out / "online.actions");
        tape.exceptions(std::ios::badbit | std::ios::failbit);
        for (auto action : actions)
            tape << action << '\n';
    }
    if (o.log_every) {
        std::ofstream log(o.out / "decisions.tsv");
        log.exceptions(std::ios::badbit | std::ios::failbit);
        log << "update\tstage\tspell\tx\ty\tyoukai\taction\tdepth\tcontroller_ms\n"
            << std::setprecision(9);
        for (const auto &l : logs)
            log << l.update << '\t' << l.stage << '\t' << l.spell << '\t' << l.x << '\t' << l.y
                << '\t' << l.youkai << '\t' << l.action << '\t' << l.depth << '\t' << l.milliseconds
                << '\n';
    }
    const double sum = std::accumulate(timings.begin(), timings.end(), 0.0);
    std::sort(timings.begin(), timings.end());
    auto quantile = [&](double q) {
        return timings.empty() ? 0.0
                               : timings[std::min(timings.size() - 1,
                                                  std::size_t(std::ceil(q * timings.size())) - 1)];
    };
    std::ofstream report(o.out / "online.json");
    report.exceptions(std::ios::badbit | std::ios::failbit);
    report << std::setprecision(17) << "{\"schema_version\":1,\"profile\":\""
           << (o.policy.queued_shots || o.policy.queued_lasers
                   ? "native-memory-kinematic-current-program-v1"
               : o.policy.spawn_guard > 0 ? "native-memory-kinematic-actor-risk-v1"
                                          : "native-memory-kinematic-v1")
           << "\",\"execution_mode\":"
              "\"forward-measured\",\"risk_parser\":\"line-tsv-v1\",\"future_native_updates\":0,"
              "\"rewinds\":0,\"deadline_enforced\":false,\"outcome\":\""
           << outcome << "\",\"failure\":\"" << failure << "\",\"binary_sha256\":\"" << binary_hash
           << "\",\"dat_sha256\":\"" << dat_hash << "\",\"actions_sha256\":\""
           << hash(o.out / "online.actions") << "\",\"prefix_sha256\":\"" << prefix_hash
           << "\",\"prefix_frames\":" << prefix.size() << ",\"stage\":" << o.stage
           << ",\"difficulty\":" << o.difficulty << ",\"seed\":" << o.seed << ",\"shot\":" << o.shot
           << ",\"spell\":" << o.spell << ",\"story\":" << o.story
           << ",\"final_route\":" << int(o.route)
           << ",\"unlock_final_routes\":" << config.unlock_final_routes
           << ",\"horizon\":" << o.policy.horizon << ",\"beam\":" << o.policy.beam
           << ",\"diversity_cell\":" << o.policy.diversity_cell
           << ",\"commit\":1,\"grow\":" << o.policy.grow
           << ",\"boss_follow\":" << o.policy.boss_follow
           << ",\"reach_filter\":" << o.policy.reach_filter << ",\"risk\":" << !o.risk.empty()
           << ",\"risk_phantoms\":" << o.policy.risk_phantoms
           << ",\"risk_hard\":" << o.policy.risk_hard
           << ",\"risk_stop_age\":" << o.policy.risk_stop_age
           << ",\"risk_weight\":" << o.policy.risk_weight
           << ",\"nowarn_weight\":" << o.policy.nowarn_weight
           << ",\"focus_bias\":" << o.policy.focus_bias
           << ",\"spawn_guard\":" << o.policy.spawn_guard << ",\"frame_budget\":" << o.frames
           << ",\"queued_shots\":" << o.policy.queued_shots
           << ",\"queued_shot_records\":" << queued_records
           << ",\"unsupported_queued_shots\":" << unsupported_queued
           << ",\"queued_forecast_coverage\":\"partial-current-main-cursors\""
           << ",\"queued_lasers\":" << o.policy.queued_lasers
           << ",\"queued_laser_records\":" << queued_laser_records
           << ",\"unsupported_queued_lasers\":" << unsupported_queued_lasers
           << ",\"queued_laser_coverage\":\"partial-current-ecl-cursors\""
           << ",\"held_fallback\":" << o.policy.held_fallback
           << ",\"direct_turn_envelope\":" << o.policy.direct_turn_envelope
           << ",\"turn_envelope_frames\":" << search_stats.turn_envelope_frames
           << ",\"turn_hint_records\":" << turn_hint_records
           << ",\"unsupported_turn_hints\":" << unsupported_turn_hints
           << ",\"direct_turn_coverage\":\"observed-or-literal-batch-rate-hypothesis\""
           << ",\"held_paths\":" << search_stats.held_paths
           << ",\"held_forecast_updates\":" << search_stats.held_updates
           << ",\"held_selected\":" << search_stats.held_selected
           << ",\"seconds_budget\":" << o.seconds << ",\"frames\":" << actions.size()
           << ",\"native_update_attempts\":" << attempts << ",\"native_updates\":" << actions.size()
           << ",\"calculation_updates\":" << hl::replay_calculation_updates() - initial_calculations
           << ",\"control_updates\":" << controls << ",\"failed_input\":" << failed_input
           << ",\"final_stage\":" << state.stage << ",\"final_spell\":" << state.spell
           << ",\"final_spell_active\":" << state.spell_active << ",\"deaths\":" << state.deaths
           << ",\"score\":" << state.score << ",\"rng_seed\":" << state.rng_seed
           << ",\"rng_draws\":" << state.rng_draws << ",\"decisions\":" << timings.size()
           << ",\"mean_controller_ms\":" << (timings.empty() ? 0 : sum / timings.size())
           << ",\"p99_controller_ms\":" << quantile(.99)
           << ",\"p999_controller_ms\":" << quantile(.999)
           << ",\"max_controller_ms\":" << quantile(1) << ",\"deadline_ms\":" << o.deadline_ms
           << ",\"deadline_misses\":" << deadline_misses
           << ",\"full_depth_decisions\":" << full_depth
           << ",\"diversity_selections\":" << search_stats.selections
           << ",\"diversity_candidates\":" << search_stats.candidates
           << ",\"diversity_bins\":" << search_stats.bins
           << ",\"diversity_retained_bins\":" << search_stats.retained_bins
           << ",\"diversity_selection_ms\":" << search_stats.selection_ms
           << ",\"native_seconds_excluding_file_io\":" << double(native_ns) / 1e9
           << ",\"setup_seconds\":" << setup_seconds << ",\"wall_seconds\":" << wall_seconds
           << ",\"stage_transitions\":[";
    for (std::size_t i = 0; i < transitions.size(); ++i) {
        const auto &t = transitions[i];
        report << (i ? "," : "") << "{\"update\":" << t.update
               << ",\"calculations_before\":" << t.before << ",\"calculations_after\":" << t.after
               << ",\"from_stage\":" << t.from << ",\"to_stage\":" << t.to << '}';
    }
    report << "],\"model_files\":[";
    for (std::size_t i = 0; i < model_hashes.size(); ++i)
        report << (i ? "," : "") << "{\"file\":\"" << model_hashes[i].first << "\",\"sha256\":\""
               << model_hashes[i].second << "\"}";
    report << "]}\n";
    report.close();
    std::cout << outcome << " frames=" << actions.size() << " decisions=" << timings.size()
              << " deadline_misses=" << deadline_misses << '\n';
    return outcome == "complete" ? 0 : outcome == "death" ? 4 : outcome == "budget" ? 3 : 5;
}
} // namespace
int main(int argc, char **argv) {
    try {
        return run(parse(argc, argv));
    } catch (const std::exception &) {
        std::cerr << "th08_online: setup_or_output_failed\n";
        return 2;
    }
}
