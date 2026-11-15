// Source-profile 60 Hz execution. Native state belongs exclusively to the main
// thread; one policy worker sees owned current observations and no native future.
#include "th08/online_live.hpp"
#include "th08/online_native.hpp"
#include "th08/plan_route.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <openssl/evp.h>
#include <sstream>
#include <thread>
#include <unistd.h>

namespace fs = std::filesystem;
namespace on = th08::online;
namespace live = th08::online::live;
namespace hl = th08::headless;
namespace plan = th08::plan;
namespace {
using Clock = live::Clock;
void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
struct Options {
    fs::path dat, out, risk, value;
    on::Configuration policy;
    int stage = 0, difficulty = 3, seed = 0, shot = 0, spell = -1;
    bool story = false;
    plan::FinalRoute route = plan::FinalRoute::Natural;
    std::uint64_t frames = 500000, queue = 4, tolerance_us = 0;
    double seconds = 7200;
};
Options options(int argc, char **argv) {
    Options o;
    const std::array<std::string, 9> stages{"1", "2", "3", "4a", "4b", "5", "6a", "6b", "extra"};
    for (int i = 1; i < argc; i += 2) {
        require(i + 1 < argc, "missing_live_option");
        const std::string key(argv[i]), value(argv[i + 1]);
        const auto integer = [&] { return plan::integer<std::uint64_t>(value, key); };
        const auto number = [&] { return plan::real(value, key); };
        const auto boolean = [&] {
            const auto n = integer();
            require(n <= 1, "invalid_live_boolean");
            return bool(n);
        };
        if (key == "--dat")
            o.dat = fs::absolute(value);
        else if (key == "--out")
            o.out = fs::absolute(value);
        else if (key == "--risk-dir")
            o.risk = fs::absolute(value);
        else if (key == "--value-model")
            o.value = fs::absolute(value);
        else if (key == "--stage") {
            const auto found = std::find(stages.begin(), stages.end(), value);
            require(found != stages.end(), "invalid_live_stage");
            o.stage = int(found - stages.begin());
        } else if (key == "--difficulty") {
            const auto n = integer();
            require(n <= 4, "invalid_live_difficulty");
            o.difficulty = int(n);
        } else if (key == "--seed") {
            const auto n = integer();
            require(n <= 65535, "invalid_live_seed");
            o.seed = int(n);
        } else if (key == "--shot") {
            const auto n = integer();
            require(n <= 11, "invalid_live_shot");
            o.shot = int(n);
        } else if (key == "--spell") {
            const auto n = integer();
            require(n <= 221, "invalid_live_spell");
            o.spell = int(n);
        } else if (key == "--story")
            o.story = boolean();
        else if (key == "--final-route") {
            require(value == "natural" || value == "a" || value == "b", "invalid_live_route");
            o.route = value == "a"   ? plan::FinalRoute::A
                      : value == "b" ? plan::FinalRoute::B
                                     : plan::FinalRoute::Natural;
        } else if (key == "--frames")
            o.frames = integer();
        else if (key == "--seconds")
            o.seconds = number();
        else if (key == "--history-queue")
            o.queue = integer();
        else if (key == "--scheduler-tolerance-us")
            o.tolerance_us = integer();
        else if (key == "--horizon") {
            const auto n = integer();
            require(n <= 200, "invalid_live_horizon");
            o.policy.horizon = int(n);
        } else if (key == "--beam") {
            const auto n = integer();
            require(n <= 8192, "invalid_live_beam");
            o.policy.beam = int(n);
        } else if (key == "--diversity-cell") {
            const auto n = integer();
            require(n <= 128, "invalid_live_cell");
            o.policy.diversity_cell = int(n);
        } else if (key == "--spawn-guard")
            o.policy.spawn_guard = float(number());
        else if (key == "--grow")
            o.policy.grow = float(number());
        else if (key == "--boss-follow")
            o.policy.boss_follow = float(number());
        else if (key == "--focus-bias")
            o.policy.focus_bias = float(number());
        else if (key == "--value-weight")
            o.policy.value_weight = float(number());
        else if (key == "--risk-weight")
            o.policy.risk_weight = float(number());
        else if (key == "--nowarn-weight")
            o.policy.nowarn_weight = float(number());
        else if (key == "--risk-hard")
            o.policy.risk_hard = boolean();
        else if (key == "--risk-stop-age")
            o.policy.risk_stop_age = boolean();
        else if (key == "--reach-filter")
            o.policy.reach_filter = boolean();
        else if (key == "--risk-phantoms")
            o.policy.risk_phantoms = boolean();
        else if (key == "--hard-phantoms")
            o.policy.hard_phantoms = boolean();
        else if (key == "--motion-envelope")
            o.policy.motion_envelope = boolean();
        else if (key == "--child-births")
            o.policy.child_births = boolean();
        else if (key == "--retain-headings")
            o.policy.retain_headings = boolean();
        else if (key == "--queued-shots")
            o.policy.queued_shots = boolean();
        else if (key == "--queued-lasers")
            o.policy.queued_lasers = boolean();
        else if (key == "--spawn-clock")
            o.policy.spawn_clock = boolean();
        else if (key == "--relative-turns")
            o.policy.relative_turns = boolean();
        else if (key == "--direct-turn-envelope")
            o.policy.direct_turn_envelope = boolean();
        else
            throw std::runtime_error("unknown_live_option");
    }
    require(!o.dat.empty() && !o.out.empty() && o.frames > 0 && o.frames <= 1000000 &&
                o.seconds > 0 && o.queue > 0 && o.queue <= 64 && o.tolerance_us <= 1000000,
            "invalid_live_configuration");
    require(!(o.story && o.spell >= 0), "unsupported_live_story_spell");
    return o;
}
std::string hash(const fs::path &path) {
    std::ifstream input(path, std::ios::binary);
    auto context =
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    require(bool(input) && context && EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) == 1,
            "live_hash_failed");
    std::array<char, 65536> buffer;
    while (input) {
        input.read(buffer.data(), buffer.size());
        require(EVP_DigestUpdate(context.get(), buffer.data(), input.gcount()) == 1,
                "live_hash_failed");
    }
    std::array<unsigned char, 32> digest;
    unsigned length = 0;
    require(input.eof() && EVP_DigestFinal_ex(context.get(), digest.data(), &length) == 1 &&
                length == 32,
            "live_hash_failed");
    std::ostringstream output;
    for (const auto byte : digest)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return output.str();
}
std::string json_string(const std::string &value) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : value) {
        if (c == '"' || c == '\\')
            out << '\\' << char(c);
        else if (c < 32)
            out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(c);
        else
            out << char(c);
    }
    out << '"';
    return out.str();
}
struct Scratch {
    fs::path previous = fs::current_path(), directory;
    Scratch() {
        char name[] = "/tmp/th08-live-XXXXXX";
        require(mkdtemp(name) != nullptr, "live_scratch_failed");
        directory = name;
        fs::current_path(directory);
    }
    ~Scratch() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(directory, error);
    }
};
struct Log {
    std::uint64_t update, epoch;
    int stage, spell;
    std::uint16_t latch, input;
    bool fallback;
    double tick_late_ms, owner_ms;
};
struct Transition {
    std::uint64_t update, before, after;
    int from, to;
};
double milliseconds(Clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}
int run(const Options &o) {
    const auto setup_begin = Clock::now();
    require(!fs::exists(o.out), "live_output_exists");
    fs::create_directories(o.out);
    const auto dat_hash = hash(o.dat), binary_hash = hash("/proc/self/exe");
    std::string model_hash, model_header, model_continuation;
    on::RiskTables risk;
    on::BranchValueModel value;
    std::vector<std::pair<std::string, std::string>> risk_hashes;
    if (!o.risk.empty()) {
        risk = on::load_risk_tables(o.risk.string());
        for (const auto *name : {"risk_table_merged.tsv", "risk_backoff_seg.tsv",
                                 "risk_backoff_global.tsv", "nowarn_merged.tsv"})
            risk_hashes.emplace_back(name, hash(o.risk / name));
    }
    if (!o.value.empty()) {
        model_hash = hash(o.value);
        value = on::load_branch_value(o.value.string());
        model_continuation = on::branch_continuation_name(value.continuation);
        std::ifstream input(o.value);
        std::getline(input, model_header);
    }
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
    config.unlock_final_routes = o.route != plan::FinalRoute::Natural;
    hl::Session session(config);
    live::Worker worker(o.policy, std::move(risk), std::move(value), o.queue);
    auto state = session.state();
    const auto initial_calculations = state.calculation_updates;
    std::vector<std::uint16_t> actions;
    actions.reserve(o.frames);
    std::vector<Log> logs;
    logs.reserve(o.frames);
    std::vector<Transition> transitions;
    std::uint64_t epoch = 0, attempts = 0, controls = 0, fallbacks = 0, overruns = 0;
    std::uint64_t native_ns = 0, native_io_ns = 0, capture_ns = 0, mailbox_ns = 0;
    std::uint64_t birth_records = 0, unsupported_births = 0;
    bool started_spell = false;
    std::string outcome = "budget", failure;
    const auto tolerance = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::microseconds(o.tolerance_us));
    const auto capture = [&] {
        const auto begin = Clock::now();
        on::Observation observation;
        on::capture_memory(session, actions.size(), observation, o.policy.spawn_guard > 0,
                           o.policy.queued_shots ? unsigned(o.policy.horizon) : 0,
                           o.policy.queued_lasers ? unsigned(o.policy.horizon) : 0,
                           o.policy.direct_turn_envelope,
                           o.policy.motion_envelope || !o.value.empty(),
                           o.policy.child_births ? unsigned(o.policy.horizon) : 0);
        birth_records += observation.child_births.size();
        unsupported_births += observation.unsupported_child_births;
        capture_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
        return observation;
    };
    auto initial = capture();
    const auto start = Clock::now();
    const auto setup_seconds = std::chrono::duration<double>(start - setup_begin).count();
    const auto submit = [&](on::Observation observation) {
        const auto begin = Clock::now();
        worker.submit(
            {std::move(observation), epoch, start + live::tick_offset(actions.size() + 1)});
        mailbox_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
    };
    submit(std::move(initial));
    try {
        while (actions.size() < o.frames &&
               std::chrono::duration<double>(Clock::now() - start).count() < o.seconds) {
            const auto update = actions.size();
            const auto tick = start + live::tick_offset(update + 1);
            const auto next_tick = start + live::tick_offset(update + 2);
            std::this_thread::sleep_until(tick);
            const auto owner_begin = Clock::now();
            if (live::deadline_overrun(next_tick, owner_begin, tolerance)) {
                ++overruns;
                throw std::runtime_error("live_executor_period_overrun");
            }
            require(worker.failure().empty(), "live_policy_failed");
            const auto mailbox_begin = Clock::now();
            auto reply = worker.take(update, epoch, tick);
            mailbox_ns +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - mailbox_begin)
                    .count();
            const bool fallback = !reply;
            std::uint16_t input =
                reply ? reply->decision.action : std::uint16_t((state.latched_input & 0xf4) | 1);
            fallbacks += fallback;
            require((input & 1) && !(input & ~0xf5u), "invalid_live_policy_input");
            if (o.spell < 0 && ((update + 1) % 2))
                input |= 4096;
            if (const auto route = plan::final_route_input(o.route, state, update + 1))
                input = *route;
            const auto before = state;
            const auto origin_epoch = epoch;
            const auto native_begin = Clock::now();
            const auto io_begin = session.file_io_time_ns();
            ++attempts;
            try {
                state = session.step(input);
            } catch (...) {
                const auto io = session.file_io_time_ns() - io_begin;
                native_io_ns += io;
                const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() -
                                                                                     native_begin)
                                    .count();
                native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
                throw;
            }
            const auto io = session.file_io_time_ns() - io_begin;
            native_io_ns += io;
            const auto ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - native_begin)
                    .count();
            native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
            actions.push_back(input);
            // Fatal submissions still belong to the exact input/timing record.
            logs.push_back({update, origin_epoch, before.stage,
                            before.spell_active ? before.spell : -1, before.latched_input, input,
                            fallback, milliseconds(owner_begin - tick),
                            milliseconds(Clock::now() - owner_begin)});
            const auto calculations = state.calculation_updates - before.calculation_updates;
            const bool changed = state.stage != before.stage;
            controls += calculations == 0;
            if (changed)
                transitions.push_back(
                    {actions.size(), before.calculation_updates - initial_calculations,
                     state.calculation_updates - initial_calculations, before.stage, state.stage});
            require(plan::supported_recording_step(calculations, changed),
                    "unsupported_live_calculation_boundary");
            require(o.spell < 0 || !state.spell_active || state.spell == o.spell,
                    "live_spell_mismatch");
            require(o.route == plan::FinalRoute::Natural || state.stage < 6 || state.stage > 7 ||
                        state.stage == (o.route == plan::FinalRoute::A ? 6 : 7),
                    "live_route_mismatch");
            require(state.deaths == 0 && state.player_state != 2, "death");
            if (changed || before.spell_active != state.spell_active || before.spell != state.spell)
                ++epoch;
            started_spell = started_spell || state.spell_active;
            const bool complete = o.spell >= 0 ? (started_spell && !state.spell_active)
                                  : o.story
                                      ? plan::story_terminal(state.stage, state.stage_complete, 8)
                                      : state.stage_complete;
            if (complete)
                outcome = "complete";
            if (!complete && actions.size() < o.frames)
                submit(capture());
            logs.back().owner_ms = milliseconds(Clock::now() - owner_begin);
            if (live::deadline_overrun(next_tick, Clock::now(), tolerance)) {
                ++overruns;
                throw std::runtime_error("live_executor_period_overrun");
            }
            if (complete)
                break;
        }
    } catch (const std::exception &error) {
        failure = error.what();
        if (failure.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789") != std::string::npos)
            failure = "live_execution_failed";
        outcome = failure == "death"                          ? "death"
                  : failure == "live_executor_period_overrun" ? "deadline"
                                                              : "runtime_error";
    }
    const auto stop = Clock::now();
    worker.stop();
    const auto joined = Clock::now();
    if (!worker.failure().empty()) {
        failure = worker.failure();
        outcome = "runtime_error";
    }
    const auto stats = worker.statistics();
    {
        std::ofstream tape(o.out / "online.actions");
        tape.exceptions(std::ios::badbit | std::ios::failbit);
        for (auto input : actions)
            tape << input << '\n';
    }
    {
        std::ofstream log(o.out / "ticks.tsv");
        log.exceptions(std::ios::badbit | std::ios::failbit);
        log << "update\tepoch\tstage\tspell\tlatched_input\tsubmitted_input\tfallback\ttick_late_"
               "ms\towner_ms\n"
            << std::setprecision(17);
        for (const auto &l : logs)
            log << l.update << '\t' << l.epoch << '\t' << l.stage << '\t' << l.spell << '\t'
                << l.latch << '\t' << l.input << '\t' << l.fallback << '\t' << l.tick_late_ms
                << '\t' << l.owner_ms << '\n';
    }
    std::ofstream report(o.out / "online.json");
    report.exceptions(std::ios::badbit | std::ios::failbit);
    report << std::setprecision(17)
           << "{\"schema_version\":1,\"execution_mode\":\"unpaused-owned-policy-worker-60hz-v1\","
              "\"clock_scope\":\"recording-api-updates-including-control-handoffs\",\"deadline_"
              "enforced\":true,\"future_native_updates\":0,\"rewinds\":0,\"planner_lag\":0,"
              "\"outcome\":"
           << json_string(outcome) << ",\"failure\":" << json_string(failure)
           << ",\"dat_sha256\":" << json_string(dat_hash)
           << ",\"binary_sha256\":" << json_string(binary_hash)
           << ",\"actions_sha256\":" << json_string(hash(o.out / "online.actions"))
           << ",\"seed\":" << o.seed << ",\"stage\":" << o.stage
           << ",\"difficulty\":" << o.difficulty << ",\"shot\":" << o.shot
           << ",\"spell\":" << o.spell << ",\"story\":" << o.story
           << ",\"final_route\":" << int(o.route) << ",\"horizon\":" << o.policy.horizon
           << ",\"beam\":" << o.policy.beam << ",\"diversity_cell\":" << o.policy.diversity_cell
           << ",\"spawn_guard\":" << o.policy.spawn_guard << ",\"grow\":" << o.policy.grow
           << ",\"boss_follow\":" << o.policy.boss_follow
           << ",\"focus_bias\":" << o.policy.focus_bias
           << ",\"reach_filter\":" << o.policy.reach_filter
           << ",\"risk_phantoms\":" << o.policy.risk_phantoms
           << ",\"hard_phantoms\":" << o.policy.hard_phantoms
           << ",\"motion_envelope\":" << o.policy.motion_envelope
           << ",\"child_births\":" << o.policy.child_births
           << ",\"child_birth_records\":" << birth_records
           << ",\"unsupported_child_births\":" << unsupported_births
           << ",\"retain_headings\":" << o.policy.retain_headings
           << ",\"risk_hard\":" << o.policy.risk_hard
           << ",\"risk_stop_age\":" << o.policy.risk_stop_age
           << ",\"risk_weight\":" << o.policy.risk_weight
           << ",\"nowarn_weight\":" << o.policy.nowarn_weight
           << ",\"queued_shots\":" << o.policy.queued_shots
           << ",\"queued_lasers\":" << o.policy.queued_lasers
           << ",\"spawn_clock\":" << o.policy.spawn_clock
           << ",\"relative_turns\":" << o.policy.relative_turns
           << ",\"direct_turn_envelope\":" << o.policy.direct_turn_envelope
           << ",\"risk\":" << !o.risk.empty() << ",\"value_model\":" << !o.value.empty()
           << ",\"value_weight\":" << o.policy.value_weight
           << ",\"value_model_sha256\":" << json_string(model_hash)
           << ",\"value_model_continuation\":" << json_string(model_continuation)
           << ",\"value_model_header\":" << json_string(model_header)
           << ",\"frame_budget\":" << o.frames << ",\"seconds_budget\":" << o.seconds
           << ",\"frames\":" << actions.size() << ",\"native_update_attempts\":" << attempts
           << ",\"native_updates\":" << actions.size()
           << ",\"calculation_updates\":" << state.calculation_updates - initial_calculations
           << ",\"control_updates\":" << controls << ",\"final_stage\":" << state.stage
           << ",\"final_spell\":" << state.spell << ",\"final_spell_active\":" << state.spell_active
           << ",\"score\":" << state.score << ",\"rng_seed\":" << state.rng_seed
           << ",\"rng_draws\":" << state.rng_draws << ",\"deaths\":" << state.deaths
           << ",\"fallback\":\"hold-actual-latched-movement-focus-and-shoot\",\"fallback_updates\":"
           << fallbacks << ",\"policy_deadline_misses\":" << fallbacks
           << ",\"executor_period_overruns\":" << overruns
           << ",\"scheduler_tolerance_us\":" << o.tolerance_us
           << ",\"history_queue_capacity\":" << o.queue << ",\"queue_peak\":" << stats.queue_peak
           << ",\"worker_submitted\":" << stats.submitted
           << ",\"worker_decisions\":" << stats.decisions
           << ",\"worker_observe_only\":" << stats.observed_only
           << ",\"stale_replies\":" << stats.stale_replies
           << ",\"overwritten_replies\":" << stats.overwritten_replies
           << ",\"abandoned_requests\":" << stats.abandoned_requests
           << ",\"abandoned_replies\":" << stats.abandoned_replies
           << ",\"worker_failed_requests\":" << stats.failed_requests
           << ",\"worker_seconds\":" << double(stats.worker_ns) / 1e9
           << ",\"worker_decision_seconds\":" << double(stats.decision_ns) / 1e9
           << ",\"worker_observe_seconds\":" << double(stats.observe_ns) / 1e9
           << ",\"worker_failed_seconds\":" << double(stats.failed_ns) / 1e9
           << ",\"value_evaluations\":" << stats.value_evaluations
           << ",\"value_ms\":" << stats.value_ms << ",\"setup_seconds\":" << setup_seconds
           << ",\"capture_seconds\":" << double(capture_ns) / 1e9
           << ",\"mailbox_seconds\":" << double(mailbox_ns) / 1e9
           << ",\"native_seconds_excluding_file_io\":" << double(native_ns) / 1e9
           << ",\"native_file_io_seconds\":" << double(native_io_ns) / 1e9
           << ",\"wall_seconds\":" << std::chrono::duration<double>(stop - start).count()
           << ",\"shutdown_join_seconds\":" << std::chrono::duration<double>(joined - stop).count()
           << ",\"expected_tick_seconds\":" << double(actions.size()) / 60
           << ",\"worker_waited_on_tick\":false,\"catch_up_updates\":0,\"strict_wall_complete\":"
           << (outcome == "complete" && overruns == 0 ? "true" : "false") << ",\"model_files\":[";
    for (std::size_t i = 0; i < risk_hashes.size(); ++i)
        report << (i ? "," : "") << "{\"file\":" << json_string(risk_hashes[i].first)
               << ",\"sha256\":" << json_string(risk_hashes[i].second) << '}';
    report << "],\"stage_transitions\":[";
    for (std::size_t i = 0; i < transitions.size(); ++i) {
        const auto &t = transitions[i];
        report << (i ? "," : "") << "{\"update\":" << t.update << ",\"before\":" << t.before
               << ",\"after\":" << t.after << ",\"from\":" << t.from << ",\"to\":" << t.to << '}';
    }
    report << "]}\n";
    std::cout << outcome << " frames=" << actions.size() << " fallbacks=" << fallbacks
              << " period_overruns=" << overruns << '\n';
    return outcome == "complete"   ? 0
           : outcome == "budget"   ? 3
           : outcome == "death"    ? 4
           : outcome == "deadline" ? 6
                                   : 5;
}
} // namespace
int main(int argc, char **argv) {
    try {
        return run(options(argc, argv));
    } catch (const std::exception &) {
        std::cerr << "th08_online_live: setup_or_output_failed\n";
        return 2;
    }
}
