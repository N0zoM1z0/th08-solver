// Branch an explicit native checkpoint using realizable held inputs. This is a
// bounded observation experiment: sampled agreement does not establish the time
// at which every possible future input ceases to affect an event.
#include "th08/hazard_probe.hpp"
#include "modern/headless/runtime.hpp"
#include "modern/headless/session.hpp"

#include <openssl/evp.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <new>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>

namespace {
namespace fs = std::filesystem;
namespace hp = th08::hazard_probe;
namespace native = th08::headless;
using Clock = std::chrono::steady_clock;
constexpr unsigned kMaxSpan = 64, kMaxLeads = 64, kMaxLaserCalls = 4096;
constexpr std::uint64_t kMaximumUpdate = 1000000;
constexpr std::uint16_t kSupportedInputs = 0x10f7;

struct Options {
    fs::path dat, prefix, output;
    native::Config config;
    std::uint64_t event = 0, max_updates = 500000;
    unsigned span = 1, seconds = 120;
    std::uint16_t baseline_input = 4;
    std::vector<unsigned> leads{160, 120, 80, 40, 20, 10};
};

std::uint64_t number(const std::string &text, std::uint64_t maximum) {
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (text.empty() || parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() ||
        result > maximum)
        throw std::runtime_error("invalid numeric option");
    return result;
}

int stage_index(const std::string &name) {
    const std::array<std::string, 9> names{"1", "2", "3", "4a", "4b", "5", "6a", "6b", "extra"};
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end())
        throw std::runtime_error("invalid stage name");
    return static_cast<int>(found - names.begin());
}

Options parse_options(int argc, char **argv) {
    Options options;
    bool selected_scene = false, selected_stage = false;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc)
            throw std::runtime_error("each option requires a value");
        const std::string key = argv[i], value = argv[i + 1];
        if (key == "--dat")
            options.dat = fs::absolute(value);
        else if (key == "--output")
            options.output = fs::absolute(value);
        else if (key == "--prefix")
            options.prefix = fs::absolute(value);
        else if (key == "--spell" || key == "--spell-id") {
            options.config.spell = static_cast<int>(number(value, 221));
            selected_scene = true;
        } else if (key == "--stage") {
            options.config.stage = stage_index(value);
            selected_scene = selected_stage = true;
        } else if (key == "--story") {
            options.config.story = number(value, 1);
            selected_scene = true;
        } else if (key == "--difficulty")
            options.config.difficulty = static_cast<int>(number(value, 4));
        else if (key == "--seed")
            options.config.seed = static_cast<std::uint16_t>(number(value, 65535));
        else if (key == "--shot")
            options.config.shot = static_cast<int>(number(value, 11));
        else if (key == "--event-update")
            options.event = number(value, kMaximumUpdate);
        else if (key == "--span" || key == "--lookahead")
            options.span = static_cast<unsigned>(number(value, kMaxSpan));
        else if (key == "--max-native-updates")
            options.max_updates = number(value, 1000000000);
        else if (key == "--seconds")
            options.seconds = static_cast<unsigned>(number(value, 3600));
        else if (key == "--baseline-input")
            options.baseline_input = static_cast<std::uint16_t>(number(value, kSupportedInputs));
        else if (key == "--leads") {
            options.leads.clear();
            std::istringstream input(value);
            std::string token;
            while (std::getline(input, token, ',')) {
                options.leads.push_back(static_cast<unsigned>(number(token, kMaximumUpdate)));
                if (options.leads.size() > kMaxLeads)
                    throw std::runtime_error("lead count exceeds bound");
            }
            if (value.empty() || value.back() == ',')
                throw std::runtime_error("empty lead value");
        } else
            throw std::runtime_error("unknown option");
    }
    if (options.config.spell >= 0 && !selected_stage)
        throw std::runtime_error("spell checkpoints require an explicit --stage");
    if (options.dat.empty() || options.output.empty() || !selected_scene || !options.event ||
        !options.span || !options.seconds || !options.max_updates || options.leads.empty() ||
        (options.baseline_input & ~kSupportedInputs) ||
        options.event + options.span - 1 > kMaximumUpdate ||
        (options.config.story && options.config.spell >= 0))
        throw std::runtime_error("invalid or missing checkpoint/window/budget option");
    for (const auto lead : options.leads)
        if (!lead || lead > options.event)
            throw std::runtime_error("each lead must be positive and at most event-update");
    std::sort(options.leads.begin(), options.leads.end(), std::greater<unsigned>());
    if (std::adjacent_find(options.leads.begin(), options.leads.end()) != options.leads.end())
        throw std::runtime_error("duplicate lead value");
    return options;
}

