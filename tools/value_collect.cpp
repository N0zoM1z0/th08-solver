// Offline branch labels from an owned native checkpoint. Every proposed input
// sequence gets a separate fork with its own mutable world and RNG. No child
// observation or outcome enters the parent's online-policy inputs.
#include "modern/headless/session.hpp"
#include "th08/online.hpp"
#include "th08/online_native.hpp"
#include "th08/online_value.hpp"
#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;
namespace on = th08::online;
namespace hl = th08::headless;
using Clock = std::chrono::steady_clock;
namespace {
struct Options {
    fs::path dat, out;
    int stage = 5, difficulty = 3, spell = 107, seed = 0, shot = 0;
    unsigned frames = 20000, stride = 128, checkpoints = 32, seconds = 900;
    bool collect = true;
};
unsigned integer(const std::string &text, unsigned maximum) {
    unsigned value = 0;
    auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (text.empty() || parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
        value > maximum)
        throw std::runtime_error("invalid_collect_option");
    return value;
}
Options options(int argc, char **argv) {
    Options o;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc)
            throw std::runtime_error("missing_collect_option");
        std::string key = argv[i], value = argv[i + 1];
        if (key == "--dat")
            o.dat = fs::absolute(value);
        else if (key == "--out")
            o.out = fs::absolute(value);
        else if (key == "--stage")
            o.stage = integer(value, 8);
        else if (key == "--difficulty")
            o.difficulty = integer(value, 4);
        else if (key == "--spell")
            o.spell = integer(value, 221);
        else if (key == "--seed")
            o.seed = integer(value, 65535);
        else if (key == "--shot")
            o.shot = integer(value, 11);
        else if (key == "--frames")
            o.frames = integer(value, 20000);
        else if (key == "--stride")
            o.stride = integer(value, 20000);
        else if (key == "--checkpoints")
            o.checkpoints = integer(value, 32);
        else if (key == "--seconds")
            o.seconds = integer(value, 1800);
        else if (key == "--collect")
            o.collect = integer(value, 1);
        else
            throw std::runtime_error("unknown_collect_option");
    }
    if (o.dat.empty() || o.out.empty() || fs::exists(o.out) || !o.frames || !o.stride || !o.seconds)
        throw std::runtime_error("invalid_collect_configuration");
    return o;
}
std::string digest(const char *bytes, std::size_t size) {
    std::array<unsigned char, 32> data;
    unsigned length = 0;
    if (EVP_Digest(bytes, size, data.data(), &length, EVP_sha256(), nullptr) != 1 || length != 32)
        throw std::runtime_error("cannot_hash_collection_identity");
    std::ostringstream output;
    for (const auto value : data)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(value);
    return output.str();
}
std::string file_hash(const fs::path &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("cannot_hash_collection_file");
    std::string bytes((std::istreambuf_iterator<char>(input)), {});
    if (!input.eof() && input.fail())
        throw std::runtime_error("cannot_read_collection_file");
    return digest(bytes.data(), bytes.size());
}
enum class LabelStatus : unsigned { Survived, Death, Complete, Fault };
struct Label {
    unsigned magic = 0x42564c31, updates = 0;
    LabelStatus status = LabelStatus::Fault;
    std::uint64_t native_ns = 0, io_ns = 0;
};
bool transfer(int descriptor, void *bytes, std::size_t length, bool writing) {
    auto *cursor = static_cast<char *>(bytes);
    while (length) {
        const auto count =
            writing ? write(descriptor, cursor, length) : read(descriptor, cursor, length);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            return false;
        cursor += count;
        length -= std::size_t(count);
    }
    return true;
}
Label label_branch(hl::Session &session, const on::ValueProposal &proposal,
                   const fs::path &scratch) {
    if (proposal.inputs.size() != on::kValueHorizon - 1)
        throw std::runtime_error("invalid_proposal_prefix");
    for (const auto action : proposal.inputs)
        if (!(action & 1) || (action & ~0xf5u))
            throw std::runtime_error("invalid_proposal_action_space");
    fs::create_directories(scratch);
    int descriptors[2];
    if (pipe(descriptors) != 0)
        throw std::runtime_error("label_pipe_failed");
    std::fflush(nullptr);
    const auto child = fork();
    if (child < 0) {
        close(descriptors[0]);
        close(descriptors[1]);
        throw std::runtime_error("label_fork_failed");
    }
    if (child == 0) {
        close(descriptors[0]);
        alarm(10);
        Label result;
        try {
            fs::current_path(scratch);
            const auto initial = session.state();
            if (!initial.spell_active || initial.player_state == 2 || initial.deaths != 0)
                throw std::runtime_error("invalid_label_checkpoint");
            result.status = LabelStatus::Survived;
            for (unsigned update = 0; update < on::kValueLabelUpdates; ++update) {
                const auto action =
                    proposal.inputs[std::min<std::size_t>(update, proposal.inputs.size() - 1)];
                const auto io_before = session.file_io_time_ns();
                const auto before = Clock::now();
                const auto state = session.step(action);
                result.native_ns +=
                    std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - before)
                        .count();
                result.io_ns += session.file_io_time_ns() - io_before;
                ++result.updates;
                if (state.player_state == 2 || state.deaths > initial.deaths) {
                    result.status = LabelStatus::Death;
                    break;
                }
                if (!state.spell_active) {
                    result.status = LabelStatus::Complete;
                    break;
                }
                if (state.retry_menu)
                    throw std::runtime_error("unsupported_label_terminal");
            }
        } catch (...) {
            result.status = LabelStatus::Fault;
        }
        const bool written = transfer(descriptors[1], &result, sizeof(result), true);
        close(descriptors[1]);
        _exit(written ? 0 : 5);
    }
    close(descriptors[1]);
    Label result;
    const bool received = transfer(descriptors[0], &result, sizeof(result), false);
    close(descriptors[0]);
    int status = 0;
    while (waitpid(child, &status, 0) < 0)
        if (errno != EINTR)
            throw std::runtime_error("label_wait_failed");
    if (!received || !WIFEXITED(status) || WEXITSTATUS(status) != 0 || result.magic != 0x42564c31 ||
        result.updates > on::kValueLabelUpdates || result.updates == 0 ||
        unsigned(result.status) > unsigned(LabelStatus::Fault) ||
        (result.status == LabelStatus::Survived && result.updates != on::kValueLabelUpdates))
        result.status = LabelStatus::Fault;
    return result;
}
const char *label_name(LabelStatus status) {
    switch (status) {
    case LabelStatus::Survived:
        return "survived";
    case LabelStatus::Death:
        return "death";
    case LabelStatus::Complete:
        return "complete";
    default:
        return "fault";
    }
}
int run(const Options &o, const std::string &binary) {
    const auto dat_hash = file_hash(o.dat), binary_hash = file_hash(binary);
    fs::create_directories(o.out / "native");
    fs::current_path(o.out / "native");
    std::ostringstream identity;
    identity << "branch-data-v1;" << dat_hash << ';' << binary_hash << ';' << o.stage << ';'
             << o.difficulty << ';' << o.shot << ';' << o.spell << ';' << o.seed
             << ";h24-b192-c16-g16-hard-motion";
    const auto identity_bytes = identity.str();
    const auto episode = digest(identity_bytes.data(), identity_bytes.size());
    hl::Config config;
    const auto dat = o.dat.string();
    config.dat_path = dat.c_str();
    config.stage = o.stage;
    config.difficulty = o.difficulty;
    config.shot = o.shot;
    config.spell = o.spell;
    config.seed = std::uint16_t(o.seed);
    hl::Session session(config);
    on::Configuration policy_config;
    policy_config.horizon = on::kValueHorizon;
    policy_config.beam = 192;
    policy_config.diversity_cell = 16;
    policy_config.spawn_guard = 16;
    policy_config.reach_filter = policy_config.risk_phantoms = policy_config.hard_phantoms = true;
    policy_config.motion_envelope = true;
    on::Policy policy(policy_config);
    on::Observation observation;
    std::ofstream dataset(o.out / "dataset.tsv"), tape(o.out / "nominal.actions"),
        branch_inputs(o.out / "branch-inputs.tsv");
    dataset.exceptions(std::ios::failbit | std::ios::badbit);
    tape.exceptions(std::ios::failbit | std::ios::badbit);
    branch_inputs.exceptions(std::ios::failbit | std::ios::badbit);
    branch_inputs << "episode\tupdate\tproposal\tprefix_sha256\tinputs\n";
    dataset << "th08_branch_dataset_v1\t24\t64\t24\n"
            << on::branch_dataset_header() << '\n'
            << std::setprecision(9);
    std::string prefix, outcome = "budget";
    std::vector<double> timings;
    std::uint64_t label_updates = 0, label_ns = 0, label_io_ns = 0, nominal_ns = 0,
                  nominal_io_ns = 0;
    unsigned checkpoints = 0, branches = 0, bad = 0, positives = 0, rows = 0;
    std::uint64_t trace = 1469598103934665603ULL;
    const auto mix = [&](std::uint64_t word) { trace = (trace ^ word) * 1099511628211ULL; };
    const auto mix_float = [&](float value) {
        std::uint32_t word;
        std::memcpy(&word, &value, 4);
        mix(word);
    };
    auto state = session.state();
    bool seen_spell = state.spell_active;
    const auto start = Clock::now();
    for (unsigned update = 0; update < o.frames; ++update) {
        if (std::chrono::duration<double>(Clock::now() - start).count() >= o.seconds)
            break;
        const bool sample = o.collect && state.spell_active && update >= 128 &&
                            update % o.stride == 0 && checkpoints < o.checkpoints;
        const auto before = Clock::now();
        on::capture_memory(session, update, observation, true, 0, 0, false, true);
        std::vector<on::ValueProposal> proposals;
        const auto decision = policy.decide(observation, nullptr, sample ? &proposals : nullptr);
        timings.push_back(std::chrono::duration<double, std::milli>(Clock::now() - before).count());
        mix(update);
        mix_float(observation.x);
        mix_float(observation.y);
        mix(session.actor_digest());
        for (const auto &bullet : observation.bullets) {
            mix(bullet.slot);
            mix(bullet.state);
            mix(bullet.collision_disabled);
            mix(bullet.active_transforms);
            mix_float(bullet.x);
            mix_float(bullet.y);
            mix_float(bullet.vx);
            mix_float(bullet.vy);
        }
        if (sample && !proposals.empty()) {
            ++checkpoints;
            const auto prefix_hash = digest(prefix.data(), prefix.size());
            for (unsigned candidate = 0; candidate < proposals.size(); ++candidate) {
                branch_inputs << episode << '\t' << update << '\t' << candidate << '\t'
                              << prefix_hash << '\t';
                bool first = true;
                for (const auto input : proposals[candidate].inputs) {
                    if (!first)
                        branch_inputs << ',';
                    branch_inputs << input;
                    first = false;
                }
                branch_inputs << '\n';
                const auto label = label_branch(session, proposals[candidate],
                                                o.out / "branches" / std::to_string(branches));
                ++branches;
                label_updates += label.updates;
                label_ns += label.native_ns;
                label_io_ns += label.io_ns;
                bad += label.status == LabelStatus::Fault;
                positives +=
                    label.status == LabelStatus::Survived || label.status == LabelStatus::Complete;
                dataset << episode << '\t' << o.seed << '\t' << o.stage << '\t' << o.difficulty
                        << '\t' << o.shot << '\t' << o.spell << '\t' << update << '\t' << candidate
                        << '\t' << label_name(label.status) << '\t' << label.updates << '\t'
                        << prefix_hash;
                for (const auto value : proposals[candidate].features)
                    dataset << '\t' << value;
                dataset << '\n';
                ++rows;
            }
        }
        const auto input = decision.action;
        if (!(input & 1) || (input & ~0xf5u))
            throw std::runtime_error("nominal_action_space_mismatch");
        prefix += std::to_string(input) + '\n';
        tape << input << '\n';
        const auto io_before = session.file_io_time_ns();
        const auto native_before = Clock::now();
        state = session.step(input);
        seen_spell = seen_spell || state.spell_active;
        nominal_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - native_before)
                .count();
        nominal_io_ns += session.file_io_time_ns() - io_before;
        if (state.player_state == 2 || state.deaths != 0) {
            outcome = "death";
            break;
        }
        if (seen_spell && !state.spell_active) {
            outcome = "complete";
            break;
        }
        if (state.retry_menu) {
            outcome = "runtime_error";
            break;
        }
    }
    tape.close();
    dataset.close();
    branch_inputs.close();
    const auto wall = std::chrono::duration<double>(Clock::now() - start).count();
    double sum = 0;
    for (const auto ms : timings)
        sum += ms;
    std::sort(timings.begin(), timings.end());
    const auto quantile = [&](double fraction) {
        return timings.empty() ? 0 : timings[std::size_t(fraction * (timings.size() - 1))];
    };
    std::ofstream report(o.out / "collection.json");
    report.exceptions(std::ios::failbit | std::ios::badbit);
    report << std::setprecision(17)
           << "{\"schema_version\":2,\"scope\":\"offline-native-branch-labels\","
           << "\"episode\":\"" << episode << "\",\"dat_sha256\":\"" << dat_hash
           << "\",\"binary_sha256\":\"" << binary_hash << "\",\"dataset_sha256\":\""
           << file_hash(o.out / "dataset.tsv") << "\",\"actions_sha256\":\""
           << file_hash(o.out / "nominal.actions") << "\",\"branch_inputs_sha256\":\""
           << file_hash(o.out / "branch-inputs.tsv") << "\",\"stage\":" << o.stage
           << ",\"difficulty\":" << o.difficulty << ",\"shot\":" << o.shot
           << ",\"spell\":" << o.spell << ",\"seed\":" << o.seed << ",\"outcome\":\"" << outcome
           << "\",\"nominal_updates\":" << timings.size() << ",\"score\":" << state.score
           << ",\"rng_seed\":" << state.rng_seed << ",\"rng_draws\":" << state.rng_draws
           << ",\"nominal_trace_digest\":\"" << trace << '"'
           << ",\"calculation_updates\":" << state.calculation_updates
           << ",\"checkpoints\":" << checkpoints << ",\"branches\":" << branches
           << ",\"rows\":" << rows << ",\"positive\":" << positives << ",\"bad_branches\":" << bad
           << ",\"label_native_updates\":" << label_updates
           << ",\"nominal_native_seconds_excluding_io\":"
           << double(nominal_ns - nominal_io_ns) / 1e9
           << ",\"label_native_seconds_excluding_io\":" << double(label_ns - label_io_ns) / 1e9
           << ",\"mean_nominal_controller_ms\":" << (timings.empty() ? 0 : sum / timings.size())
           << ",\"p99_nominal_controller_ms\":" << quantile(.99)
           << ",\"max_nominal_controller_ms\":" << quantile(1) << ",\"wall_seconds\":" << wall
           << ",\"input_horizon\":24,\"label_updates\":64,\"deadline_enforced\":false}\n";
    return bad || outcome == "runtime_error" ? 5 : 0;
}
} // namespace
int main(int argc, char **argv) {
    try {
        const auto binary = fs::canonical("/proc/self/exe").string();
        const auto o = options(argc, argv);
        return run(o, binary);
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
