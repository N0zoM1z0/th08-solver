#include "headless_report.hpp"
#include <array>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
namespace {
using Clock = std::chrono::steady_clock;
std::uint64_t integer(const std::string &text, std::uint64_t maximum) {
    std::size_t end = 0;
    if (text.empty() || text.front() == '-')
        throw std::runtime_error("invalid unsigned integer");
    const auto value = std::stoull(text, &end);
    if (end != text.size() || value > maximum)
        throw std::runtime_error("integer out of range");
    return value;
}
std::string read(const fs::path &path) {
    if (fs::file_size(path) > 1024 * 1024)
        throw std::runtime_error("child report too large");
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("missing child report: " + path.string());
    std::ostringstream output;
    output << input.rdbuf();
    if (input.bad())
        throw std::runtime_error("cannot read child report");
    return output.str();
}
// Read only our CLI's unique scalar fields. Preserve the complete report alongside
// the ledger; never coerce 64-bit digests through floating point.
using th08::headless_report::field;
struct Process {
    int code;
    bool interrupted;
    double wall_ms;
    long rss_kib;
};
Process run(const std::vector<std::string> &args, Clock::time_point deadline) {
    std::vector<char *> argv;
    for (const auto &arg : args)
        argv.push_back(const_cast<char *>(arg.c_str()));
    argv.push_back(nullptr);
    const auto start = Clock::now();
    const pid_t child = fork();
    if (child < 0)
        throw std::runtime_error("cannot fork native runner");
    if (!child) {
        execv(argv[0], argv.data());
        _exit(127);
    }
    int status = 0;
    rusage usage{};
    bool interrupted = false, killed = false;
    auto grace = deadline;
    for (;;) {
        const auto waited = wait4(child, &status, WNOHANG, &usage);
        if (waited == child)
            break;
        if (waited < 0 && errno != EINTR)
            throw std::runtime_error("cannot reap native runner");
        const auto now = Clock::now();
        if (!interrupted && now >= deadline) {
            interrupted = true;
            kill(child, SIGTERM);
            grace = now + std::chrono::milliseconds(250);
        } else if (interrupted && !killed && now >= grace) {
            killed = true;
            kill(child, SIGKILL);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return {WIFEXITED(status) ? WEXITSTATUS(status) : 128, interrupted,
            std::chrono::duration<double, std::milli>(Clock::now() - start).count(),
            usage.ru_maxrss};
}
std::string file_hash(const fs::path &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("cannot hash executable");
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("SHA256 initialization failed");
    char bytes[16384];
    while (input) {
        input.read(bytes, sizeof bytes);
        if (EVP_DigestUpdate(ctx.get(), bytes, std::size_t(input.gcount())) != 1)
            throw std::runtime_error("SHA256 update failed");
    }
    if (!input.eof())
        throw std::runtime_error("cannot read executable");
    unsigned char digest[32];
    unsigned length = 0;
    if (EVP_DigestFinal_ex(ctx.get(), digest, &length) != 1 || length != 32)
        throw std::runtime_error("SHA256 finalization failed");
    std::ostringstream output;
    for (auto byte : digest)
        output << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return output.str();
}
struct Audit {
    fs::path directory;
    std::ofstream journal;
    std::array<std::uint64_t, 3> known{}, uncertain{}, attempts{};
    unsigned phase = 0, cap = 0;
    std::string native_hash, repair_hash;
    explicit Audit(const fs::path &path) : directory(path), journal(path / "processes.tsv") {
        journal << "phase\tcase\tstatus\tupper_bound\texit_code\twall_ms\n";
        if (!journal)
            throw std::runtime_error("cannot open process journal");
    }
    Process execute(unsigned which, const fs::path &stem, unsigned limit,
                    const std::vector<std::string> &args, Clock::time_point deadline) {
        phase = which;
        cap = limit;
        ++attempts[phase];
        uncertain[phase] += cap;
        journal << phase << '\t' << stem.filename().string() << "\tstarted\t" << cap << "\t-1\t0\n";
        journal.flush();
        if (!journal)
            throw std::runtime_error("cannot journal process start");
        const auto result = run(args, deadline);
        journal << phase << '\t' << stem.filename().string() << '\t'
                << (result.interrupted ? "interrupted" : "exited") << '\t' << cap << '\t'
                << result.code << '\t' << result.wall_ms << '\n';
        journal.flush();
        if (!journal)
            throw std::runtime_error("cannot journal process result");
        return result;
    }
    void validated(unsigned frames) {
        known[phase] += frames;
        uncertain[phase] -= cap;
        cap = 0;
    }
    void write(const std::string &outcome, bool failed = false) {
        std::ofstream output(directory / (failed ? "summary.json" : "process-costs.json"));
        output << "{\"producer\":\"th08_headless_repair\",\"outcome\":\"" << outcome
               << "\",\"child_executable_sha256\":\"" << native_hash
               << "\",\"repair_executable_sha256\":\"" << repair_hash << "\",\"phases\":[";
        for (unsigned i = 0; i < 3; ++i) {
            if (i)
                output << ',';
            output << "{\"phase\":" << i << ",\"attempts\":" << attempts[i]
                   << ",\"verified_native_updates\":" << known[i]
                   << ",\"unverified_update_upper_bound\":" << uncertain[i] << '}';
        }
        output << "]}\n";
        if (!output)
            throw std::runtime_error("cannot write process costs");
    }
};

struct Observation {
    unsigned overlap, horizon;
    std::string digest;
};
struct Record {
    fs::path stem;
    std::string report, outcome;
    unsigned frames = 0;
    std::vector<unsigned> actions;
    std::vector<Observation> observations;
};
Record record(const fs::path &stem, int code, unsigned cap, bool trace) {
    Record result;
    result.stem = stem;
    result.report = read(stem.string() + ".json");
    result.outcome = field(result.report, "outcome");
    result.frames = unsigned(integer(field(result.report, "frames"), cap));
    const bool complete = result.outcome == "\"complete\"";
    if ((complete ? code != 0 : code != 2) ||
        (!complete && result.outcome != "\"collision\"" && result.outcome != "\"frame_limit\"" &&
         result.outcome != "\"retry_menu\"") ||
        !result.frames)
        throw std::runtime_error("child exit/outcome mismatch");
    if (integer(field(result.report, "frame_budget"), 1000000) != cap)
        throw std::runtime_error("child frame budget mismatch");
    std::ifstream tape(stem.string() + ".actions");
    std::string line;
    if (!tape)
        throw std::runtime_error("missing child action tape");
    while (std::getline(tape, line))
        result.actions.push_back(unsigned(integer(line, 65535)));
    if (!tape.eof() || result.actions.size() != result.frames)
        throw std::runtime_error("invalid child tape length");
    if (trace) {
        std::ifstream log(stem.string() + ".policy");
        if (!std::getline(log, line) || line != "frame\toverlap\thorizon\ttrace_digest")
            throw std::runtime_error("invalid policy log header");
        result.observations.push_back({0, 0, "\"1469598103934665603\""});
        while (std::getline(log, line)) {
            std::istringstream row(line);
            std::string frame, overlap, horizon, digest, extra;
            if (!(row >> frame >> overlap >> horizon >> digest) || row >> extra ||
                integer(frame, cap) != result.observations.size())
                throw std::runtime_error("invalid policy log row");
            integer(digest, UINT64_MAX);
            result.observations.push_back({unsigned(integer(overlap, 1000000)),
                                           unsigned(integer(horizon, 1000000)),
                                           "\"" + digest + "\""});
        }
        if (!log.eof() || result.observations.size() != result.frames + 1 ||
            result.observations.back().digest != field(result.report, "trace_digest"))
            throw std::runtime_error("incomplete policy trace");
    }
    return result;
}
struct RepairTrigger {
    unsigned onset = 0, horizon = 0;
};
RepairTrigger terminal_unsafe_onset(const Record &source) {
    // Derive the intervention duration from this selected failure's actual
    // proposal profile. A horizon change breaks the observed run: an H12 safe
    // observation cannot certify an H32 transition (or vice versa). Unknown
    // rows likewise supply no safety evidence. The next round derives anew;
    // candidates within this round all use this fixed source and duration.
    const unsigned horizon = source.observations.back().horizon;
    if (horizon != 12 && horizon != 32)
        return {};
    unsigned onset = 0;
    bool safe = false, unsafe = false;
    for (unsigned frame = 1; frame <= source.frames; ++frame) {
        const auto &sample = source.observations.at(frame);
        if (sample.horizon != horizon || !sample.overlap) {
            onset = 0;
            safe = false;
            unsafe = false;
        } else if (sample.overlap > sample.horizon) {
            safe = true;
            unsafe = false;
            onset = 0;
        } else if (!unsafe) {
            onset = safe ? frame : 0;
            unsafe = true;
        }
    }
    return {unsafe ? onset : 0, horizon};
}
void agree(const Record &a, const Record &b) {
    for (const char *key : {"source_revision",
                            "profile",
                            "dat_sha256",
                            "stage",
                            "requested_spell_id",
                            "difficulty",
                            "seed",
                            "frame_budget",
                            "outcome",
                            "frames",
                            "spell_id",
                            "spell_start",
                            "first_hit",
                            "deaths",
                            "player_state",
                            "rng_draws",
                            "rng_seed",
                            "graze",
                            "score",
                            "gauge",
                            "peak_bullets",
                            "trace_digest",
                            "previous_trace_digest"})
        if (field(a.report, key) != field(b.report, key))
            throw std::runtime_error(std::string("fresh replay differs: ") + key);
    const auto ca = a.report.find("\"collision\":"), cb = b.report.find("\"collision\":");
    if (ca == std::string::npos || cb == std::string::npos ||
        a.report.substr(ca) != b.report.substr(cb) || a.actions != b.actions)
        throw std::runtime_error("fresh replay collision/tape differs");
}
#include "headless_resume.hpp"
} // namespace

int main(int argc, char **argv) {
    std::unique_ptr<Audit> audit;
    try {
        fs::path executable, directory, resume_directory, resume_producer;
        std::vector<std::string> scene;
        std::map<std::string, std::string> seen;
        unsigned cap = 15000, seconds = 600, rounds = 2;
        std::uint64_t update_budget = 0;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::runtime_error("option needs a value");
            std::string key = argv[i], value = argv[i + 1];
            if (!seen.emplace(key, value).second)
                throw std::runtime_error("duplicate option");
            if (key == "--executable")
                executable = fs::absolute(value);
            else if (key == "--output-dir")
                directory = fs::absolute(value);
            else if (key == "--resume-search")
                resume_directory = fs::absolute(value);
            else if (key == "--resume-producer")
                resume_producer = fs::absolute(value);
            else if (key == "--frames")
                cap = unsigned(integer(value, 1000000));
            else if (key == "--seconds")
                seconds = unsigned(integer(value, 3600));
            else if (key == "--rounds")
                rounds = unsigned(integer(value, 2));
            else if (key == "--update-budget")
                update_budget = integer(value, UINT64_MAX);
            else if (key == "--dat") {
                scene.push_back(key);
                scene.push_back(fs::absolute(value).string());
            } else if (key == "--stage" || key == "--spell-id" || key == "--difficulty" ||
                       key == "--seed" || key == "--shoot") {
                scene.push_back(key);
                scene.push_back(value);
            } else
                throw std::runtime_error("unknown option: " + key);
        }
        if (executable.empty() || directory.empty() || scene.empty() || !cap || !seconds || !rounds)
            throw std::runtime_error(
                "executable, scene, fresh output directory and positive budgets required");
        if (!update_budget)
            update_budget = std::uint64_t(rounds) * 72 * cap;
        if (resume_directory.empty() != resume_producer.empty())
            throw std::runtime_error("resume-search requires its original producer binary");
        if (!fs::create_directory(directory))
            throw std::runtime_error("output directory must be new");
        audit = std::make_unique<Audit>(directory);
        // Pin bytes for the entire search, so rebuilding the source executable
        // cannot silently mix proposal/native profiles between candidates.
        fs::copy_file(executable, directory / "native-runner");
        executable = directory / "native-runner";
        audit->native_hash = file_hash(executable);
        audit->repair_hash = file_hash("/proc/self/exe");
        const auto overall_start = Clock::now();
        std::unique_ptr<Resume> resumed;
        if (!resume_directory.empty())
            resumed = std::make_unique<Resume>(snapshot_resume(
                resume_directory, directory / "prior", resume_producer, audit->native_hash, cap));
        auto arguments = [&](const fs::path &stem, bool trace) {
            std::vector<std::string> args{executable.string()};
            args.insert(args.end(), scene.begin(), scene.end());
            args.insert(args.end(),
                        {"--frames", std::to_string(cap), "--actions", stem.string() + ".actions",
                         "--output", stem.string() + ".json"});
            if (trace)
                args.insert(args.end(), {"--policy-log", stem.string() + ".policy"});
            return args;
        };
        const fs::path initial = directory / "initial";
        auto args = arguments(initial, true);
        args.insert(args.end(), {"--strategy", "spell-portfolio"});
        if (resumed && !resumed->prefix.empty())
            args.insert(args.end(), {"--resume-prefix", resumed->prefix.string(), "--prefix-frame",
                                     field(resumed->selected.report, "prefix_frame")});
        const auto initial_process =
            audit->execute(0, initial, cap, args, Clock::now() + std::chrono::seconds(seconds));
        if (initial_process.interrupted)
            throw std::runtime_error(
                "initial run exceeded wall budget; updates bounded by frame cap");
        auto current = record(initial, initial_process.code, cap, true);
        if (resumed) {
            agree(resumed->selected, current);
            for (const auto *key : {"prefix_frame", "prefix_trace_digest", "unused_actions"})
                if (field(resumed->selected.report, key) != field(current.report, key))
                    throw std::runtime_error("resume prefix provenance differs");
            if (resumed->selected.observations.size() != current.observations.size())
                throw std::runtime_error("resume policy history length differs");
            for (std::size_t i = 0; i < current.observations.size(); ++i) {
                const auto &old = resumed->selected.observations[i];
                const auto &fresh = current.observations[i];
                if (old.overlap != fresh.overlap || old.horizon != fresh.horizon ||
                    old.digest != fresh.digest)
                    throw std::runtime_error("resume policy history differs");
            }
        }
        for (const auto &entry : std::map<std::string, std::pair<std::string, std::string>>{
                 {"--stage", {"stage", "1"}},
                 {"--spell-id", {"requested_spell_id", "-1"}},
                 {"--difficulty", {"difficulty", "0"}},
                 {"--seed", {"seed", "0"}}}) {
            auto expected = seen.count(entry.first) ? seen.at(entry.first) : entry.second.second;
            if (entry.first == "--stage")
                expected = "\"" + expected + "\"";
            if (field(current.report, entry.second.first) != expected)
                throw std::runtime_error("initial scene identity differs from request");
        }
        audit->validated(current.frames);
        const auto search_start = Clock::now(),
                   deadline = search_start + std::chrono::seconds(seconds);
        unsigned candidates = 0;
        std::uint64_t updates = 0, uncertain_updates_bound = 0, prefix_updates = 0;
        double replay_ms = 0;
        unsigned replay_updates = 0;
        std::ofstream ledger(directory / "ledger.tsv");
        ledger << "round\tonset\trollback_segments\tbranch_frame\taction\tframes\toutcome\twall_"
                  "ms\tmaximum_rss_kib\tprefix_digest\n";
        std::string reason = current.outcome == "\"complete\"" ? "complete" : "round-limit";
        bool stop = current.outcome == "\"complete\"";
        for (unsigned round = 1; round <= rounds && !stop; ++round) {
            if (current.outcome != "\"collision\"") {
                reason = "noncollision-terminal";
                break;
            }
            const auto trigger = terminal_unsafe_onset(current);
            if (!trigger.onset) {
                reason = "no-supported-unsafe-transition";
                break;
            }
            const auto source_stem = current.stem;
            auto best = current;
            for (unsigned rollback = 1; rollback <= 8 && !stop; ++rollback) {
                const unsigned segment = trigger.horizon / 2;
                if (trigger.onset <= rollback * segment)
                    continue;
                const unsigned branch = trigger.onset - rollback * segment;
                if (branch - 1 + segment > cap)
                    continue;
                for (int y = -1; y <= 1 && !stop; ++y)
                    for (int x = -1; x <= 1 && !stop; ++x) {
                        if (candidates >= rounds * 72 || update_budget - updates < cap ||
                            Clock::now() >= deadline) {
                            reason = "search-budget";
                            stop = true;
                            break;
                        }
                        const unsigned direction = (x < 0   ? 64
                                                    : x > 0 ? 128
                                                            : 0) |
                                                   (y < 0   ? 16
                                                    : y > 0 ? 32
                                                            : 0);
                        const unsigned action =
                            (current.actions.at(branch - 1) & ~0xf0u) | direction;
                        const fs::path stem =
                            directory / ("r" + std::to_string(round) + "k" +
                                         std::to_string(rollback) + "a" + std::to_string(action));
                        // branch is the first replaced one-based action. Native latch
                        // semantics are unchanged; this does not move the player early.
                        {
                            std::ofstream prefix(stem.string() + ".prefix");
                            for (unsigned i = 0; i < branch - 1; ++i)
                                prefix << current.actions[i] << '\n';
                            for (unsigned i = 0; i < segment; ++i)
                                prefix << action << '\n';
                            if (!prefix)
                                throw std::runtime_error("cannot write prefix");
                        }
                        args = arguments(stem, true);
                        args.insert(args.end(), {"--strategy", "spell-portfolio", "--resume-prefix",
                                                 stem.string() + ".prefix", "--prefix-frame",
                                                 std::to_string(branch - 1)});
                        ++candidates;
                        const auto process = audit->execute(1, stem, cap, args, deadline);
                        if (process.interrupted) {
                            uncertain_updates_bound = cap;
                            reason = "interrupted-search-budget";
                            stop = true;
                            break;
                        }
                        auto candidate = record(stem, process.code, cap, true);
                        for (const char *key :
                             {"source_revision", "profile", "dat_sha256", "stage",
                              "requested_spell_id", "difficulty", "seed", "frame_budget"})
                            if (field(candidate.report, key) != field(current.report, key))
                                throw std::runtime_error(
                                    "candidate scene/profile identity mismatch");
                        if (integer(field(candidate.report, "prefix_frame"), cap) != branch - 1)
                            throw std::runtime_error("candidate prefix boundary mismatch");
                        for (unsigned i = branch - 1;
                             i < std::min(candidate.frames, branch - 1 + segment); ++i)
                            if (candidate.actions.at(i) != action)
                                throw std::runtime_error("candidate forced hold mismatch");
                        updates += candidate.frames;
                        prefix_updates += std::min(branch - 1, candidate.frames);
                        if (field(candidate.report, "prefix_trace_digest") !=
                            current.observations.at(branch - 1).digest)
                            throw std::runtime_error(
                                "candidate prefix differs from selected source run");
                        for (unsigned i = 0; i < branch - 1; ++i)
                            if (candidate.actions.at(i) != current.actions[i])
                                throw std::runtime_error("candidate prefix action mismatch");
                        audit->validated(candidate.frames);
                        ledger << round << '\t' << trigger.onset << '\t' << rollback << '\t'
                               << branch << '\t' << action << '\t' << candidate.frames << '\t'
                               << candidate.outcome << '\t' << process.wall_ms << '\t'
                               << process.rss_kib << '\t'
                               << field(candidate.report, "prefix_trace_digest") << '\n';
                        ledger.flush();
                        if (!ledger)
                            throw std::runtime_error("cannot write search ledger");
                        if (candidate.outcome == "\"complete\"") {
                            current = std::move(candidate);
                            reason = "complete";
                            stop = true;
                            break;
                        }
                        if (candidate.outcome == "\"collision\"" && candidate.frames > best.frames)
                            best = std::move(candidate);
                    }
            }
            // Budget interruption does not erase an earlier fully validated
            // improvement. Never promote the interrupted child, and retain the
            // first candidate on equal survival. A complete winner already owns
            // current and must not be overwritten by an earlier failed best.
            if (reason != "complete" && best.frames > current.frames)
                current = best;
            if (!stop) {
                if (best.stem == source_stem) {
                    reason = "no-progress-finite-family";
                    break;
                }
            }
        }
        const double search_ms =
            std::chrono::duration<double, std::milli>(Clock::now() - search_start).count();
        if (reason == "complete") {
            const fs::path replay = directory / "verified-replay";
            args = arguments(replay, false);
            args.insert(args.end(), {"--replay", current.stem.string() + ".actions"});
            const auto process =
                audit->execute(2, replay, cap, args, Clock::now() + std::chrono::seconds(seconds));
            replay_ms = process.wall_ms;
            if (process.interrupted)
                throw std::runtime_error(
                    "verification replay interrupted; completion not certified");
            const auto verified = record(replay, process.code, cap, false);
            replay_updates = verified.frames;
            agree(current, verified);
            audit->validated(verified.frames);
        }
        audit->write(reason);
        const double total_ms =
            std::chrono::duration<double, std::milli>(Clock::now() - overall_start).count();
        std::ofstream output(directory / "summary.json");
        // Keep phase costs distinct; embed the selected native report.
        output << "{\"producer\":\"th08_headless_repair\",\"outcome\":\"" << reason
               << "\",\"prior_verified_native_updates\":" << (resumed ? resumed->known : 0)
               << ",\"prior_unverified_update_upper_bound\":" << (resumed ? resumed->uncertain : 0)
               << ",\"prior_wall_ms\":" << (resumed ? resumed->wall_ms : 0)
               << ",\"cumulative_wall_ms\":" << total_ms + (resumed ? resumed->wall_ms : 0)
               << ",\"initial_kind\":\"" << (resumed ? "fresh-prefix-regeneration" : "fresh-policy")
               << "\""
               << ",\"prior_manifest_sha256\":\""
               << (resumed ? file_hash(directory / "prior" / "manifest.tsv") : "") << "\""
               << ",\"cumulative_verified_native_updates\":"
               << checked_add(
                      resumed ? resumed->known : 0,
                      checked_add(audit->known[0], checked_add(audit->known[1], audit->known[2])))
               << ",\"cumulative_unverified_update_upper_bound\":"
               << checked_add(resumed ? resumed->uncertain : 0,
                              checked_add(audit->uncertain[0],
                                          checked_add(audit->uncertain[1], audit->uncertain[2])))
               << ",\"candidate_limit\":" << rounds * 72 << ",\"child_executable_sha256\":\""
               << audit->native_hash << "\",\"repair_executable_sha256\":\"" << audit->repair_hash
               << "\""
               << ",\"candidate_update_budget\":" << update_budget
               << ",\"search_seconds\":" << seconds << ",\"candidates\":" << candidates
               << ",\"candidate_native_updates\":" << updates
               << ",\"interrupted_update_upper_bound\":" << uncertain_updates_bound
               << ",\"replayed_prefix_updates\":" << prefix_updates
               << ",\"initial_native_updates\":"
               << field(read(initial.string() + ".json"), "frames")
               << ",\"initial_wall_ms\":" << initial_process.wall_ms
               << ",\"search_wall_ms\":" << search_ms
               << ",\"verification_native_updates\":" << replay_updates
               << ",\"verification_wall_ms\":" << replay_ms << ",\"total_wall_ms\":" << total_ms
               << ",\"selected_case\":\"" << current.stem.filename().string()
               << "\",\"execution\":" << current.report << "}\n";
        if (!output)
            throw std::runtime_error("cannot write repair summary");
        std::cout << reason << ": " << candidates << " candidates, " << updates
                  << " native candidate updates\n";
        return reason == "complete" ? 0 : 2;
    } catch (const std::exception &error) {
        if (audit) {
            try {
                audit->write("protocol-or-process-error", true);
            } catch (...) {
            }
        }
        std::cerr << "th08_headless_repair: " << error.what() << '\n';
        return 1;
    }
}