std::string file_hash(const fs::path &path) {
    std::ifstream input(path, std::ios::binary);
    auto context =
        std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!input || !context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("cannot hash private input");
    std::array<char, 65536> buffer;
    while (input) {
        input.read(buffer.data(), buffer.size());
        if (EVP_DigestUpdate(context.get(), buffer.data(),
                             static_cast<std::size_t>(input.gcount())) != 1)
            throw std::runtime_error("private input hash failed");
    }
    if (!input.eof())
        throw std::runtime_error("private input read failed");
    std::array<unsigned char, EVP_MAX_MD_SIZE> hash;
    unsigned length = 0;
    if (EVP_DigestFinal_ex(context.get(), hash.data(), &length) != 1)
        throw std::runtime_error("private input hash failed");
    std::ostringstream result;
    for (unsigned i = 0; i < length; ++i)
        result << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(hash[i]);
    return result.str();
}

std::vector<std::uint16_t> read_prefix(const fs::path &path) {
    if (path.empty())
        return {};
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open action prefix");
    std::vector<std::uint16_t> result;
    std::string token;
    while (input >> token) {
        if (result.size() == kMaximumUpdate)
            throw std::runtime_error("action prefix exceeds update bound");
        const auto action = number(token, 0xffff);
        if (action & ~kSupportedInputs)
            throw std::runtime_error("unsupported action prefix input");
        result.push_back(static_cast<std::uint16_t>(action));
    }
    if (!input.eof() || result.empty())
        throw std::runtime_error("invalid or empty action prefix");
    return result;
}

class RunDirectory {
  public:
    RunDirectory() : previous_(fs::current_path()) {
        char path[] = "/tmp/th08-hazard-probe-XXXXXX";
        if (!mkdtemp(path))
            throw std::runtime_error("cannot create isolated runtime directory");
        path_ = path;
        fs::current_path(path_);
    }
    ~RunDirectory() {
        std::error_code ignored;
        fs::current_path(previous_, ignored);
        fs::remove_all(path_, ignored);
    }

  private:
    fs::path previous_, path_;
};

struct Variant {
    const char *name;
    std::uint16_t input;
    bool baseline = false;
};
constexpr std::array<Variant, 12> kVariants{{
    {"baseline-continuation", 0, true},
    {"quiet-focused-still", 0x004},
    {"shoot-focused-still", 0x005},
    {"quiet-unfocused-still", 0x000},
    {"shoot-unfocused-still", 0x001},
    {"quiet-focused-left", 0x044},
    {"quiet-focused-right", 0x084},
    {"shoot-focused-left", 0x045},
    {"shoot-focused-right", 0x085},
    {"quiet-unfocused-left", 0x040},
    {"quiet-unfocused-right", 0x080},
    {"quiet-focused-up", 0x014},
}};

enum class Status : std::uint32_t {
    Complete,
    Death,
    Terminal,
    Budget,
    RuntimeError,
    Capacity,
    CheckpointMismatch,
    WorkerError,
    NotRun
};

const char *status_name(Status status) {
    switch (status) {
    case Status::Complete:
        return "complete";
    case Status::Death:
        return "death";
    case Status::Terminal:
        return "terminal-before-window-end";
    case Status::Budget:
        return "budget-exhausted";
    case Status::RuntimeError:
        return "native-runtime-error";
    case Status::Capacity:
        return "observation-capacity-exceeded";
    case Status::CheckpointMismatch:
        return "checkpoint-identity-mismatch";
    case Status::WorkerError:
        return "worker-protocol-or-exit-error";
    case Status::NotRun:
        return "checkpoint-not-reached";
    }
    return "invalid";
}

