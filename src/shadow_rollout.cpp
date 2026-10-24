#include "th08/shadow_rollout.hpp"
#include "modern/headless/session.hpp"
#include "th08/shadow_native.hpp"
#include <cerrno>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <new>
#include <poll.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace th08::shadow {
namespace {
using Clock = std::chrono::steady_clock;
constexpr std::size_t maximum_horizon = 2048;
struct Shared {
    BranchStatus status = BranchStatus::Incomplete;
    std::uint32_t frames = 0;
    std::uint64_t attempts = 0, completed = 0, native_ns = 0, file_io_ns = 0;
    std::array<Packet, maximum_horizon> records;
};
class Arena {
  public:
    Shared *data;
    Arena() {
        void *storage = mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (storage == MAP_FAILED)
            throw std::runtime_error("shadow_rollout_allocation_failed");
        data = new (storage) Shared{};
    }
    ~Arena() {
        munmap(data, sizeof(Shared));
    }
    Arena(const Arena &) = delete;
    Arena &operator=(const Arena &) = delete;
};
void run(headless::Session &session, const Packet &origin, const std::vector<std::uint16_t> &inputs,
         Boundary boundary, Shared &out) {
    auto current = origin;
    const float initial_deaths = session.state().deaths;
    bool spell_seen = session.state().spell_active;
    for (auto input : inputs) {
        const auto before = session.state();
        if (stage_terminal(boundary.story, before) ||
            (boundary.spell >= 0 && spell_seen && !before.spell_active)) {
            out.status = BranchStatus::Terminal;
            return;
        }
        const auto start = Clock::now();
        const auto io_before = session.file_io_time_ns();
        ++out.attempts;
        headless::State after;
        try {
            after = session.step(input);
        } catch (...) {
            const auto ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
            const auto io = session.file_io_time_ns() - io_before;
            out.file_io_ns += io;
            out.native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
            out.status = BranchStatus::Fault;
            return;
        }
        const auto ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
        const auto io = session.file_io_time_ns() - io_before;
        out.file_io_ns += io;
        out.native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
        ++out.completed;
        advance_receipt(current, input, capture(session));
        out.records[out.frames++] = current;
        if (after.player_state == 2 || after.deaths != initial_deaths) {
            out.status = BranchStatus::Death;
            return;
        }
        spell_seen = spell_seen || after.spell_active;
        if (stage_terminal(boundary.story, after) ||
            (boundary.spell >= 0 && spell_seen && !after.spell_active)) {
            out.status = BranchStatus::Terminal;
            return;
        }
    }
    out.status = BranchStatus::Horizon;
}
} // namespace
Prediction predict(headless::Session &session, const Packet &origin,
                   const std::vector<std::uint16_t> &inputs, Boundary boundary,
                   unsigned timeout_ms) {
    const auto begin = Clock::now();
    if (inputs.empty() || inputs.size() > maximum_horizon || !timeout_ms || timeout_ms > 60000)
        throw std::runtime_error("invalid_shadow_rollout_budget");
    for (auto input : inputs)
        input_value(input);
    Packet actual = origin;
    actual.state = capture(session);
    if (difference(origin, actual))
        throw std::runtime_error("shadow_rollout_origin_mismatch");
    if (origin.state[Field::Deaths] || origin.state[Field::BombsUsed] ||
        origin.state[Field::PlayerState] == 2)
        throw std::runtime_error("shadow_rollout_after_nmnb_failure");
    unsigned threads = 0;
    for (const auto &entry : std::filesystem::directory_iterator("/proc/self/task")) {
        (void)entry;
        ++threads;
    }
    if (threads != 1)
        throw std::runtime_error("shadow_rollout_requires_single_thread");
    // A fork isolates memory, not paths or inherited file descriptions. Native
    // score/config writes must not change the live owner's next initialization.
    char scratch[] = "/tmp/th08-shadow-branch-XXXXXX";
    if (!mkdtemp(scratch))
        throw std::runtime_error("shadow_branch_scratch_failed");
    struct Cleanup {
        std::filesystem::path directory;
        ~Cleanup() {
            std::error_code error;
            std::filesystem::remove_all(directory, error);
        }
    } cleanup{scratch};
    for (const char *name : {"score.dat", "th08.cfg"})
        if (std::filesystem::is_regular_file(name))
            std::filesystem::copy_file(name, cleanup.directory / name);
    Arena arena;
    const auto parent = getpid();
    const pid_t child = fork();
    if (child < 0)
        throw std::runtime_error("shadow_rollout_fork_failed");
    if (!child) {
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) != 0 || getppid() != parent)
            _exit(2);
        try {
            std::filesystem::current_path(cleanup.directory);
            run(session, origin, inputs, boundary, *arena.data);
        } catch (...) {
            arena.data->status = BranchStatus::Fault;
        }
        _exit(0);
    }
    int status = 0;
    bool timed_out = false, wait_failed = false;
    const auto deadline = begin + std::chrono::milliseconds(timeout_ms);
    for (;;) {
        const auto result = waitpid(child, &status, WNOHANG);
        if (result == child)
            break;
        if (result < 0 && errno == EINTR)
            continue;
        if (result < 0) {
            wait_failed = true;
            break;
        }
        if (Clock::now() >= deadline) {
            timed_out = true;
            break;
        }
        poll(nullptr, 0, 1);
    }
    if (timed_out) {
        kill(child, SIGKILL);
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
        }
    }
    Prediction result;
    result.origin = origin;
    result.requested = inputs.size();
    result.status = timed_out ? BranchStatus::Timeout : arena.data->status;
    if (wait_failed || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        result.status = timed_out ? BranchStatus::Timeout : BranchStatus::Fault;
    result.attempts = arena.data->attempts;
    result.completed = arena.data->completed;
    result.native_ns = arena.data->native_ns;
    result.file_io_ns = arena.data->file_io_ns;
    // Interrupted work can stop between counter writes. Charge the full cap and
    // never expose its partial records as verified execution.
    const bool interrupted =
        result.status == BranchStatus::Timeout || result.status == BranchStatus::Fault;
    result.charged_updates = interrupted ? inputs.size() : result.completed;
    if (!interrupted && arena.data->frames <= maximum_horizon)
        result.frames.assign(arena.data->records.begin(),
                             arena.data->records.begin() + arena.data->frames);
    result.wall_ms = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    return result;
}
} // namespace th08::shadow
