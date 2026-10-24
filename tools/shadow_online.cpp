#include "modern/headless/session.hpp"
#include "th08/online_native.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_compare.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_plan.hpp"
#include "th08/shadow_proposal.hpp"
#include "wine-shadow/file_hash.hpp"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>

namespace {
namespace s = th08::shadow;
namespace b = s::bridge;
namespace h = th08::headless;
namespace o = th08::online;
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
constexpr unsigned lead_updates = 8;
constexpr unsigned proposal_budget_ms = 60;
struct Mapping {
    std::uint32_t *data = nullptr;
    explicit Mapping(const fs::path &path) {
        const int file = open(path.c_str(), O_RDWR);
        if (file < 0)
            throw std::runtime_error("cannot_open_shadow_commands");
        const auto size = lseek(file, 0, SEEK_END);
        void *memory = size == b::map_bytes ? mmap(nullptr, b::map_bytes, PROT_READ | PROT_WRITE,
                                                   MAP_SHARED, file, 0)
                                            : MAP_FAILED;
        close(file);
        if (memory == MAP_FAILED)
            throw std::runtime_error("cannot_map_shadow_commands");
        data = static_cast<std::uint32_t *>(memory);
        if (b::load(data + b::Magic) != b::magic || b::load(data + b::Version) != b::version) {
            munmap(data, b::map_bytes);
            data = nullptr;
            throw std::runtime_error("wrong_shadow_command_version");
        }
    }
    ~Mapping() {
        if (data) {
            if (b::load(data + b::Outcome) == unsigned(b::OutcomeCode::Running))
                b::save(data + b::StopRequested, 1);
            munmap(data, b::map_bytes);
        }
    }
};
struct Scratch {
    fs::path previous = fs::current_path(), directory;
    Scratch() {
        char path[] = "/tmp/th08-wine-shadow-XXXXXX";
        if (!mkdtemp(path))
            throw std::runtime_error("shadow_scratch_failed");
        directory = path;
        fs::current_path(directory);
    }
    ~Scratch() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(directory, error);
    }
};
struct Statistics {
    std::uint64_t searches = 0, candidates = 0, verified = 0, stale = 0, invalid_prefix = 0;
    std::uint64_t terminal_before_change = 0;
    std::uint64_t warm_attempts = 0, warm_selected = 0;
    std::uint64_t branch_attempts = 0, branch_completed = 0, branch_charged = 0;
    std::uint64_t branch_native_ns = 0, branch_io_ns = 0, owner_native_ns = 0;
    std::uint64_t guidance_ns = 0, guidance_requests = 0;
    std::uint64_t rollout_timeouts = 0, rollout_deaths = 0, proposal_timeouts = 0,
                  proposal_deaths = 0;
    std::uint64_t unqueued_updates = 0, unqueued_streak = 0, max_unqueued_streak = 0;
    std::uint64_t first_unqueued = 0, last_unqueued = 0, plan_invalidations = 0;
    std::uint64_t follow_attempts = 0, follow_completed = 0;
    std::uint64_t drift_updates = 0, drift_words = 0, first_drift = 0;
    std::uint64_t inactive_updates = 0, inactive_words = 0;
    float max_player_error = 0;
    std::uint64_t proposal_attempts = 0, proposal_completed = 0, proposal_charged = 0;
    std::uint64_t proposal_native_ns = 0, proposal_io_ns = 0;
    double elapsed_ms = 0;
    std::vector<double> planning_ms, periods_ms;
};
double percentile(std::vector<double> values, double q) {
    if (values.empty())
        return 0;
    std::sort(values.begin(), values.end());
    return values[std::min(values.size() - 1, std::size_t(q * double(values.size() - 1)))];
}
std::uint64_t diagnostic_value(const s::Packet &packet, const char *field) {
    if (!field)
        return 0;
    if (std::string(field) == "sequence")
        return packet.sequence;
    if (std::string(field) == "epoch")
        return packet.epoch;
    if (std::string(field) == "actual_input")
        return packet.input;
    for (unsigned i = 0; i < s::field_names.size(); ++i)
        if (std::string(field) == s::field_names[i])
            return packet.state.values[i];
    return 0;
}
std::uint64_t trace_word(const b::Trace &bytes, unsigned index) {
    std::uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i)
        value |= std::uint64_t(bytes[s::Bytes{}.size() + index * 8 + i]) << (i * 8);
    return value;
}
struct Follower {
    h::Session &session;
    s::Packet actual;
    s::ActionBuffer queue;
    o::Policy &policy;
    Statistics &statistics;
    std::deque<s::Packet> history;
    std::uint64_t offset = 0, last_elapsed = 0;
    int descriptor;
    const char *mismatch = nullptr;
    s::Packet retail;
    bool exact = false;
    Follower(h::Session &session_, o::Policy &policy_, Statistics &statistics_,
             const fs::path &path)
        : session(session_), queue(s::Packet{}), policy(policy_), statistics(statistics_) {
        actual.state = s::capture(session);
        queue = s::ActionBuffer(actual);
        descriptor = open(path.c_str(), O_RDONLY);
        if (descriptor < 0)
            throw std::runtime_error("cannot_open_shadow_journal");
    }
    ~Follower() {
        close(descriptor);
    }
    unsigned drain() {
        unsigned count = 0;
        b::Trace bytes;
        for (;;) {
            const auto size = pread(descriptor, bytes.data(), bytes.size(), off_t(offset));
            if (size < 0 && errno == EINTR)
                continue;
            if (size < 0)
                throw std::runtime_error("cannot_read_shadow_journal");
            if (size != ssize_t(bytes.size()))
                return count; // Writer may still be appending this record.
            retail = s::decode(bytes.data(), s::Bytes{}.size());
            const auto begin = Clock::now();
            const auto io = session.file_io_time_ns();
            ++statistics.follow_attempts;
            try {
                session.step(std::uint16_t(retail.input));
            } catch (...) {
                const auto total =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin)
                        .count();
                const auto file_io = session.file_io_time_ns() - io;
                statistics.owner_native_ns +=
                    std::uint64_t(total) > file_io ? std::uint64_t(total) - file_io : 0;
                throw;
            }
            ++statistics.follow_completed;
            const auto total =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
            const auto file_io = session.file_io_time_ns() - io;
            statistics.owner_native_ns +=
                std::uint64_t(total) > file_io ? std::uint64_t(total) - file_io : 0;
            s::advance_receipt(actual, retail.input, s::capture(session));
            offset += bytes.size();
            ++count;
            const auto comparison = s::compare_live(retail, actual, exact);
            if (comparison.numeric_words) {
                ++statistics.drift_updates;
                statistics.drift_words += comparison.numeric_words;
                if (!statistics.first_drift)
                    statistics.first_drift = actual.sequence;
            }
            statistics.max_player_error =
                std::max(statistics.max_player_error, comparison.player_error);
            if (comparison.inactive_words) {
                ++statistics.inactive_updates;
                statistics.inactive_words += comparison.inactive_words;
            }
            if ((mismatch = comparison.failure))
                return count;
            const bool had_plan = queue.next() != nullptr;
            const bool matched_plan = queue.confirm(actual);
            if (had_plan && !matched_plan)
                ++statistics.plan_invalidations;
            if (!matched_plan) {
                ++statistics.unqueued_updates;
                ++statistics.unqueued_streak;
                statistics.max_unqueued_streak =
                    std::max(statistics.max_unqueued_streak, statistics.unqueued_streak);
                if (!statistics.first_unqueued)
                    statistics.first_unqueued = actual.sequence;
                statistics.last_unqueued = actual.sequence;
            } else
                statistics.unqueued_streak = 0;
            history.push_back(actual);
            if (history.size() > 256)
                history.pop_front();
            const auto elapsed = trace_word(bytes, 0);
            if (actual.sequence > 1)
                statistics.periods_ms.push_back(double(elapsed - last_elapsed) / 1e6);
            last_elapsed = elapsed;
            statistics.elapsed_ms = double(elapsed) / 1e6;
            o::Observation observation;
            o::capture_memory(session, actual.sequence, observation, true);
            policy.observe(observation);
        }
    }
    std::vector<s::Packet> executed_since(std::uint64_t sequence) const {
        std::vector<s::Packet> records;
        for (const auto &frame : history)
            if (frame.sequence > sequence)
                records.push_back(frame);
        return records;
    }
};