struct Progress {
    // Each fork owns one mapping slot. The parent reads only after waitpid, so
    // a killed worker retains the counts of both completed and started calls.
    std::uint64_t attempted = 0, completed = 0, step_ns = 0, io_ns = 0;
    native::State last;
    native::CollisionEvent collision;
};

struct Sample {
    hp::Frame geometry;
    native::State state;
    native::CollisionEvent collision;
    std::uint64_t actor_digest = 0, bullet_digest = 0, region_digest = 0;
    std::uint32_t bullets = 0, regions = 0;
};

struct Result {
    Status status = Status::NotRun;
    Progress progress;
    std::vector<Sample> samples;
    int exit_code = -1, signal = 0;
    bool timed_out = false;
};

struct WireHeader {
    std::uint32_t magic = 0x48505231, status = 0, frames = 0;
};
struct WireSample {
    native::State state;
    native::CollisionEvent collision;
    std::uint64_t actor_digest = 0, bullet_digest = 0, region_digest = 0;
    std::uint32_t bullets = 0, regions = 0, lasers = 0;
};
static_assert(std::is_trivially_copyable<WireSample>::value);
static_assert(std::is_trivially_copyable<hp::Laser>::value);

std::uint16_t baseline_action(const Options &options, const std::vector<std::uint16_t> &prefix,
                              std::uint64_t completed_updates) {
    return completed_updates < prefix.size() ? prefix[completed_updates] : options.baseline_input;
}

native::State step(native::Session &session, std::uint16_t input, Progress &progress) {
    const auto before = Clock::now();
    const auto io_before = session.file_io_time_ns();
    ++progress.attempted;
    const auto state = session.step(input);
    ++progress.completed;
    if (!std::isfinite(state.x) || !std::isfinite(state.y) || !std::isfinite(state.deaths))
        throw std::runtime_error("non-finite native player state");
    progress.last = state;
    progress.collision = session.collision();
    progress.step_ns +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - before).count();
    progress.io_ns += session.file_io_time_ns() - io_before;
    return state;
}

bool died(const native::State &state, float checkpoint_deaths) {
    return state.player_state == 2 || state.deaths > checkpoint_deaths;
}

bool terminal(const Options &options, const native::State &state, bool seen_spell) {
    return state.retry_menu ||
           (!options.config.story &&
            (options.config.spell >= 0 ? seen_spell && !state.spell_active : state.stage_complete));
}

void hash_word(std::uint64_t &digest, std::uint32_t value) {
    digest = (digest ^ value) * 1099511628211ULL;
}

Sample capture(native::Session &session, const native::State &state) {
    Sample sample;
    sample.state = state;
    sample.collision = session.collision();
    sample.actor_digest = session.actor_digest();
    sample.geometry.update = state.frame;
    const auto &lasers = session.laser_hitboxes();
    if (lasers.size() > kMaxLaserCalls)
        throw std::length_error("laser observation capacity exceeded");
    sample.geometry.lasers.reserve(lasers.size());
    for (const auto &laser : lasers) {
        hp::Laser copied{laser.center_x,    laser.center_y,    laser.full_width,
                         laser.full_height, laser.origin_x,    laser.origin_y,
                         laser.angle,       laser.pooled_slot, laser.graze_enabled};
        hp::laser_key(copied); // Stop this case on non-finite geometry.
        sample.geometry.lasers.push_back(copied);
    }
    sample.bullet_digest = sample.region_digest = 1469598103934665603ULL;
    const auto &bullets = native::current_bullet_boxes();
    sample.bullets = static_cast<std::uint32_t>(bullets.size());
    for (const auto &bullet : bullets)
        for (const auto value : {bullet.x, bullet.y, bullet.w, bullet.h})
            hash_word(sample.bullet_digest, hp::float_bits(value));
    const auto &regions = session.lethal_regions();
    sample.regions = static_cast<std::uint32_t>(regions.size());
    for (const auto &region : regions)
        for (const auto value :
             {region.bounds.left, region.bounds.top, region.bounds.right, region.bounds.bottom})
            hash_word(sample.region_digest, hp::float_bits(value));
    return sample;
}

