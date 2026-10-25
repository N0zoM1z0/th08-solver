#include "th08/shadow_proposal.hpp"
#include "modern/headless/session.hpp"
#include "th08/online_native.hpp"
#include "th08/shadow_dialogue.hpp"
#include "th08/shadow_native.hpp"
#include <algorithm>
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
struct Result {
    BranchStatus status = BranchStatus::Incomplete;
    unsigned count = 0;
    std::uint64_t attempts = 0, completed = 0, native_ns = 0, file_io_ns = 0;
    std::array<std::array<std::uint16_t, 23>, 4> inputs{};
};
void run(headless::Session &session, online::Policy &policy, const Packet &origin,
         const std::vector<std::uint16_t> &lead, Boundary boundary, Result &out,
         bool advance_dialogue) {
    bool spell_seen = session.state().spell_active;
    online::Observation observation;
    for (unsigned i = 0; i < lead.size(); ++i) {
        const auto begin = Clock::now();
        const auto io_before = session.file_io_time_ns();
        ++out.attempts;
        headless::State state;
        const auto record_time = [&] {
            const auto total =
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
            const auto io = session.file_io_time_ns() - io_before;
            out.native_ns += std::uint64_t(total) > io ? std::uint64_t(total) - io : 0;
            out.file_io_ns += io;
        };
        try {
            state = session.step(lead[i]);
        } catch (...) {
            record_time();
            out.status = BranchStatus::Fault;
            return;
        }
        record_time();
        ++out.completed;
        if (state.player_state == 2 || state.deaths != 0) {
            out.status = BranchStatus::Death;
            return;
        }
        spell_seen = spell_seen || state.spell_active;
        if (stage_terminal(boundary.story, state) ||
            (boundary.spell >= 0 && spell_seen && !state.spell_active)) {
            out.status = BranchStatus::Terminal;
            return;
        }
        online::capture_memory(session, origin.sequence + i + 1, observation, true);
        policy.observe(observation);
    }
    if (advance_dialogue && session.state().dialogue_present) {
        auto previous = lead.back();
        for (unsigned i = 0; i < 23; ++i) {
            previous = dialogue_input(previous, origin.sequence + lead.size() + i + 1);
            out.inputs[0][i] = previous;
        }
        out.count = 1;
        out.status = BranchStatus::Horizon;
        return;
    }
    std::vector<online::ValueProposal> proposals;
    policy.decide(observation, nullptr, &proposals);
    if (proposals.size() > 4)
        throw std::runtime_error("shadow_proposal_capacity");
    for (const auto &proposal : proposals) {
        if (proposal.inputs.size() != 23)
            throw std::runtime_error("shadow_proposal_horizon");
        for (unsigned i = 0; i < 23; ++i) {
            input_value(proposal.inputs[i]);
            out.inputs[out.count][i] = proposal.inputs[i];
        }
        ++out.count;
    }
    out.status = BranchStatus::Horizon;
}
} // namespace
std::vector<MotionProposal> motion_recovery_proposals(const std::vector<std::uint16_t> &lead) {
    if (lead.empty() || lead.size() > 32)
        throw std::runtime_error("invalid_shadow_motion_lead");
    for (auto input : lead)
        input_value(input);
    std::vector<MotionProposal> out;
    out.reserve(66);
    for (unsigned focus : {4u, 0u})
        for (unsigned direction : {0u, 16u, 32u, 64u, 128u, 80u, 144u, 96u, 160u}) {
            const auto motion = std::uint16_t(1 | focus | direction);
            auto held = lead;
            held.insert(held.end(), 24, motion);
            out.push_back({std::move(held), 0});
            if (!direction)
                continue;
            for (unsigned duration : {16u, 32u, 64u}) {
                auto pulse = lead;
                pulse.insert(pulse.end(), duration, motion);
                pulse.push_back(std::uint16_t(1 | focus));
                out.push_back({std::move(pulse), duration});
            }
        }
    return out;
}
Prediction predict_with_hold(headless::Session &session, const Packet &origin,
                             const std::vector<std::uint16_t> &prefix, std::size_t horizon,
                             Boundary boundary, unsigned timeout_ms, bool advance_dialogue) {
    if (prefix.empty())
        throw std::runtime_error("empty_shadow_hold_prefix");
    class Hold final : public RolloutGuidance {
        headless::Session &session_;
        std::uint16_t held_, actual_ = 0;
        std::uint64_t sequence_ = 0;
        bool spell_, advance_dialogue_;

      public:
        Hold(headless::Session &session, std::uint16_t held, Boundary boundary, bool dialogue)
            : session_(session), held_(held), spell_(boundary.spell >= 0),
              advance_dialogue_(dialogue) {}
        void observe(const Packet &packet) override {
            actual_ = std::uint16_t(packet.input);
            sequence_ = packet.sequence;
        }
        std::uint16_t next_input() override {
            if (advance_dialogue_ && session_.state().dialogue_present)
                return dialogue_input(actual_, sequence_ + 1);
            return std::uint16_t((held_ & 0xf5) |
                                 ((!spell_ && ((sequence_ + 1) & 1)) ? 0x1000 : 0));
        }
    } guidance(session, prefix.back(), boundary, advance_dialogue);
    return predict_guided(session, origin, prefix, horizon, guidance, boundary, timeout_ms);
}
Prediction predict_with_policy(headless::Session &session, online::Policy &policy,
                               const Packet &origin, const std::vector<std::uint16_t> &prefix,
                               std::size_t horizon, Boundary boundary, unsigned timeout_ms,
                               bool advance_dialogue) {
    class Guidance final : public RolloutGuidance {
        headless::Session &session_;
        online::Policy &policy_;
        online::Observation observation_;
        std::array<std::uint16_t, 8> pending_{};
        unsigned position_ = 0, count_ = 0;
        bool dialogue_;
        bool advance_dialogue_;
        std::uint16_t actual_input_ = 0;
        std::uint64_t sequence_ = 0;

      public:
        Guidance(headless::Session &session, online::Policy &policy, bool dialogue,
                 bool advance_dialogue)
            : session_(session), policy_(policy), dialogue_(dialogue),
              advance_dialogue_(advance_dialogue) {}
        void observe(const Packet &packet) override {
            sequence_ = packet.sequence;
            actual_input_ = std::uint16_t(packet.input);
            online::capture_memory(session_, sequence_, observation_, true);
            policy_.observe(observation_);
        }
        std::uint16_t next_input() override {
            if (advance_dialogue_ && session_.state().dialogue_present) {
                position_ = count_ = 0;
                return dialogue_input(actual_input_, sequence_ + 1);
            }
            if (position_ == count_) {
                std::vector<online::ValueProposal> proposals;
                const auto decision = policy_.decide(observation_, nullptr, &proposals);
                count_ = pending_.size();
                position_ = 0;
                if (!proposals.empty()) {
                    count_ = std::min<std::size_t>(count_, proposals.front().inputs.size());
                    if (!count_)
                        throw std::runtime_error("empty_shadow_policy_continuation");
                    std::copy_n(proposals.front().inputs.begin(), count_, pending_.begin());
                } else
                    pending_.fill(decision.action);
            }
            const auto input = pending_[position_++];
            return std::uint16_t((input & ~0x1000) |
                                 ((dialogue_ && ((sequence_ + 1) & 1)) ? 0x1000 : 0));
        }
    } guidance(session, policy, boundary.spell < 0, advance_dialogue);
    return predict_guided(session, origin, prefix, horizon, guidance, boundary, timeout_ms);
}
FutureProposals propose_after_lead(headless::Session &session, online::Policy &policy,
                                   const Packet &origin, const std::vector<std::uint16_t> &lead,
                                   Boundary boundary, unsigned timeout_ms, bool advance_dialogue) {
    const auto begin = Clock::now();
    if (lead.empty() || lead.size() > 32 || !timeout_ms || timeout_ms > 1000)
        throw std::runtime_error("invalid_shadow_proposal_budget");
    for (auto input : lead)
        input_value(input);
    Packet actual = origin;
    actual.state = capture(session);
    if (difference(actual, origin))
        throw std::runtime_error("shadow_proposal_origin_mismatch");
    if (origin.state[Field::PlayerState] == 2 || origin.state[Field::Deaths] ||
        origin.state[Field::BombsUsed])
        throw std::runtime_error("shadow_proposal_after_nmnb_failure");
    unsigned threads = 0;
    for (const auto &entry : std::filesystem::directory_iterator("/proc/self/task")) {
        (void)entry;
        ++threads;
    }
    if (threads != 1)
        throw std::runtime_error("shadow_proposal_requires_single_thread");
    char path[] = "/tmp/th08-shadow-proposal-XXXXXX";
    if (!mkdtemp(path))
        throw std::runtime_error("shadow_proposal_scratch_failed");
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() {
            std::error_code error;
            std::filesystem::remove_all(path, error);
        }
    } cleanup{path};
    for (const char *name : {"score.dat", "th08.cfg"})
        if (std::filesystem::is_regular_file(name))
            std::filesystem::copy_file(name, cleanup.path / name);
    void *memory =
        mmap(nullptr, sizeof(Result), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED)
        throw std::runtime_error("shadow_proposal_allocation_failed");
    struct Arena {
        Result *data;
        ~Arena() {
            munmap(data, sizeof(Result));
        }
    } arena{new (memory) Result{}};
    const auto parent = getpid();
    const auto child = fork();
    if (child < 0)
        throw std::runtime_error("shadow_proposal_fork_failed");
    if (!child) {
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) || getppid() != parent)
            _exit(2);
        try {
            std::filesystem::current_path(cleanup.path);
            run(session, policy, origin, lead, boundary, *arena.data, advance_dialogue);
        } catch (...) {
            arena.data->status = BranchStatus::Fault;
        }
        _exit(0);
    }
    bool timeout = false, failed = false;
    int status = 0;
    while (true) {
        const auto result = waitpid(child, &status, WNOHANG);
        if (result == child)
            break;
        if (result < 0 && errno == EINTR)
            continue;
        if (result < 0) {
            failed = true;
            break;
        }
        if (Clock::now() >= begin + std::chrono::milliseconds(timeout_ms)) {
            timeout = true;
            kill(child, SIGKILL);
            while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
            }
            break;
        }
        poll(nullptr, 0, 1);
    }
    FutureProposals result;
    const auto &raw = *arena.data;
    result.status = timeout ? BranchStatus::Timeout : raw.status;
    if (!timeout && (failed || !WIFEXITED(status) || WEXITSTATUS(status)))
        result.status = BranchStatus::Fault;
    result.attempts = raw.attempts;
    result.completed = raw.completed;
    result.charged_updates =
        result.status == BranchStatus::Timeout || result.status == BranchStatus::Fault
            ? lead.size()
            : raw.completed;
    result.native_ns = raw.native_ns;
    result.file_io_ns = raw.file_io_ns;
    if (result.status == BranchStatus::Horizon) {
        if (raw.count > 4)
            throw std::runtime_error("shadow_proposal_corrupt_result");
        for (unsigned i = 0; i < raw.count; ++i) {
            result.inputs.emplace_back(raw.inputs[i].begin(), raw.inputs[i].end());
        }
    }
    result.wall_ms = std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    return result;
}
} // namespace th08::shadow