// Proposals remain heuristic. The future's complete true-RNG native execution,
// including the forced immutable lead, decides whether it can enter the queue.
struct PlanCandidate {
    s::Prediction prediction;
    unsigned unchanged_inputs = lead_updates;
};
PlanCandidate search(Follower &follower, const s::Prediction &previous, s::Boundary boundary,
                     Statistics &statistics, unsigned horizon) {
    const auto begin = Clock::now();
    constexpr unsigned lead = lead_updates;
    const unsigned search_budget_ms = horizon == 64 ? 110 : 80;
    const auto origin = follower.actual;
    PlanCandidate selected;
    ++statistics.searches;
    const auto finish = [&] {
        statistics.planning_ms.push_back(
            std::chrono::duration<double, std::milli>(Clock::now() - begin).count());
        return std::move(selected);
    };
    const auto record = [&](const s::Prediction &prediction) {
        ++statistics.candidates;
        statistics.branch_attempts += prediction.attempts;
        statistics.branch_completed += prediction.completed;
        statistics.branch_charged += prediction.charged_updates;
        statistics.branch_native_ns += prediction.native_ns;
        statistics.branch_io_ns += prediction.file_io_ns;
        statistics.guidance_ns += prediction.guidance_ns;
        statistics.guidance_requests += prediction.guidance_requests;
        statistics.rollout_timeouts += prediction.status == s::BranchStatus::Timeout;
        statistics.rollout_deaths += prediction.status == s::BranchStatus::Death;
        if (prediction.status == s::BranchStatus::Fault)
            throw std::runtime_error("shadow_native_branch_fault");
    };
    // A deterministic shadow already knows the verified nominal continuation.
    // Re-execute it in a fresh owned branch and extend its tail, saving repeated
    // proposal searches while still requiring a complete new native horizon.
    if (horizon == 64 && previous.certifies() && previous.origin.sequence <= origin.sequence) {
        const auto consumed = origin.sequence - previous.origin.sequence;
        if (consumed < previous.frames.size() &&
            follower.queue.size() == previous.frames.size() - consumed &&
            follower.queue.size() > lead_updates) {
            std::vector<std::uint16_t> warm;
            for (auto i = std::size_t(consumed); i < previous.frames.size(); ++i)
                warm.push_back(std::uint16_t(previous.frames[i].input));
            ++statistics.warm_attempts;
            auto prediction = s::predict_with_policy(follower.session, follower.policy, origin,
                                                     warm, horizon, boundary, search_budget_ms);
            record(prediction);
            if (prediction.certifies()) {
                ++statistics.verified;
                ++statistics.warm_selected;
                selected.unchanged_inputs = warm.size();
                selected.prediction = std::move(prediction);
                return finish();
            }
        }
    }
    std::vector<std::uint16_t> prefix;
    auto last = std::uint16_t(origin.input);
    for (unsigned i = 0; i < lead; ++i) {
        const auto sequence = origin.sequence + i + 1;
        last = b::fallback(last, sequence, boundary.spell >= 0);
        if (follower.queue.size() > i && sequence > previous.origin.sequence &&
            sequence <= previous.origin.sequence + previous.frames.size())
            last = std::uint16_t(previous.frames[sequence - previous.origin.sequence - 1].input);
        prefix.push_back(last);
    }
    const auto elapsed_before_proposal =
        std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    if (elapsed_before_proposal >= search_budget_ms)
        return finish();
    const auto proposals = s::propose_after_lead(
        follower.session, follower.policy, origin, prefix, boundary,
        std::min(proposal_budget_ms,
                 std::max(1u, unsigned(search_budget_ms - elapsed_before_proposal))));
    statistics.proposal_attempts += proposals.attempts;
    statistics.proposal_completed += proposals.completed;
    statistics.proposal_charged += proposals.charged_updates;
    statistics.proposal_native_ns += proposals.native_ns;
    statistics.proposal_io_ns += proposals.file_io_ns;
    statistics.proposal_timeouts += proposals.status == s::BranchStatus::Timeout;
    statistics.proposal_deaths += proposals.status == s::BranchStatus::Death;
    if (proposals.status == s::BranchStatus::Fault)
        throw std::runtime_error("shadow_native_proposal_fault");
    std::vector<std::vector<std::uint16_t>> candidates;
    for (const auto &proposal : proposals.inputs) {
        auto inputs = prefix;
        inputs.insert(inputs.end(), proposal.begin(), proposal.end());
        while (horizon == 32 && inputs.size() < horizon)
            inputs.push_back(inputs.back());
        candidates.push_back(std::move(inputs));
    }
    for (unsigned focus : {4u, 0u})
        for (unsigned direction : {0u, 16u, 32u, 64u, 128u, 80u, 144u, 96u, 160u}) {
            auto inputs = prefix;
            while (inputs.size() < 32)
                inputs.push_back(std::uint16_t(1 | focus | direction));
            candidates.push_back(std::move(inputs));
        }
    for (auto &inputs : candidates) {
        if (boundary.spell < 0) {
            for (unsigned i = lead; i < inputs.size(); ++i)
                inputs[i] = std::uint16_t((inputs[i] & ~0x1000) |
                                          (((origin.sequence + i + 1) & 1) ? 0x1000 : 0));
        }
        const auto elapsed =
            std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
        if (elapsed >= search_budget_ms)
            break;
        const auto timeout = std::max(1u, unsigned(search_budget_ms - elapsed));
        const auto prediction =
            horizon == 32 ? s::predict(follower.session, origin, inputs, boundary, timeout)
                          : s::predict_with_policy(follower.session, follower.policy, origin,
                                                   inputs, horizon, boundary, timeout);
        record(prediction);
        if (prediction.certifies()) {
            ++statistics.verified;
            selected.prediction = prediction;
            break;
        }
    }
    return finish();
}