bool write_bytes(int descriptor, const void *data, std::size_t size) {
    const auto *next = static_cast<const char *>(data);
    while (size) {
        const auto wrote = write(descriptor, next, size);
        if (wrote < 0 && errno == EINTR)
            continue;
        if (wrote <= 0)
            return false;
        next += wrote;
        size -= static_cast<std::size_t>(wrote);
    }
    return true;
}

bool send_result(int descriptor, const Result &result) {
    const WireHeader header{0x48505231, static_cast<std::uint32_t>(result.status),
                            static_cast<std::uint32_t>(result.samples.size())};
    if (!write_bytes(descriptor, &header, sizeof(header)))
        return false;
    for (const auto &sample : result.samples) {
        const WireSample wire{
            sample.state,         sample.collision,
            sample.actor_digest,  sample.bullet_digest,
            sample.region_digest, sample.bullets,
            sample.regions,       static_cast<std::uint32_t>(sample.geometry.lasers.size())};
        if (!write_bytes(descriptor, &wire, sizeof(wire)) ||
            !write_bytes(descriptor, sample.geometry.lasers.data(),
                         sample.geometry.lasers.size() * sizeof(hp::Laser)))
            return false;
    }
    return true;
}

Result run_branch(native::Session &session, const Options &options,
                  const std::vector<std::uint16_t> &prefix, const Variant &variant,
                  Progress &progress, std::uint64_t update_budget, Clock::time_point deadline,
                  bool seen_spell) {
    Result result;
    result.status = Status::Complete;
    auto state = session.state();
    const auto initial_deaths = state.deaths;
    native::record_bullet_boxes = true;
    try {
        while (state.frame < options.event + options.span - 1) {
            if (progress.attempted == update_budget || Clock::now() >= deadline) {
                result.status = Status::Budget;
                break;
            }
            const auto action =
                variant.baseline ? baseline_action(options, prefix, state.frame) : variant.input;
            state = step(session, action, progress);
            seen_spell = seen_spell || state.spell_active;
            if (state.frame >= options.event) {
                if (options.config.spell >= 0 &&
                    (!state.spell_active || state.spell != options.config.spell)) {
                    result.status = Status::CheckpointMismatch;
                    break;
                }
                result.samples.push_back(capture(session, state));
            }
            if (died(state, initial_deaths)) {
                result.status = Status::Death;
                break;
            }
            if (terminal(options, state, seen_spell)) {
                if (state.frame < options.event + options.span - 1)
                    result.status = Status::Terminal;
                break;
            }
        }
    } catch (const std::length_error &) {
        result.status = Status::Capacity;
    } catch (const std::exception &) {
        result.status = Status::RuntimeError;
    }
    return result;
}

template <class T> T take(const std::vector<char> &bytes, std::size_t &offset) {
    if (offset > bytes.size() || sizeof(T) > bytes.size() - offset)
        throw std::runtime_error("truncated worker result");
    T result;
    std::memcpy(&result, bytes.data() + offset, sizeof(result));
    offset += sizeof(result);
    return result;
}

Result decode_result(const std::vector<char> &bytes, const Options &options) {
    Result result;
    std::size_t offset = 0;
    const auto header = take<WireHeader>(bytes, offset);
    if (header.magic != 0x48505231 ||
        header.status > static_cast<unsigned>(Status::CheckpointMismatch) ||
        header.frames > options.span)
        throw std::runtime_error("invalid worker result header");
    result.status = static_cast<Status>(header.status);
    for (unsigned i = 0; i < header.frames; ++i) {
        const auto wire = take<WireSample>(bytes, offset);
        if (wire.lasers > kMaxLaserCalls || wire.state.frame != options.event + i)
            throw std::runtime_error("invalid worker observation boundary");
        Sample sample;
        sample.state = wire.state;
        sample.collision = wire.collision;
        sample.actor_digest = wire.actor_digest;
        sample.bullet_digest = wire.bullet_digest;
        sample.region_digest = wire.region_digest;
        sample.bullets = wire.bullets;
        sample.regions = wire.regions;
        sample.geometry.update = wire.state.frame;
        for (unsigned j = 0; j < wire.lasers; ++j)
            sample.geometry.lasers.push_back(take<hp::Laser>(bytes, offset));
        result.samples.push_back(std::move(sample));
    }
    if (offset != bytes.size() ||
        (result.status == Status::Complete && result.samples.size() != options.span))
        throw std::runtime_error("incomplete or trailing worker result");
    return result;
}

Result fork_branch(native::Session &session, const Options &options,
                   const std::vector<std::uint16_t> &prefix, const Variant &variant,
                   Progress &progress, std::uint64_t update_budget, Clock::time_point deadline,
                   bool seen_spell) {
    Result result;
    result.status = Status::WorkerError;
    progress.last = session.state();
    int descriptors[2];
    if (pipe(descriptors))
        return result;
    // One pipe and one mmap slot per branch; all children are reaped before the
    // next fork. PbgArchive also reopens the DAT for every resource entry read.
    std::cout.flush();
    std::cerr.flush();
    const auto child = fork();
    if (child == 0) {
        close(descriptors[0]);
        const auto branch = run_branch(session, options, prefix, variant, progress, update_budget,
                                       deadline, seen_spell);
        const auto sent = send_result(descriptors[1], branch);
        close(descriptors[1]);
        _exit(sent ? 0 : 2);
    }
    close(descriptors[1]);
    if (child < 0) {
        close(descriptors[0]);
        return result;
    }
    std::vector<char> bytes;
    const auto maximum = sizeof(WireHeader) +
                         options.span * (sizeof(WireSample) + kMaxLaserCalls * sizeof(hp::Laser));
    bool failed = false, timed_out = false;
    for (;;) {
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
        if (remaining <= 0) {
            timed_out = true;
            break;
        }
        pollfd descriptor{descriptors[0], POLLIN, 0};
        const auto ready =
            poll(&descriptor, 1, static_cast<int>(std::min<std::int64_t>(remaining, 1000)));
        if (ready < 0 && errno == EINTR)
            continue;
        if (ready < 0 || (descriptor.revents & (POLLERR | POLLNVAL))) {
            failed = true;
            break;
        }
        if (!ready)
            continue;
        std::array<char, 65536> buffer;
        const auto count = read(descriptors[0], buffer.data(), buffer.size());
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0) {
            failed = true;
            break;
        }
        if (!count)
            break;
        if (static_cast<std::size_t>(count) > maximum - bytes.size()) {
            failed = true;
            break;
        }
        bytes.insert(bytes.end(), buffer.data(), buffer.data() + count);
    }
    close(descriptors[0]);
    if (failed || timed_out)
        kill(child, SIGKILL);
    int status = 0;
    pid_t reaped;
    do {
        reaped = waitpid(child, &status, 0);
    } while (reaped < 0 && errno == EINTR);
    const auto exited = reaped == child && WIFEXITED(status) && WEXITSTATUS(status) == 0;
    if (exited && !failed && !timed_out) {
        try {
            result = decode_result(bytes, options);
        } catch (const std::exception &) {
            result.status = Status::WorkerError;
        }
    }
    result.progress = progress;
    result.timed_out = timed_out;
    if (timed_out)
        result.status = Status::Budget;
    result.exit_code = reaped == child && WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    result.signal = reaped == child && WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    return result;
}

std::vector<hp::Frame> geometry(const Result &result) {
    std::vector<hp::Frame> frames;
    for (const auto &sample : result.samples)
        frames.push_back(sample.geometry);
    return frames;
}

bool comparable(const Result &result, const Options &options) {
    return (result.status == Status::Complete || result.status == Status::Death) &&
           result.samples.size() == options.span;
}