int run(int argc, char **argv) {
    if ((argc != 6 && argc != 7 && argc != 8) ||
        (std::string(argv[4]) != "follow" && std::string(argv[4]) != "mpc") ||
        (argc >= 7 && std::string(argv[6]) != "exact" && std::string(argv[6]) != "behavioral") ||
        (argc == 8 && std::string(argv[7]) != "h32" && std::string(argv[7]) != "h64"))
        throw std::runtime_error("usage: th08_shadow_online DAT RUN_DIRECTORY OUTPUT follow|mpc "
                                 "WALL_SECONDS [behavioral|exact] [h32|h64]");
    const auto dat = fs::absolute(argv[1]).string();
    const auto directory = fs::absolute(argv[2]), output = fs::absolute(argv[3]);
    const bool mpc = std::string(argv[4]) == "mpc";
    const unsigned verified_horizon = argc == 8 && std::string(argv[7]) == "h64" ? 64 : 32;
    const unsigned minimum_search_interval = verified_horizon == 64 ? 8 : 4;
    const unsigned search_budget_ms = verified_horizon == 64 ? 110 : 80;
    const unsigned replan_remaining = verified_horizon - minimum_search_interval;
    std::size_t used;
    const auto seconds = std::stoul(argv[5], &used);
    if (used != std::string(argv[5]).size() || !seconds || seconds > 7200)
        throw std::runtime_error("invalid_shadow_wall_budget");
    const auto deadline = Clock::now() + std::chrono::seconds(seconds);
    while (!fs::exists(directory / "start.txt")) {
        if (Clock::now() >= deadline)
            throw std::runtime_error("shadow_start_timeout");
        poll(nullptr, 0, 10);
    }
    h::Config config;
    std::ifstream start(directory / "start.txt");
    std::string version, retail_hash;
    unsigned seed, story, max_updates, row_size;
    if (!(start >> version >> seed >> config.stage >> config.difficulty >> config.shot >> story >>
          config.spell >> max_updates >> row_size >> retail_hash) ||
        version != "TH08_SHADOW_START_V1" || seed > 65535 || story > 1 ||
        row_size != b::trace_bytes ||
        retail_hash != "330fbdbf58a710829d65277b4f312cfbb38d5448b3df523e79350b879213d924")
        throw std::runtime_error("invalid_shadow_start_record");
    config.seed = std::uint16_t(seed);
    config.story = story;
    config.input_profile = h::InputProfile::SampledGameplay;
    config.dat_path = dat.c_str();
    Mapping map(directory / "commands.bin");
    const auto dat_hash = wine_shadow::file_hash(dat),
               binary_hash = wine_shadow::file_hash("/proc/self/exe");
    Scratch scratch;
    h::Session session(config);
    o::Configuration profile;
    profile.horizon = 24;
    profile.beam = 192;
    profile.diversity_cell = 16;
    profile.spawn_guard = 16;
    profile.reach_filter = profile.hard_phantoms = profile.motion_envelope = true;
    o::Policy policy(profile);
    Statistics statistics;
    Follower follower(session, policy, statistics, directory / "frames.bin");
    follower.exact = argc >= 7 && std::string(argv[6]) == "exact";
    s::Prediction previous;
    std::uint32_t generation = 0;
    std::uint64_t last_search = 0;
    bool timed_out = false, native_fault = false;
    try {
        while (!follower.mismatch) {
            follower.drain();
            if (follower.mismatch)
                break;
            if (b::load(map.data + b::Outcome) != unsigned(b::OutcomeCode::Running)) {
                follower.drain();
                if (fs::file_size(directory / "frames.bin") != follower.offset ||
                    b::load(map.data + b::Sequence) != follower.actual.sequence)
                    throw std::runtime_error("truncated_shadow_journal");
                break;
            }
            if (Clock::now() >= deadline) {
                timed_out = true;
                b::save(map.data + b::StopRequested, 1);
                break;
            }
            if (generation && b::load(map.data + b::Rejected) == generation)
                follower.queue = s::ActionBuffer(follower.actual);
            const bool pending = generation && b::load(map.data + b::Accepted) != generation &&
                                 b::load(map.data + b::Rejected) != generation;
            if (mpc && follower.actual.sequence && !pending &&
                follower.queue.size() <= replan_remaining &&
                follower.actual.sequence >= last_search + minimum_search_interval) {
                last_search = follower.actual.sequence;
                const auto candidate = search(follower, previous, {config.story, config.spell},
                                              statistics, verified_horizon);
                const auto &prediction = candidate.prediction;
                follower.drain();
                if (follower.mismatch)
                    break;
                if (prediction.certifies()) {
                    if (prediction.status == s::BranchStatus::Terminal &&
                        prediction.frames.size() <= candidate.unchanged_inputs) {
                        ++statistics.terminal_before_change;
                        continue;
                    }
                    b::Plan plan;
                    plan.generation = generation + 2;
                    plan.origin = prediction.origin.sequence;
                    plan.epoch = prediction.origin.epoch;
                    plan.first_change = plan.origin + candidate.unchanged_inputs + 1;
                    plan.count = prediction.frames.size();
                    for (unsigned i = 0; i < plan.count; ++i)
                        plan.inputs[i] = std::uint16_t(prediction.frames[i].input);
                    if (!b::timely(plan, b::load(map.data + b::Sequence)))
                        ++statistics.stale;
                    else if (!follower.queue.replace(prediction,
                                                     follower.executed_since(plan.origin)))
                        ++statistics.invalid_prefix;
                    else {
                        b::publish(map.data, plan);
                        generation = plan.generation;
                        previous = prediction;
                    }
                }
            }
            poll(nullptr, 0, 1);
        }
    } catch (const std::exception &error) {
        native_fault = true;
        std::cerr << error.what() << '\n';
    }
    if ((follower.mismatch || native_fault) &&
        b::load(map.data + b::Outcome) == unsigned(b::OutcomeCode::Running)) {
        b::save(map.data + b::StopRequested, 1);
    }
    if (follower.mismatch) {
        const auto details = s::diagnostics(session);
        std::ofstream dump(output.string() + ".bullets.tsv");
        dump
            << "sequence " << follower.actual.sequence << " cw " << details.x87_control
            << "\nslot state transforms x y vx vy width height ax ay relative_angle relative_speed "
               "bullet_angle bullet_speed acceleration_angle acceleration_magnitude accel_timer "
               "accel_duration relative_timer\n";
        unsigned index = 0;
        for (const auto &bullet : session.bullets()) {
            const auto &detail = details.bullets.at(index++);
            if (detail.slot != bullet.slot)
                throw std::runtime_error("nonmatching_shadow_diagnostic_slot");
            dump << bullet.slot << ' ' << bullet.state << ' ' << bullet.active_transforms;
            for (float value :
                 {bullet.x, bullet.y, bullet.vx, bullet.vy, bullet.full_width, bullet.full_height,
                  bullet.vector_acceleration_x, bullet.vector_acceleration_y,
                  bullet.relative_direction.angle, bullet.relative_direction.speed, detail.angle,
                  detail.speed, detail.acceleration_angle, detail.acceleration_magnitude}) {
                std::uint32_t bits;
                std::memcpy(&bits, &value, sizeof(bits));
                dump << ' ' << bits;
            }
            dump << ' ' << bullet.vector_acceleration_timer << ' '
                 << bullet.vector_acceleration_duration << ' ' << bullet.relative_direction.timer
                 << '\n';
        }
    }
    const auto outcome = b::load(map.data + b::Outcome);
    const bool actual_failed = outcome == unsigned(b::OutcomeCode::Death) ||
                               follower.retail.state[s::Field::PlayerState] == 2 ||
                               follower.retail.state[s::Field::Deaths] ||
                               follower.retail.state[s::Field::BombsUsed];
    const bool actual_complete = outcome == unsigned(b::OutcomeCode::Complete) && !actual_failed;
    const bool practical_win = !follower.exact && actual_complete;
    const bool practical_death = !follower.exact && actual_failed;
    const char *name = practical_win                                   ? "complete"
                       : practical_death                               ? "death"
                       : native_fault                                  ? "native_fault"
                       : follower.mismatch                             ? "desynchronized"
                       : timed_out                                     ? "wall_budget"
                       : outcome == unsigned(b::OutcomeCode::Complete) ? "complete"
                       : outcome == unsigned(b::OutcomeCode::Death)    ? "death"
                       : outcome == unsigned(b::OutcomeCode::Budget)   ? "update_budget"
                       : outcome == unsigned(b::OutcomeCode::Failure)  ? "owner_fault"
                                                                       : "owner_stopped";
    std::ofstream report(output);
    if (!report)
        throw std::runtime_error("cannot_write_shadow_report");
    std::ofstream actions(output.string() + ".actions");
    for (std::uint64_t offset = 0; offset < follower.offset; offset += b::trace_bytes) {
        b::Trace record;
        if (pread(follower.descriptor, record.data(), record.size(), off_t(offset)) !=
            ssize_t(record.size()))
            throw std::runtime_error("cannot_export_actual_shadow_inputs");
        actions << s::decode(record.data(), s::Bytes{}.size()).input << '\n';
    }
    actions.close();
    if (!actions)
        throw std::runtime_error("cannot_export_actual_shadow_inputs");
    report << "{\"schema_version\":1,\"profile\":\"wine-true-rng-shadow-v1\",\"mode\":\""
           << (mpc ? "mpc" : "follow") << "\",\"outcome\":\"" << name << "\",\"mismatch\":\""
           << (follower.mismatch ? follower.mismatch : "")
           << "\",\"updates\":" << follower.actual.sequence << ",\"seed\":" << seed
           << ",\"stage\":" << config.stage << ",\"difficulty\":" << config.difficulty
           << ",\"shot\":" << config.shot << ",\"spell\":" << config.spell << ",\"sync_mode\":\""
           << (follower.exact ? "exact" : "behavioral") << '"'
           << ",\"input_profile\":\"sampled-gameplay\""
           << ",\"actual_complete\":" << (actual_complete ? "true" : "false")
           << ",\"actual_nmnb_failed\":" << (actual_failed ? "true" : "false")
           << ",\"actual_player_state\":" << follower.retail.state[s::Field::PlayerState]
           << ",\"actual_deaths_bits\":" << follower.retail.state[s::Field::Deaths]
           << ",\"actual_bombs_used_bits\":" << follower.retail.state[s::Field::BombsUsed]
           << ",\"final_stage\":" << session.state().stage
           << ",\"actual_final_stage\":" << follower.retail.state[s::Field::Stage]
           << ",\"handoffs\":" << follower.actual.epoch
           << ",\"calculation_updates\":" << session.state().calculation_updates
           << ",\"numeric_drift_updates\":" << statistics.drift_updates
           << ",\"numeric_drift_words\":" << statistics.drift_words
           << ",\"first_numeric_drift_update\":" << statistics.first_drift
           << ",\"max_player_error_px\":" << statistics.max_player_error
           << ",\"inactive_metadata_updates\":" << statistics.inactive_updates
           << ",\"inactive_metadata_words\":" << statistics.inactive_words
           << ",\"lead_updates\":" << lead_updates << ",\"verified_horizon\":" << verified_horizon
           << ",\"replan_remaining\":" << replan_remaining
           << ",\"minimum_search_interval\":" << minimum_search_interval
           << ",\"search_budget_ms\":" << search_budget_ms
           << ",\"proposal_budget_ms\":" << proposal_budget_ms << ",\"continuation\":\""
           << (verified_horizon == 64 ? "policy-prefix8" : "fixed") << '"'
           << ",\"policy_horizon\":24,\"policy_beam\":192,\"policy_diversity_cell\":16"
           << ",\"proposal_boundary\":\"after-forced-lead\",\"search_rng\":\"branch-private\""
           << ",\"start_sha256\":\"" << wine_shadow::file_hash((directory / "start.txt").string())
           << "\",\"trace_sha256\":\""
           << wine_shadow::file_hash((directory / "frames.bin").string())
           << "\",\"actions_sha256\":\"" << wine_shadow::file_hash(output.string() + ".actions")
           << '"' << ",\"dat_sha256\":\"" << dat_hash << "\",\"native_sha256\":\"" << binary_hash
           << "\",\"retail_sha256\":\"" << retail_hash << "\",\"searches\":" << statistics.searches
           << ",\"candidates\":" << statistics.candidates << ",\"verified\":" << statistics.verified
           << ",\"stale\":" << statistics.stale
           << ",\"terminal_before_change\":" << statistics.terminal_before_change
           << ",\"warm_attempts\":" << statistics.warm_attempts
           << ",\"warm_selected\":" << statistics.warm_selected
           << ",\"invalid_prefix\":" << statistics.invalid_prefix
           << ",\"planned_input_calls\":" << b::load(map.data + b::Planned)
           << ",\"fallback_input_calls\":" << b::load(map.data + b::Fallback)
           << ",\"rejected_generation\":" << b::load(map.data + b::Rejected)
           << ",\"last_rejection_reason\":" << b::load(map.data + b::RejectionReason)
           << ",\"branch_attempts\":" << statistics.branch_attempts
           << ",\"branch_completed\":" << statistics.branch_completed
           << ",\"branch_charged\":" << statistics.branch_charged
           << ",\"proposal_attempts\":" << statistics.proposal_attempts
           << ",\"proposal_completed\":" << statistics.proposal_completed
           << ",\"proposal_charged\":" << statistics.proposal_charged
           << ",\"proposal_native_ms\":" << double(statistics.proposal_native_ns) / 1e6
           << ",\"proposal_io_ms\":" << double(statistics.proposal_io_ns) / 1e6
           << ",\"branch_native_ms\":" << double(statistics.branch_native_ns) / 1e6
           << ",\"branch_io_ms\":" << double(statistics.branch_io_ns) / 1e6
           << ",\"continuation_ms\":" << double(statistics.guidance_ns) / 1e6
           << ",\"continuation_requests\":" << statistics.guidance_requests
           << ",\"rollout_timeouts\":" << statistics.rollout_timeouts
           << ",\"rollout_deaths\":" << statistics.rollout_deaths
           << ",\"proposal_timeouts\":" << statistics.proposal_timeouts
           << ",\"proposal_deaths\":" << statistics.proposal_deaths
           << ",\"unqueued_updates\":" << statistics.unqueued_updates
           << ",\"max_unqueued_streak\":" << statistics.max_unqueued_streak
           << ",\"first_unqueued_update\":" << statistics.first_unqueued
           << ",\"last_unqueued_update\":" << statistics.last_unqueued
           << ",\"plan_invalidations\":" << statistics.plan_invalidations
           << ",\"follow_native_ms\":" << double(statistics.owner_native_ns) / 1e6
           << ",\"follow_attempts\":" << statistics.follow_attempts
           << ",\"follow_completed\":" << statistics.follow_completed
           << ",\"planning_p99_ms\":" << percentile(statistics.planning_ms, 0.99)
           << ",\"planning_max_ms\":" << percentile(statistics.planning_ms, 1)
           << ",\"planning_mean_ms\":"
           << (statistics.planning_ms.empty() ? 0
                                              : std::accumulate(statistics.planning_ms.begin(),
                                                                statistics.planning_ms.end(), 0.) /
                                                    statistics.planning_ms.size())
           << ",\"planning_budget_overruns\":"
           << std::count_if(statistics.planning_ms.begin(), statistics.planning_ms.end(),
                            [search_budget_ms](double ms) { return ms > search_budget_ms; })
           << ",\"owner_elapsed_ms\":" << statistics.elapsed_ms
           << ",\"owner_period_p99_ms\":" << percentile(statistics.periods_ms, .99)
           << ",\"owner_period_max_ms\":" << percentile(statistics.periods_ms, 1)
           << ",\"owner_period_misses_60hz\":"
           << std::count_if(statistics.periods_ms.begin(), statistics.periods_ms.end(),
                            [](double ms) { return ms > 1000. / 60.; })
           << ",\"deaths\":" << session.state().deaths
           << ",\"bombs_used_bits\":" << follower.actual.state[s::Field::BombsUsed]
           << ",\"retail_value\":" << diagnostic_value(follower.retail, follower.mismatch)
           << ",\"native_value\":" << diagnostic_value(follower.actual, follower.mismatch)
           << ",\"state_differences\":[";
    bool first_difference = true;
    if (follower.mismatch)
        for (unsigned i = 0; i < s::field_names.size(); ++i)
            if (follower.retail.state.values[i] != follower.actual.state.values[i]) {
                report << (first_difference ? "" : ",") << "{\"field\":\"" << s::field_names[i]
                       << "\",\"retail\":" << follower.retail.state.values[i]
                       << ",\"native\":" << follower.actual.state.values[i] << '}';
                first_difference = false;
            }
    report << "]}\n";
    std::cout << name << " updates=" << follower.actual.sequence
              << " verified=" << statistics.verified
              << " mismatch=" << (follower.mismatch ? follower.mismatch : "") << '\n';
    return practical_win                                                  ? 0
           : practical_death                                              ? 4
           : native_fault || outcome == unsigned(b::OutcomeCode::Failure) ? 1
           : follower.mismatch                                            ? 3
           : outcome == unsigned(b::OutcomeCode::Death)                   ? 4
           : outcome == unsigned(b::OutcomeCode::Complete)                ? 0
                                                                          : 5;
}
} // namespace
int main(int argc, char **argv) try {
    return run(argc, argv);
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