void write_state(std::ostream &out, const native::State &state) {
    out << "{\"update\":" << state.frame << ",\"stage\":" << state.stage
        << ",\"spell\":" << state.spell << ",\"spell_active\":" << state.spell_active
        << ",\"player_state\":" << state.player_state << ",\"youkai\":" << state.youkai
        << ",\"x\":" << state.x << ",\"y\":" << state.y << ",\"deaths\":" << state.deaths
        << ",\"rng_seed\":" << state.rng_seed << ",\"rng_draws\":" << state.rng_draws
        << ",\"latched_input\":" << state.latched_input
        << ",\"sampled_input\":" << state.sampled_input << ",\"graze\":" << state.graze
        << ",\"score\":" << state.score << ",\"gauge\":" << state.gauge << '}';
}

void write_result(std::ostream &out, const Result &result, const Variant &variant,
                  const Result &baseline, const Options &options) {
    out << "{\"variant\":\"" << variant.name << "\",\"held_input\":";
    if (variant.baseline)
        out << "null";
    else
        out << variant.input;
    out << ",\"status\":\"" << status_name(result.status)
        << "\",\"completed_native_updates\":" << result.progress.completed
        << ",\"attempted_native_updates\":" << result.progress.attempted
        << ",\"simulation_seconds_excluding_file_io\":"
        << static_cast<double>(result.progress.step_ns -
                               std::min(result.progress.step_ns, result.progress.io_ns)) /
               1e9
        << ",\"exit_code\":" << result.exit_code << ",\"signal\":" << result.signal
        << ",\"timed_out\":" << result.timed_out << ",\"same_geometry_as_baseline\":";
    if (!comparable(result, options) || !comparable(baseline, options))
        out << "null";
    else
        out << hp::same_window(geometry(result), geometry(baseline), options.event, options.span);
    out << ",\"last_state\":";
    write_state(out, result.progress.last);
    out << ",\"collision\":{\"kind\":" << static_cast<int>(result.progress.collision.kind)
        << ",\"update\":" << result.progress.collision.frame
        << ",\"laser_slot\":" << result.progress.collision.laser_slot
        << ",\"laser_call\":" << result.progress.collision.laser_hitbox_call << "},\"samples\":[";
    for (std::size_t i = 0; i < result.samples.size(); ++i) {
        if (i)
            out << ',';
        const auto &sample = result.samples[i];
        out << "{\"state\":";
        write_state(out, sample.state);
        out << ",\"actor_digest\":\"" << sample.actor_digest << '"'
            << ",\"bullet_box_calls\":" << sample.bullets << ",\"bullet_box_ordered_digest\":\""
            << sample.bullet_digest << '"' << ",\"lethal_region_calls\":" << sample.regions
            << ",\"lethal_region_ordered_digest\":\"" << sample.region_digest << '"'
            << ",\"laser_calls\":[";
        for (std::size_t j = 0; j < sample.geometry.lasers.size(); ++j) {
            if (j)
                out << ',';
            const auto &laser = sample.geometry.lasers[j];
            out << "{\"center\":[" << laser.center_x << ',' << laser.center_y << "],\"size\":["
                << laser.full_width << ',' << laser.full_height << "],\"origin\":["
                << laser.origin_x << ',' << laser.origin_y << "],\"angle\":" << laser.angle
                << ",\"pooled_slot\":" << laser.pooled_slot << ",\"source\":\""
                << (laser.pooled_slot < 0 ? "non-pooled-call" : "pooled-laser")
                << "\",\"graze_enabled\":" << laser.graze_enabled << '}';
        }
        out << "]}";
    }
    out << "]}";
}

int run(Options options) {
    const auto dat_identity = file_hash(options.dat);
    const auto binary_identity = file_hash("/proc/self/exe");
    const auto prefix_identity = options.prefix.empty() ? std::string{} : file_hash(options.prefix);
    const auto prefix = read_prefix(options.prefix);
    std::ofstream out(options.output);
    if (!out)
        throw std::runtime_error("cannot open probe output");
    out << std::boolalpha << std::setprecision(9);
    const auto slot_count = options.leads.size() * kVariants.size();
    const auto mapping_size = sizeof(Progress) * slot_count;
    void *mapping =
        mmap(nullptr, mapping_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED)
        throw std::runtime_error("cannot allocate probe progress slots");
    auto *slots = static_cast<Progress *>(mapping);
    for (std::size_t i = 0; i < slot_count; ++i)
        new (&slots[i]) Progress{};
    const auto dat_path = options.dat.string();
    options.config.dat_path = dat_path.c_str();
    RunDirectory directory;
    const auto began = Clock::now();
    const auto deadline = began + std::chrono::seconds(options.seconds);
    native::Session session(options.config);
    Progress parent;
    parent.last = session.state();
    bool seen_spell = parent.last.spell_active;
    Status prefix_status = Status::Complete;
    bool hard_failure = false, budget_failure = false;
    unsigned nonempty_windows = 0;
    std::uint64_t child_attempted = 0, child_completed = 0, child_step_ns = 0, child_io_ns = 0;
    out << "{\"schema\":\"native-hazard-stability-v1\",\"evidence\":\"source-faithful-native-"
           "profile\","
        << "\"comparison_scope\":\"Complete sampled laser-call multisets, exact float bits, source "
           "slots and graze flags. "
        << "Death on the final sampled update retains that update. Bullet/region digests are "
           "diagnostics only.\","
        << "\"limitation\":\"Agreement covers these sampled inputs only; it proves neither a "
           "universal lock-in time "
        << "nor quiet-input RNG freezing.\",\"source_revision\":\""
        << "02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415\",\"binary_sha256\":\"" << binary_identity
        << "\",\"dat_sha256\":\"" << dat_identity << "\",\"prefix_sha256\":\"" << prefix_identity
        << "\",\"prefix_actions\":" << prefix.size() << ",\"stage\":" << options.config.stage
        << ",\"spell\":" << options.config.spell << ",\"story\":" << options.config.story
        << ",\"difficulty\":" << options.config.difficulty << ",\"seed\":" << options.config.seed
        << ",\"shot\":" << options.config.shot << ",\"event_update\":" << options.event
        << ",\"span\":" << options.span
        << ",\"baseline_input_after_prefix\":" << options.baseline_input
        << ",\"budget\":{\"native_update_calls\":" << options.max_updates
        << ",\"wall_seconds\":" << options.seconds
        << ",\"maximum_laser_calls_per_update\":" << kMaxLaserCalls
        << "},\"environment\":{\"platform\":\"linux-native\",\"compiler\":\"" << __VERSION__
        << "\",\"native_optimization\":" << TH08_HEADLESS_OPTIMIZATION
        << ",\"fp_contract\":\"off\"},\"leads\":[\n";
    for (std::size_t lead_index = 0; lead_index < options.leads.size(); ++lead_index) {
        const auto lead = options.leads[lead_index];
        const auto checkpoint = options.event - lead;
        while (prefix_status == Status::Complete && parent.last.frame < checkpoint) {
            if (parent.attempted + child_attempted == options.max_updates ||
                Clock::now() >= deadline) {
                prefix_status = Status::Budget;
                break;
            }
            try {
                const auto state =
                    step(session, baseline_action(options, prefix, parent.last.frame), parent);
                seen_spell = seen_spell || state.spell_active;
                if (died(state, 0))
                    prefix_status = Status::Death;
                else if (terminal(options, state, seen_spell))
                    prefix_status = Status::Terminal;
            } catch (const std::exception &) {
                prefix_status = Status::RuntimeError;
            }
        }
        std::array<Result, kVariants.size()> results;
        const bool reached = prefix_status == Status::Complete && parent.last.frame == checkpoint;
        for (std::size_t variant_index = 0; variant_index < kVariants.size(); ++variant_index) {
            auto &result = results[variant_index];
            result.progress.last = parent.last;
            if (reached) {
                const auto used = parent.attempted + child_attempted;
                if (used >= options.max_updates || Clock::now() >= deadline)
                    result.status = Status::Budget;
                else
                    result = fork_branch(session, options, prefix, kVariants[variant_index],
                                         slots[lead_index * kVariants.size() + variant_index],
                                         options.max_updates - used, deadline, seen_spell);
            }
            child_attempted += result.progress.attempted;
            child_completed += result.progress.completed;
            child_step_ns += result.progress.step_ns;
            child_io_ns += result.progress.io_ns;
            hard_failure = hard_failure || result.status == Status::RuntimeError ||
                           result.status == Status::Capacity ||
                           result.status == Status::CheckpointMismatch ||
                           result.status == Status::WorkerError;
            budget_failure = budget_failure || result.status == Status::Budget;
        }
        std::vector<std::vector<hp::Frame>> groups;
        unsigned observed = 0, nonempty = 0, baseline_same = 0;
        for (const auto &result : results) {
            if (!comparable(result, options))
                continue;
            ++observed;
            const auto window = geometry(result);
            if (std::any_of(window.begin(), window.end(),
                            [](const auto &frame) { return !frame.lasers.empty(); }))
                ++nonempty;
            if (comparable(results.front(), options) &&
                hp::same_window(window, geometry(results.front()), options.event, options.span))
                ++baseline_same;
            if (std::none_of(groups.begin(), groups.end(), [&](const auto &group) {
                    return hp::same_window(window, group, options.event, options.span);
                }))
                groups.push_back(window);
        }
        nonempty_windows += nonempty;
        if (lead_index)
            out << ",\n";
        out << "{\"lead_updates\":" << lead << ",\"checkpoint_update\":" << checkpoint
            << ",\"checkpoint_reached\":" << reached << ",\"checkpoint_state\":";
        write_state(out, parent.last);
        out << ",\"observed_complete_windows\":" << observed
            << ",\"windows_with_lasers\":" << nonempty << ",\"geometry_groups\":" << groups.size()
            << ",\"same_as_baseline_windows\":" << baseline_same
            << ",\"all_sampled_variants_agree\":"
            << (observed == kVariants.size() && nonempty == kVariants.size() && groups.size() == 1)
            << ",\"variants\":[";
        for (std::size_t i = 0; i < results.size(); ++i) {
            if (i)
                out << ',';
            write_result(out, results[i], kVariants[i], results.front(), options);
        }
        out << "]}";
        out.flush();
    }
    hard_failure = hard_failure || prefix_status == Status::RuntimeError;
    budget_failure = budget_failure || prefix_status == Status::Budget;
    const auto total_step_ns = parent.step_ns + child_step_ns;
    const auto total_io_ns = parent.io_ns + child_io_ns;
    const bool prefix_incomplete = prefix_status != Status::Complete;
    const auto overall = hard_failure        ? "error"
                         : budget_failure    ? "budget-exhausted"
                         : prefix_incomplete ? "prefix-incomplete"
                         : !nonempty_windows ? "event-not-observed"
                                             : "observed";
    out << "\n],\"status\":\"" << overall << "\",\"prefix_status\":\"" << status_name(prefix_status)
        << "\",\"completed_native_updates\":" << parent.completed + child_completed
        << ",\"attempted_native_updates\":" << parent.attempted + child_attempted
        << ",\"prefix_completed_updates\":" << parent.completed
        << ",\"simulation_seconds_excluding_file_io\":"
        << static_cast<double>(total_step_ns - std::min(total_step_ns, total_io_ns)) / 1e9
        << ",\"wall_seconds\":" << std::chrono::duration<double>(Clock::now() - began).count()
        << "}\n";
    out.close();
    munmap(mapping, mapping_size);
    if (!out)
        throw std::runtime_error("cannot finish probe output");
    std::cout << "hazard probe " << overall << ": " << parent.completed + child_completed
              << " completed native updates\n";
    return hard_failure ? 2 : budget_failure ? 3 : prefix_incomplete || !nonempty_windows ? 4 : 0;
}
} // namespace

int main(int argc, char **argv) {
    try {
        return run(parse_options(argc, argv));
    } catch (const std::exception &error) {
        std::cerr << "hazard probe: " << error.what() << '\n';
        return 2;
    }
}
