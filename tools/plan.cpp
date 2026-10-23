// th08_plan: exact-simulator planner for the native headless runtime.
//
// The game itself is the forecaster. A fork()ed child replays a nominal input
// sequence for R updates and records every hazard exactly as the original
// collision code receives it (bullet boxes at the graze/lethal gate,
// CalcLaserHitbox arguments, CheckLethalCollision bounds). A beam search then
// moves an exact float32 player model through that tape using the game's own
// Float3/Rotate arithmetic. The chosen plan is verified by another forked
// rollout (CEGAR-style: a failing verification adds its tape and replans), and
// only a verified prefix is committed in the owning process.
//
// Backtracking uses checkpoint processes: every C committed updates the worker
// forks; the parent blocks as a checkpoint holder and, if the child reports
// that it is stuck, retries from its own (older) state with a perturbed
// variant. A subreaper root reaps the chain. No game state is serialized; fork
// gives each branch its own copy of every process-global manager.
#include "modern/headless/replay_check.hpp"
#include "modern/headless/runtime.hpp"
#include "modern/headless/session.hpp"
#include "th08/plan_beam.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_observation.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/plan_route.hpp"

#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "Supervisor.hpp"
#include "ZunMath.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <new>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
using th08::Float3;
using Clock = std::chrono::steady_clock;

namespace {
using th08::plan::RolloutStatus;
// ---------------------------------------------------------------- shared tape
constexpr int kMaxFrames = th08::plan::max_rollout;
constexpr int kMaxBullets = 2048, kMaxLasers = 640, kMaxRegions = 512;
constexpr int kMaxTapes = 6;
struct Box {
    float x0, y0, x1, y1; // native inclusive bounds (min, max)
};
struct Laser {
    float cx, cy, w, h, ox, oy, angle; // CalcLaserHitbox(position, size, origin, angle)
};
struct FrameTape {
    int nb, nl, nr;
    th08::headless::State state; // Full public observation after this native update.
    std::uint16_t input;         // Actual sampled command, including dialogue choices.
    bool complete;
    Box b[kMaxBullets];
    Laser l[kMaxLasers];
    Box r[kMaxRegions];
};
struct Tape : th08::plan::RolloutSummary {
    int death_kind = 0;          // headless::CollisionKind of the native hit
    FrameTape f[kMaxFrames + 1]; // 1-based
};
enum ExitCode {
    kOk = 0,
    kRuntimeError = 1,
    kPrefixRejected = 2,
    kBacktrack = 3,
    kReleased = 4,
    kBudget = 5
};
struct StageTransition {
    std::uint64_t frame = 0, calculations_before = 0, calculations_after = 0;
    int from = 0, to = 0;
};
struct ExecutionAccounting {
    std::uint64_t control_updates = 0;
    int stage_transition_count = 0;
    // A native story traverses at most the nine source-owned stage entries.
    std::array<StageTransition, 9> stage_transitions{};
};
ExecutionAccounting accounting;
struct Shared {
    std::atomic<long> rollouts, searches, cegar, backtracks, commits, releases;
    std::atomic<long> max_frame, model_breaks, retries, quiet;
    // Path-dependence probes: hazards of an alternative path that also occur,
    // bit-identically, in the nominal path's tape, per horizon bucket.
    std::atomic<long> probe_count, probe_total[4], probe_same[4], probe_rng_diverged,
        probe_rng_frame_sum;
    // Attempted includes an update interrupted by an exception or signal. Every
    // retry, discarded rollout and committed update contributes to these totals.
    std::atomic<std::uint64_t> native_attempts, native_updates, native_ns;
    std::atomic<std::uint64_t> native_calculations, native_controls, native_stage_transitions;
    std::atomic<std::uint64_t> rollout_attempts, rollout_updates, commit_updates;
    std::atomic<std::uint64_t> prefix_attempts, prefix_updates;
    std::atomic<bool> prefix_complete;
    // Sums across enabled, over-budget layers, including discarded searches and
    // retried branches. Bin counts are per-layer work, not distinct world states.
    std::atomic<std::uint64_t> diversity_selections, diversity_candidates, diversity_bins,
        diversity_retained_bins, diversity_selection_ns;
    std::atomic<long> rollout_failures;
    std::atomic<bool> fatal_error, report_written;
    th08::headless::State report_state;
    ExecutionAccounting report_accounting;
    long report_frames = 0;
    int depth_pids[4096];
    int holder_depth[4096];
    bool released[4096];
    Tape tapes[kMaxTapes];
    // Hazards seen by a branch that got stuck, keyed by absolute update. Most
    // observed hazards can guide an older retry before its own rollout reaches
    // the trap. They remain branch-specific proposals, never an acceptance oracle.
    Tape future;
    long future_base; // future.f[1] is absolute update future_base + 1
    bool future_valid;
};
Shared *shared = nullptr;

// ---------------------------------------------------------------- options
th08::plan::Options opt;
Clock::time_point t_start;
std::string scratch_path;
// Populated once by the root before forking. Descendants inherit these immutable
// hashes without repeating file I/O or charging it to native simulation timing.
std::string dat_sha256, binary_sha256, prefix_sha256, actions_sha256;
std::vector<std::uint16_t> prefix_actions;

std::string file_sha256(const std::string &path, const char *label,
                        th08::plan::PrefixParser *prefix = nullptr) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error(std::string("cannot read ") + label + " for SHA256");
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(),
                                                                    EVP_MD_CTX_free);
    if (!context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("SHA256 initialization failed");
    std::array<char, 65536> buffer;
    while (input.read(buffer.data(), buffer.size()) || input.gcount()) {
        if (EVP_DigestUpdate(context.get(), buffer.data(), std::size_t(input.gcount())) != 1)
            throw std::runtime_error("SHA256 update failed");
        if (prefix)
            prefix->append(buffer.data(), std::size_t(input.gcount()));
    }
    if (!input.eof())
        throw std::runtime_error("SHA256 input read failed");
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest;
    unsigned length = 0;
    if (EVP_DigestFinal_ex(context.get(), digest.data(), &length) != 1 || length != 32)
        throw std::runtime_error("SHA256 finalization failed");
    constexpr char hex[] = "0123456789abcdef";
    std::string result;
    result.reserve(64);
    for (unsigned i = 0; i < length; ++i) {
        result.push_back(hex[digest[i] >> 4]);
        result.push_back(hex[digest[i] & 15]);
    }
    return result;
}

void write_hashes(std::ostream &out) {
    // Missing inputs carry no digest; never serialize their private locations.
    out << ",\"dat_sha256\":";
    if (dat_sha256.empty())
        out << "null";
    else
        out << '"' << dat_sha256 << '"';
    out << ",\"binary_sha256\":";
    if (binary_sha256.empty())
        out << "null";
    else
        out << '"' << binary_sha256 << '"';
    out << ",\"prefix_sha256\":";
    if (prefix_sha256.empty())
        out << "null";
    else
        out << '"' << prefix_sha256 << '"';
    if (!opt.verify.empty()) {
        out << ",\"actions_sha256\":";
        if (actions_sha256.empty())
            out << "null";
        else
            out << '"' << actions_sha256 << '"';
    }
    const auto &target = th08::headless::replay_executable_target;
    out << ",\"replay_target_exe_size\":" << target.size
        << ",\"replay_target_exe_checksum\":" << target.checksum << ",\"replay_target_version\":\""
        << target.version << '"';
}

void cleanup_scratch() {
    if (scratch_path.empty())
        return;
    std::error_code ec;
    fs::current_path("/", ec);
    fs::remove_all(scratch_path, ec);
}

// ---------------------------------------------------------------- exact model
struct Speeds {
    float axis[2], diag[2]; // [0] unfocused (primary), [1] focused (secondary)
    float hmul, vmul, frm;
    float minx, miny, maxx, maxy;
    Float3 half;
};
Speeds read_speeds() {
    Speeds s;
    s.axis[0] = th08::g_Player.primaryShtFile->normalAxisSpeed;
    s.diag[0] = th08::g_Player.primaryShtFile->normalDiagonalSpeed;
    s.axis[1] = th08::g_Player.secondaryShtFile->focusedAxisSpeed;
    s.diag[1] = th08::g_Player.secondaryShtFile->focusedDiagonalSpeed;
    s.hmul = th08::g_Player.horizontalSpeedMultiplier;
    s.vmul = th08::g_Player.verticalSpeedMultiplier;
    s.frm = th08::g_Supervisor.framerateMultiplier;
    s.minx = th08::g_GameManager.playerMovementTopLeftPos.x;
    s.miny = th08::g_GameManager.playerMovementTopLeftPos.y;
    s.maxx = th08::g_GameManager.playerMovementTopLeftPos.x +
             th08::g_GameManager.playerMovementAreaSize.x;
    s.maxy = th08::g_GameManager.playerMovementTopLeftPos.y +
             th08::g_GameManager.playerMovementAreaSize.y;
    s.half = th08::g_Player.hurtboxHalfSize;
    return s;
}
// Player::UpdateMovementAndOptions direction priority and float operation order.
inline void move(const Speeds &s, float &x, float &y, std::uint16_t in) {
    int dx = 0, dy = 0;
    if ((in & 0x50) == 0x50)
        dx = -1, dy = -1;
    else if ((in & 0x60) == 0x60)
        dx = -1, dy = 1;
    else if ((in & 0x90) == 0x90)
        dx = 1, dy = -1;
    else if ((in & 0xA0) == 0xA0)
        dx = 1, dy = 1;
    else if (in & 0x20)
        dy = 1;
    else if (in & 0x10)
        dy = -1;
    else if (in & 0x40)
        dx = -1;
    else if (in & 0x80)
        dx = 1;
    const int f = (in & 4) ? 1 : 0;
    float h = 0, v = 0;
    if (dx && dy) {
        h = dx * s.diag[f];
        v = dy * s.diag[f];
    } else {
        h = dx * s.axis[f];
        v = dy * s.axis[f];
    }
    h *= s.hmul;
    v *= s.vmul;
    x += h * s.frm;
    y += v * s.frm;
    if (x < s.minx)
        x = s.minx;
    else if (x > s.maxx)
        x = s.maxx;
    if (y < s.miny)
        y = s.miny;
    else if (y > s.maxy)
        y = s.maxy;
}
inline bool box_hit(const Box &b, const Float3 &mn, const Float3 &mx) {
    return !(mn.x > b.x1 || mn.y > b.y1 || mx.x < b.x0 || mx.y < b.y0);
}
// Player::CalcLaserHitbox lethal predicate with the game's own Float3/Rotate.
inline bool laser_hit(const Laser &l, const Float3 &p, const Float3 &half, float margin) {
    Float3 origin, position, size, in_min, in_max, pmin, pmax;
    origin.x = l.ox, origin.y = l.oy, origin.z = 0;
    position.x = l.cx, position.y = l.cy, position.z = 0;
    size.x = l.w, size.y = l.h, size.z = 0;
    in_min = p - origin;
    th08::Rotate(&in_max, &in_min, -l.angle);
    in_max.z = 0.0f;
    in_min = in_max + origin;
    pmin = in_min - half;
    pmax = in_min + half;
    in_min = position - size / 2.0f;
    in_max = position + size / 2.0f;
    return !(pmin.x - margin > in_max.x) && !(pmax.x + margin < in_min.x) &&
           !(pmin.y - margin > in_max.y) && !(pmax.y + margin < in_min.y);
}

// ---------------------------------------------------------------- grid index
constexpr float kCell = 16.f, kGx0 = -64.f, kGy0 = -64.f;
constexpr int kGW = 34, kGH = 38;
constexpr float kMargin = 1.5f;
struct FrameIndex {
    std::vector<int> start, items; // CSR over cells; items index bullets (>=0) or regions (~i)
};
inline int cell_of(float v, float origin, int n) {
    int c = int(std::floor((v - origin) / kCell));
    return c < 0 ? 0 : c >= n ? n - 1 : c;
}
void build_index(const FrameTape &f, const Float3 &half, FrameIndex &ix) {
    const float ex = half.x + kMargin + 0.01f, ey = half.y + kMargin + 0.01f;
    std::vector<int> cnt(kGW * kGH + 1, 0);
    auto each = [&](const Box &b, auto &&fn) {
        const int cx0 = cell_of(b.x0 - ex, kGx0, kGW), cx1 = cell_of(b.x1 + ex, kGx0, kGW);
        const int cy0 = cell_of(b.y0 - ey, kGy0, kGH), cy1 = cell_of(b.y1 + ey, kGy0, kGH);
        for (int cy = cy0; cy <= cy1; ++cy)
            for (int cx = cx0; cx <= cx1; ++cx)
                fn(cy * kGW + cx);
    };
    for (int i = 0; i < f.nb; ++i)
        each(f.b[i], [&](int c) { ++cnt[c + 1]; });
    for (int i = 0; i < f.nr; ++i)
        each(f.r[i], [&](int c) { ++cnt[c + 1]; });
    for (int c = 0; c < kGW * kGH; ++c)
        cnt[c + 1] += cnt[c];
    ix.start = cnt;
    ix.items.assign(cnt[kGW * kGH], 0);
    std::vector<int> pos(cnt.begin(), cnt.end() - 1);
    for (int i = 0; i < f.nb; ++i)
        each(f.b[i], [&](int c) { ix.items[pos[c]++] = i; });
    for (int i = 0; i < f.nr; ++i)
        each(f.r[i], [&](int c) { ix.items[pos[c]++] = ~i; });
}
// Returns 2 for a native lethal hit, 1 for a margin-only near miss, 0 if clear.
inline int probe(const FrameTape &f, const FrameIndex &ix, const Float3 &half, float x, float y,
                 bool want_margin) {
    Float3 p;
    p.x = x, p.y = y, p.z = 0;
    const Float3 mn = p - half, mx = p + half;
    Float3 mmn = mn, mmx = mx;
    mmn.x -= kMargin, mmn.y -= kMargin, mmx.x += kMargin, mmx.y += kMargin;
    const int c = cell_of(y, kGy0, kGH) * kGW + cell_of(x, kGx0, kGW);
    int result = 0;
    for (int k = ix.start[c]; k < ix.start[c + 1]; ++k) {
        const int it = ix.items[k];
        const Box &b = it >= 0 ? f.b[it] : f.r[~it];
        if (box_hit(b, mn, mx))
            return 2;
        if (want_margin && !result && box_hit(b, mmn, mmx))
            result = 1;
    }
    for (int i = 0; i < f.nl; ++i) {
        if (laser_hit(f.l[i], p, half, 0))
            return 2;
        if (want_margin && !result && laser_hit(f.l[i], p, half, kMargin))
            result = 1;
    }
    return result;
}

// ---------------------------------------------------------------- rollouts
th08::headless::Session *session = nullptr;
std::vector<std::uint16_t> history; // committed inputs from the checkpoint start
long frame_now = 0;                 // committed updates
int variant = 0, depth = 0;
long quiet_until = -1; // absolute update up to which shots are withheld

std::uint16_t decorate(std::uint16_t move_bits, long frame_index, int shot_off) {
    std::uint16_t a = move_bits;
    // Withholding shots reduces shot-hit effect RNG coupling. Grazing, existing
    // shots and character/focus changes can still make futures path-dependent.
    if (opt.shoot && (!opt.quiet_retries || (shot_off <= 0 && frame_index > quiet_until)))
        a |= 1;
    if (opt.spell < 0 && (frame_index % 2))
        a |= 4096; // advance real dialogue scripts
    return a;
}
bool record_frame(FrameTape &f, const th08::headless::State &st) {
    using namespace th08::headless;
    const auto &bb = current_bullet_boxes();
    const auto &lh = current_laser_hitboxes();
    const auto &lr = current_lethal_regions();
    if (!th08::plan::fits_tape(bb.size(), lh.size(), lr.size(), kMaxBullets, kMaxLasers,
                               kMaxRegions))
        return false;
    f.nb = int(bb.size());
    for (int i = 0; i < f.nb; ++i) {
        // CheckBulletCollision bound expressions, evaluated in the same order.
        const float px = bb[i].x, py = bb[i].y, sx = bb[i].w, sy = bb[i].h;
        f.b[i] = {px - sx / 2.0f, py - sy / 2.0f, sx / 2.0f + px, sy / 2.0f + py};
    }
    f.nl = int(lh.size());
    for (int i = 0; i < f.nl; ++i)
        f.l[i] = {lh[i].center_x, lh[i].center_y, lh[i].full_width, lh[i].full_height,
                  lh[i].origin_x, lh[i].origin_y, lh[i].angle};
    f.nr = int(lr.size());
    for (int i = 0; i < f.nr; ++i)
        f.r[i] = {lr[i].bounds.left, lr[i].bounds.top, lr[i].bounds.right, lr[i].bounds.bottom};
    f.state = st;
    f.complete = opt.spell >= 0 || opt.story ? false : st.stage_complete;
    return true;
}
bool started_spell = false;
bool is_complete(const th08::headless::State &st) {
    if (opt.story) // Final A/B clear, or the requested partial-run boundary
        return th08::plan::story_terminal(st.stage, st.stage_complete, opt.until_stage);
    if (opt.spell >= 0) {
        if (st.spell_active)
            started_spell = true;
        return started_spell && !st.spell_active;
    }
    return st.stage_complete;
}
enum class NativeUse { Rollout, Commit, Verify, Prefix };

th08::headless::State native_step(std::uint16_t input, NativeUse use) {
    if (shared->fatal_error)
        throw std::runtime_error("another planner process failed");
    ++shared->native_attempts;
    if (use == NativeUse::Rollout)
        ++shared->rollout_attempts;
    if (use == NativeUse::Prefix)
        ++shared->prefix_attempts;
    const auto before = session->state();
    const auto start = Clock::now();
    const auto io_start = th08::headless::file_io_ns;
    auto add_time = [&] {
        const auto wall_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
        const auto io_ns = th08::headless::file_io_ns - io_start;
        shared->native_ns += std::uint64_t(wall_ns) > io_ns ? std::uint64_t(wall_ns) - io_ns : 0;
    };
    th08::headless::State st;
    try {
        st = session->step(input);
    } catch (...) {
        add_time();
        shared->native_calculations +=
            th08::headless::replay_calculation_updates() - before.calculation_updates;
        throw;
    }
    add_time();
    ++shared->native_updates;
    if (use == NativeUse::Rollout)
        ++shared->rollout_updates;
    else if (use == NativeUse::Commit)
        ++shared->commit_updates;
    else if (use == NativeUse::Prefix)
        ++shared->prefix_updates;
    const auto calculations = st.calculation_updates - before.calculation_updates;
    shared->native_calculations += calculations;
    const bool stage_changed = st.stage != before.stage;
    if (calculations == 0) {
        ++accounting.control_updates;
        ++shared->native_controls;
    }
    if (stage_changed) {
        ++shared->native_stage_transitions;
        if (accounting.stage_transition_count >= int(accounting.stage_transitions.size()))
            throw std::runtime_error("unsupported number of native stage transitions");
        accounting.stage_transitions[accounting.stage_transition_count++] = {
            st.frame, before.calculation_updates, st.calculation_updates, before.stage, st.stage};
    }
    if (!th08::plan::supported_recording_step(calculations, stage_changed)) {
        std::fprintf(stderr,
                     "unsupported recording calculation boundary at update %llu: "
                     "%llu calculations, stage %d -> %d\n",
                     static_cast<unsigned long long>(st.frame),
                     static_cast<unsigned long long>(calculations), before.stage, st.stage);
        throw std::runtime_error("unexplained native recording calculation boundary");
    }
    if (opt.final_route != th08::plan::FinalRoute::Natural && (st.stage == 6 || st.stage == 7) &&
        st.stage != (opt.final_route == th08::plan::FinalRoute::A ? 6 : 7))
        throw std::runtime_error("native story entered a different final route");
    // An explicit stage selects the DAT program; it does not establish that its
    // active spell matches the requested checkpoint. Check each native update
    // before rollout, commit or fresh verification can count it as progress.
    if (opt.spell >= 0 && st.spell_active && st.spell != opt.spell) {
        std::fprintf(stderr, "native checkpoint mismatch: requested spell %d, active spell %d\n",
                     opt.spell, st.spell);
        throw std::runtime_error("active spell does not match requested checkpoint");
    }
    return st;
}

// Fork, replay `plan` for opt.rollout updates with hazard recording, write tape `slot`.
void rollout(const std::vector<std::uint16_t> &plan, int slot) {
    ++shared->rollouts;
    Tape &t = shared->tapes[slot];
    t.frames = 0;
    t.death = 0;
    t.death_kind = 0;
    t.status = RolloutStatus::Running;
    if (int(plan.size()) != opt.rollout)
        throw std::logic_error("rollout plan does not cover the requested horizon");
    const pid_t pid = fork();
    if (pid < 0) {
        t.status = RolloutStatus::ChildError;
        ++shared->rollout_failures;
        throw std::runtime_error("rollout fork failed");
    }
    if (pid == 0) {
        th08::headless::record_bullet_boxes = true;
        const float initial_deaths = session->state().deaths;
        for (int j = 1; j <= opt.rollout; ++j) {
            th08::headless::State st;
            try {
                const auto input =
                    th08::plan::final_route_input(opt.final_route, session->state(), frame_now + j)
                        .value_or(plan[j - 1]);
                st = native_step(input, NativeUse::Rollout);
                if (!record_frame(t.f[j], st)) {
                    t.status = RolloutStatus::Capacity;
                    _exit(kRuntimeError);
                }
                t.f[j].input = input;
            } catch (...) {
                t.status = RolloutStatus::StepError;
                _exit(kRuntimeError);
            }
            t.f[j].complete = is_complete(st);
            t.frames = j;
            if (st.player_state == 2 || st.deaths != initial_deaths) {
                t.death = j;
                t.death_kind = int(session->collision().kind);
                t.status = RolloutStatus::Death;
                break;
            }
            if (t.f[j].complete) {
                t.status = RolloutStatus::Terminal;
                break;
            }
            if (j == opt.rollout)
                t.status = RolloutStatus::Horizon;
        }
        _exit(kOk);
    }
    const auto child = th08::plan::wait_for_child(pid);
    if (!child.exited() || th08::plan::failed_rollout(t, opt.rollout)) {
        if (t.status == RolloutStatus::Running)
            t.status = child.exited() ? RolloutStatus::Incomplete : RolloutStatus::ChildError;
        ++shared->rollout_failures;
        std::fprintf(stderr, "rollout failed: status=%d frames=%d child_status=%d wait_error=%d\n",
                     int(t.status), t.frames, child.status, child.error);
        throw std::runtime_error("native rollout failed");
    }
}

// ---------------------------------------------------------------- beam search
using Node = th08::plan::BeamNode;
struct SearchResult {
    std::vector<std::uint16_t> moves;             // movement/focus bits for plan[0..]
    int depth = 0;                                // updates survived in the model
    std::vector<std::vector<std::uint16_t>> alts; // diverse end regions, best first
};
const std::uint16_t kDirs[9] = {0, 16, 32, 64, 128, 16 | 64, 16 | 128, 32 | 64, 32 | 128};
std::uint64_t rng_state = 88172645463325252ULL;
// Enough open-addressing slots for 18 successors per maximum-width beam.
constexpr int kSeenBits = 18, kSeenSize = 1 << kSeenBits;
std::vector<std::uint32_t> seen_key(kSeenSize), seen_gen(kSeenSize);
std::uint32_t generation = 0;
inline std::uint64_t xrand() {
    rng_state ^= rng_state << 13, rng_state ^= rng_state >> 7, rng_state ^= rng_state << 17;
    return rng_state;
}
SearchResult search(const std::vector<int> &slots, int horizon, int beam_width, float pref_x,
                    float pref_y, const Speeds &sp, std::uint16_t latched, float x0, float y0,
                    int future_offset = 0, int future_until = 0,
                    const std::vector<std::uint16_t> *anchor = nullptr, int anchor_len = 0) {
    ++shared->searches;
    const int own = horizon;
    if (future_until > horizon)
        horizon = future_until;
    std::vector<std::vector<FrameIndex>> ix(slots.size(), std::vector<FrameIndex>(horizon + 1));
    for (std::size_t s = 0; s < slots.size(); ++s)
        for (int j = 1; j <= own && j <= shared->tapes[slots[s]].frames; ++j)
            build_index(shared->tapes[slots[s]].f[j], sp.half, ix[s][j]);
    std::vector<FrameIndex> fix(horizon + 1);
    for (int j = own + 1; j <= horizon; ++j)
        if (j - future_offset >= 1 && j - future_offset <= shared->future.frames)
            build_index(shared->future.f[j - future_offset], sp.half, fix[j]);
    auto test = [&](int j, float x, float y, bool margin) {
        if (j > own) {
            const int q = j - future_offset;
            if (q < 1 || q > shared->future.frames)
                return 0;
            return probe(shared->future.f[q], fix[j], sp.half, x, y, margin);
        }
        int worst = 0;
        for (std::size_t s = 0; s < slots.size(); ++s) {
            const Tape &t = shared->tapes[slots[s]];
            if (j > t.frames)
                continue;
            worst = std::max(worst, probe(t.f[j], ix[s][j], sp.half, x, y, margin));
            if (worst == 2)
                break;
        }
        return worst;
    };
    std::vector<std::vector<Node>> layers(horizon + 1);
    float x1 = x0, y1 = y0;
    move(sp, x1, y1, latched);
    if (test(1, x1, y1, false) == 2)
        return {};
    layers[1].push_back({x1, y1, -1, std::uint16_t(latched & 0xF4), 0, 0});
    int deepest = 1;
    for (int j = 1; j < horizon && !layers[j].empty(); ++j) {
        auto &cur = layers[j];
        auto &next = layers[j + 1];
        ++generation;
        const bool branch = j < 16 || (j % 2) == 0; // coarser decisions far ahead
        for (int pi = 0; pi < int(cur.size()); ++pi) {
            const Node &p = cur[pi];
            for (int f = 0; f < 2; ++f)
                for (int d = 0; d < 9; ++d) {
                    std::uint16_t a = std::uint16_t(kDirs[d] | (f ? 4 : 0));
                    if (anchor && j - 1 < anchor_len) { // follow the nominal until the anchor ends
                        if (a !=
                            ((*anchor)[std::min<std::size_t>(j - 1, anchor->size() - 1)] & 0xF4))
                            continue;
                    } else if (!branch && a != p.act)
                        continue;
                    float x = p.x, y = p.y;
                    move(sp, x, y, a);
                    // Cell-merged exact representatives: 0.5 px cells, focus kept apart.
                    const std::uint32_t key = (th08::plan::detail::half_pixel_bin(x) << 12 |
                                               th08::plan::detail::half_pixel_bin(y))
                                                  << 1 |
                                              std::uint32_t(f);
                    std::uint32_t h = (key * 2654435761u) >> (32 - kSeenBits);
                    bool dup = false;
                    while (seen_gen[h] == generation) {
                        if (seen_key[h] == key) {
                            dup = true;
                            break;
                        }
                        h = (h + 1) & (kSeenSize - 1);
                    }
                    if (dup)
                        continue;
                    seen_gen[h] = generation, seen_key[h] = key;
                    const int hit = test(j + 1, x, y, j + 1 <= 24);
                    if (hit == 2)
                        continue;
                    Node n{x, y, pi, a, std::uint16_t(p.viol + (hit == 1)), p.score};
                    const float dx = (x - pref_x) / 96.f, dy = (y - pref_y) / 96.f;
                    n.score += (hit == 1 ? (j < 12 ? 40.f : 10.f) : 0.f) +
                               0.02f * (dx * dx + dy * dy) + (a != p.act ? 0.01f : 0.f) +
                               float(xrand() % 1000) * 1e-6f;
                    next.push_back(n);
                }
        }
        if (opt.diversity_cell > 0 && int(next.size()) > beam_width) {
            const auto selection_start = Clock::now();
            const auto stats =
                th08::plan::retain_spatial_beam(next, std::size_t(beam_width), opt.diversity_cell);
            shared->diversity_selection_ns +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - selection_start)
                    .count();
            ++shared->diversity_selections;
            shared->diversity_candidates += stats.candidates;
            shared->diversity_bins += stats.bins;
            shared->diversity_retained_bins += stats.retained_bins;
        } else {
            if (int(next.size()) > beam_width) {
                std::nth_element(next.begin(), next.begin() + beam_width, next.end(),
                                 [](const Node &a, const Node &b) { return a.score < b.score; });
                next.resize(beam_width);
            }
            std::sort(next.begin(), next.end(),
                      [](const Node &a, const Node &b) { return a.score < b.score; });
        }
        if (!next.empty())
            deepest = j + 1;
    }
    SearchResult r;
    r.depth = deepest;
    const auto &last = layers[deepest];
    int best = 0;
    for (int i = 1; i < int(last.size()); ++i)
        if (last[i].score < last[best].score)
            best = i;
    auto trace = [&](int i) {
        std::vector<std::uint16_t> rev;
        for (int j = deepest; j > 1; --j) {
            rev.push_back(layers[j][i].act);
            i = layers[j][i].parent;
        }
        return std::vector<std::uint16_t>(rev.rbegin(),
                                          rev.rend()); // moves[k]: transition k+1 -> k+2
    };
    r.moves = trace(best);
    // Diverse alternatives (distinct 48 px end cells) for native re-ranking.
    std::vector<int> order(last.size());
    for (int i = 0; i < int(order.size()); ++i)
        order[i] = i;
    std::sort(order.begin(), order.end(),
              [&](int a, int b) { return last[a].score < last[b].score; });
    std::vector<int> cells;
    for (int i : order) {
        const int c = int(last[i].x / 48) * 64 + int(last[i].y / 48);
        if (std::find(cells.begin(), cells.end(), c) != cells.end())
            continue;
        cells.push_back(c);
        r.alts.push_back(trace(i));
        if (r.alts.size() >= 8)
            break;
    }
    return r;
}

// ---------------------------------------------------------------- reporting
void save_replay(const char *filename = "th8_solver.rpy") {
    const pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("replay writer fork failed");
    if (pid == 0) {
        try {
            const std::string path = (fs::path(opt.out_dir) / filename).string();
            fs::remove(path); // A failed writer must not certify an older artifact.
            th08::ReplayManager::SaveReplay(path.c_str(), "PLANNER ");
            _exit(fs::exists(path) && fs::file_size(path) > 0 ? kOk : kRuntimeError);
        } catch (...) {
            _exit(kRuntimeError);
        }
    }
    if (!th08::plan::wait_for_child(pid).exited())
        throw std::runtime_error("replay writer failed");
}

void write_options(std::ostream &out) {
    // Paths are private inputs, not public experiment metadata. Record every
    // behavioral option without leaking absolute data/output/replay locations.
    out << "{\"stage\":" << opt.stage << ",\"spell\":" << opt.spell
        << ",\"difficulty\":" << opt.difficulty << ",\"seed\":" << opt.seed
        << ",\"rollout\":" << opt.rollout << ",\"beam\":" << opt.beam
        << ",\"commit\":" << opt.commit << ",\"checkpoint\":" << opt.checkpoint
        << ",\"keep\":" << opt.keep << ",\"attempts\":" << opt.attempts
        << ",\"cegar\":" << opt.cegar << ",\"frames\":" << opt.frame_limit
        << ",\"seconds\":" << opt.seconds << ",\"shoot\":" << opt.shoot
        << ",\"quiet_retries\":" << opt.quiet_retries << ",\"story\":" << opt.story
        << ",\"until_stage\":" << opt.until_stage << ",\"final_route\":" << int(opt.final_route)
        << ",\"shot\":" << opt.shot << ",\"measure\":" << opt.measure_every
        << ",\"diversity_cell\":" << opt.diversity_cell << ",\"prefix\":" << !opt.prefix.empty()
        << ",\"boss_follow\":" << opt.boss_follow << ",\"verify\":" << !opt.verify.empty() << "}";
}

void write_accounting(std::ostream &out, const th08::headless::State &st,
                      const ExecutionAccounting &record) {
    out << ",\"calculation_updates\":" << st.calculation_updates
        << ",\"control_updates\":" << record.control_updates
        << ",\"stage_transition_count\":" << record.stage_transition_count
        << ",\"stage_transitions\":[";
    for (int i = 0; i < record.stage_transition_count; ++i) {
        const auto &event = record.stage_transitions[i];
        out << (i ? "," : "") << "{\"frame\":" << event.frame << ",\"from_stage\":" << event.from
            << ",\"to_stage\":" << event.to
            << ",\"calculation_updates_before\":" << event.calculations_before
            << ",\"calculation_updates_after\":" << event.calculations_after << "}";
    }
    out << "]";
}

void write_summary(const char *outcome, const th08::headless::State &st, long frames,
                   const ExecutionAccounting &record = accounting) {
    std::ofstream rep(fs::path(opt.out_dir) / "plan.json");
    rep.exceptions(std::ios::badbit | std::ios::failbit);
    rep << std::setprecision(std::numeric_limits<double>::max_digits10);
    const double secs = std::chrono::duration<double>(Clock::now() - t_start).count();
    rep << "{\"source_revision\":\"02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415\","
        << "\"profile\":\"native-headless-fork-float32\",\"shot\":" << opt.shot << ",\"outcome\":\""
        << outcome << "\",\"stage\":" << opt.stage << ",\"spell\":" << opt.spell
        << ",\"difficulty\":" << opt.difficulty << ",\"seed\":" << opt.seed
        << ",\"frames\":" << frames << ",\"wall_seconds\":" << secs
        << ",\"rollouts\":" << shared->rollouts << ",\"searches\":" << shared->searches
        << ",\"cegar_replans\":" << shared->cegar << ",\"backtracks\":" << shared->backtracks
        << ",\"retries\":" << shared->retries << ",\"releases\":" << shared->releases
        << ",\"quiet_windows\":" << shared->quiet << ",\"model_breaks\":" << shared->model_breaks
        << ",\"max_frame\":" << shared->max_frame << ",\"score\":" << st.score
        << ",\"graze\":" << st.graze << ",\"rng_seed\":" << st.rng_seed
        << ",\"rng_draws\":" << st.rng_draws << ",\"deaths\":" << st.deaths
        << ",\"rollout\":" << opt.rollout << ",\"beam\":" << opt.beam
        << ",\"commit\":" << opt.commit << ",\"checkpoint\":" << opt.checkpoint
        << ",\"story\":" << opt.story << ",\"final_stage\":" << st.stage
        << ",\"final_spell\":" << st.spell
        << ",\"final_spell_active\":" << (st.spell_active ? "true" : "false")
        << ",\"native_update_attempts\":" << shared->native_attempts
        << ",\"native_updates\":" << shared->native_updates
        << ",\"native_calculation_updates\":" << shared->native_calculations
        << ",\"native_control_updates\":" << shared->native_controls
        << ",\"native_stage_transitions\":" << shared->native_stage_transitions
        << ",\"incomplete_native_updates\":" << (shared->native_attempts - shared->native_updates)
        << ",\"rollout_update_attempts\":" << shared->rollout_attempts
        << ",\"rollout_updates\":" << shared->rollout_updates
        << ",\"commit_updates\":" << shared->commit_updates
        << ",\"prefix_requested_frames\":" << prefix_actions.size()
        << ",\"prefix_native_update_attempts\":" << shared->prefix_attempts
        << ",\"prefix_native_updates\":" << shared->prefix_updates
        << ",\"prefix_complete\":" << (shared->prefix_complete ? "true" : "false")
        << ",\"rollout_failures\":" << shared->rollout_failures << ",\"beam_retention\":\""
        << (opt.diversity_cell ? "spatial-focus-bins" : "global-score")
        << "\",\"diversity_selections\":" << shared->diversity_selections
        << ",\"diversity_candidates\":" << shared->diversity_candidates
        << ",\"diversity_bins\":" << shared->diversity_bins
        << ",\"diversity_retained_bins\":" << shared->diversity_retained_bins
        << ",\"diversity_selection_seconds\":" << double(shared->diversity_selection_ns) / 1e9
        << ",\"native_seconds_excluding_file_io\":" << double(shared->native_ns) / 1e9
        << ",\"shared_capacity_bytes\":" << sizeof(Shared) << ",\"max_rollout\":" << kMaxFrames
        << ",\"movement_model\":\"float32-current-speeds\""
        << ",\"proposal_pruning\":\"heuristic-half-pixel-cells\""
        << ",\"acceptance\":\"native-whole-window-or-terminal\""
        << ",\"hazard_probe_geometry\":\"bullet-boxes-only\""
        << ",\"probes\":" << shared->probe_count
        << ",\"probe_rng_diverged\":" << shared->probe_rng_diverged
        << ",\"probe_rng_first_frame_mean\":"
        << (shared->probe_rng_diverged
                ? double(shared->probe_rng_frame_sum) / shared->probe_rng_diverged
                : 0.0);
    for (int b = 0; b < 4; ++b)
        rep << ",\"same_hazards_" << (16 * b + 1) << "_" << (16 * b + 16) << "\":"
            << (shared->probe_total[b] ? double(shared->probe_same[b]) / shared->probe_total[b]
                                       : -1.0);
    rep << ",\"options\":";
    write_options(rep);
    write_hashes(rep);
    write_accounting(rep, st, record);
    rep << "}\n";
    rep.close();
}

void write_result(const char *outcome, const th08::headless::State &st) {
    fs::create_directories(opt.out_dir);
    if (std::string(outcome) == "complete" && opt.spell < 0)
        save_replay();
    std::ofstream tape(fs::path(opt.out_dir) / "plan.actions");
    tape.exceptions(std::ios::badbit | std::ios::failbit);
    for (auto a : history)
        tape << a << '\n';
    tape.close();
    write_summary(outcome, st, long(history.size()));
    shared->report_state = st;
    shared->report_accounting = accounting;
    shared->report_frames = long(history.size());
    shared->report_written = true;
}

// Reconstruct once before checkpoint holders fork. This episode may replace only
// the continuation: game objects, RNG and latched input all carry natively, while
// planner tie RNG/quiet windows start fresh rather than reconstructing a search.
int replay_prefix() {
    if (opt.prefix.empty())
        return -1;
    auto st = session->state();
    for (std::size_t i = 0; i < prefix_actions.size(); ++i) {
        if (frame_now >= opt.frame_limit ||
            std::chrono::duration<double>(Clock::now() - t_start).count() > opt.seconds) {
            write_result("budget", st);
            return kBudget;
        }
        st = native_step(prefix_actions[i], NativeUse::Prefix);
        history.push_back(prefix_actions[i]);
        ++frame_now;
        if (frame_now > shared->max_frame)
            shared->max_frame = frame_now;
        if (st.player_state == 2 || st.deaths != 0) {
            std::fprintf(stderr, "prefix rejected: death at update %ld\n", frame_now);
            write_result("prefix_rejected", st);
            return kPrefixRejected;
        }
        const auto boundary =
            th08::plan::prefix_boundary(prefix_actions.size(), i + 1, is_complete(st));
        if (boundary == th08::plan::PrefixBoundary::TrailingTerminal) {
            std::fprintf(stderr, "prefix rejected: actions follow terminal update %ld\n",
                         frame_now);
            write_result("prefix_rejected", st);
            return kPrefixRejected;
        }
        if (boundary == th08::plan::PrefixBoundary::Complete) {
            shared->prefix_complete = true;
            write_result("complete", st);
            return kOk;
        }
    }
    shared->prefix_complete = true;
    return -1;
}

// ---------------------------------------------------------------- checkpoints
void on_release(int) {
    _exit(kReleased);
}
// The calling worker becomes a holder; returns only in a (new) worker child.
void checkpoint() {
    const int my_depth = ++depth;
    // Install before fork: a fast child can release this holder immediately.
    struct sigaction release_action {};
    release_action.sa_handler = on_release;
    sigemptyset(&release_action.sa_mask);
    if (sigaction(SIGUSR1, &release_action, nullptr) < 0)
        throw std::runtime_error("checkpoint signal handler failed");
    for (int attempt = 0; attempt <= opt.attempts; ++attempt) {
        const pid_t pid = fork();
        if (pid < 0)
            throw std::runtime_error("checkpoint fork failed");
        if (pid == 0) {
            struct sigaction worker_action {};
            worker_action.sa_handler = SIG_DFL;
            sigemptyset(&worker_action.sa_mask);
            if (sigaction(SIGUSR1, &worker_action, nullptr) < 0)
                throw std::runtime_error("worker signal handler failed");
            variant = attempt;
            if (attempt) {
                ++shared->retries;
            }
            rng_state ^= std::uint64_t(attempt) * 0x9E3779B97F4A7C15ULL;
            // Odd retries withhold shots up to and past the remembered failure, so
            // one source of RNG coupling is reduced. Native verification remains
            // mandatory because movement/focus/graze can still change the future.
            if (opt.quiet_retries && attempt % 2 == 1)
                quiet_until = std::max(quiet_until, shared->future_valid
                                                        ? shared->future_base + 2L * opt.rollout
                                                        : frame_now + 4L * opt.rollout);
            const int n = my_depth;
            const int slot = n % th08::plan::holder_slots;
            shared->depth_pids[slot] = getppid();
            shared->holder_depth[slot] = n;
            shared->released[slot] = false;
            // Release holders that are too old to be useful backtrack targets.
            const int old = n - opt.keep;
            const int old_slot = old >= 1 ? old % th08::plan::holder_slots : 0;
            if (old >= 1 && shared->holder_depth[old_slot] == old && !shared->released[old_slot]) {
                shared->released[old_slot] = true;
                if (shared->depth_pids[old_slot] <= 0 ||
                    kill(shared->depth_pids[old_slot], SIGUSR1) < 0)
                    throw std::runtime_error("checkpoint holder release failed");
                ++shared->releases;
            }
            return;
        }
        const auto child = th08::plan::wait_for_child(pid);
        if (!child.waited || !WIFEXITED(child.status)) {
            shared->fatal_error = true;
            _exit(kRuntimeError);
        }
        const int code = WEXITSTATUS(child.status);
        if (code != kBacktrack)
            _exit(code); // success, budget or a hard error
        ++shared->backtracks;
    }
    _exit(kBacktrack);
}

int run_worker() {
    auto st = session->state();
    Speeds sp = read_speeds();
    std::vector<std::uint16_t> nominal(opt.rollout, std::uint16_t(st.latched_input & 0xF4));
    long last_checkpoint = frame_now;
    int shot_off = 0;
    auto expand = [&](const std::vector<std::uint16_t> &moves, long base) {
        std::vector<std::uint16_t> plan(opt.rollout);
        for (int k = 0; k < opt.rollout; ++k) {
            const std::uint16_t m = moves.empty()
                                        ? std::uint16_t(4)
                                        : moves[std::min<std::size_t>(k, moves.size() - 1)];
            plan[k] = decorate(m, base + k + 1, shot_off - k);
        }
        return plan;
    };
    for (;;) {
        if (frame_now >= opt.frame_limit ||
            std::chrono::duration<double>(Clock::now() - t_start).count() > opt.seconds) {
            write_result("budget", st);
            return kBudget;
        }
        if (opt.quiet_retries && variant && frame_now == last_checkpoint && shot_off == 0)
            shot_off = 3 * variant;
        sp = read_speeds();
        auto plan = expand(nominal, frame_now);
        const auto nominal_plan = plan;
        rollout(plan, 0);
        const bool nominal_survived = th08::plan::verified_window(shared->tapes[0], opt.rollout);
        if (opt.measure_every && shared->rollouts % opt.measure_every == 0) {
            // Same start state, a different player path (constant opposite drift).
            const Tape &a = shared->tapes[0];
            const std::uint16_t drift = (st.x < 192.f ? 128 : 64) | 4;
            std::vector<std::uint16_t> alt(opt.rollout);
            for (int q = 0; q < opt.rollout; ++q)
                alt[q] = decorate(drift, frame_now + q + 1, 0);
            rollout(alt, kMaxTapes - 1);
            const Tape &b = shared->tapes[kMaxTapes - 1];
            const int n =
                std::min(a.death ? a.death - 1 : a.frames, b.death ? b.death - 1 : b.frames);
            int rng_div = 0;
            for (int j = 1; j <= n; ++j) {
                if (!rng_div && (a.f[j].state.rng_seed != b.f[j].state.rng_seed ||
                                 a.f[j].state.rng_draws != b.f[j].state.rng_draws))
                    rng_div = j;
                std::vector<std::array<float, 4>> keys(a.f[j].nb);
                for (int i = 0; i < a.f[j].nb; ++i)
                    keys[i] = {a.f[j].b[i].x0, a.f[j].b[i].y0, a.f[j].b[i].x1, a.f[j].b[i].y1};
                std::sort(keys.begin(), keys.end());
                long same = 0;
                for (int i = 0; i < b.f[j].nb; ++i) {
                    const std::array<float, 4> k2{b.f[j].b[i].x0, b.f[j].b[i].y0, b.f[j].b[i].x1,
                                                  b.f[j].b[i].y1};
                    same += std::binary_search(keys.begin(), keys.end(), k2);
                }
                // The legacy four report buckets cover only updates 1..64;
                // longer rollouts must not relabel later observations as 49..64.
                if (j <= 64) {
                    const int bucket = (j - 1) / 16;
                    shared->probe_total[bucket] += b.f[j].nb;
                    shared->probe_same[bucket] += same;
                }
            }
            ++shared->probe_count;
            if (rng_div)
                ++shared->probe_rng_diverged, shared->probe_rng_frame_sum += rng_div;
        }
        std::vector<int> slots{0};
        int accepted = -1;
        int needed = std::min(opt.rollout - 1, opt.commit + 24);
        std::vector<std::uint16_t> best_moves = nominal;
        for (int it = 0; it <= opt.cegar; ++it) {
            const Tape &t0 = shared->tapes[slots.back()];
            // Model check against native positions of the tape's own plan.
            int valid = t0.frames;
            {
                float x = st.x, y = st.y;
                std::uint16_t lat = st.latched_input;
                for (int j = 1; j <= t0.frames; ++j) {
                    move(sp, x, y, lat);
                    if (t0.death && j >= t0.death)
                        break;
                    if (!th08::plan::same_float(x, t0.f[j].state.x) ||
                        !th08::plan::same_float(y, t0.f[j].state.y)) {
                        valid = j - 1;
                        ++shared->model_breaks;
                        break;
                    }
                    lat = t0.f[j].input;
                }
            }
            const bool calm = it == 0 && !t0.death && valid >= needed;
            if (th08::plan::verified_window(t0, opt.rollout) &&
                (valid < needed || t0.status == RolloutStatus::Terminal)) {
                accepted = slots.back(); // uncontrollable/terminal window: trust the native result
                break;
            }
            const int horizon = std::min(valid, t0.frames);
            if (horizon < 2)
                break;
            float pref_x =
                192.f + (variant ? (variant % 2 ? -1.f : 1.f) * 48.f * ((variant + 1) / 2) : 0.f);
            const FrameTape &f1 = t0.f[1];
            float area = 0;
            for (int i = 0; i < f1.nr; ++i) {
                const float a = (f1.r[i].x1 - f1.r[i].x0) * (f1.r[i].y1 - f1.r[i].y0);
                if (a > area && f1.r[i].y1 < 300)
                    area = a, pref_x = 192.f +
                                       opt.boss_follow * ((f1.r[i].x0 + f1.r[i].x1) / 2 - 192.f) +
                                       (variant ? (variant % 2 ? -32.f : 32.f) : 0.f);
            }
            pref_x = std::clamp(pref_x, 48.f, 336.f);
            const int width = calm ? std::max(48, opt.beam / 8) : opt.beam;
            int f_off = 0, f_until = 0;
            if (shared->future_valid && shared->future_base >= frame_now) {
                f_off =
                    int(shared->future_base - frame_now); // search frame j <-> future.f[j - f_off]
                f_until = int(std::min(long(f_off) + shared->future.frames, long(horizon) + 64));
                if (f_until <= horizon)
                    f_until = 0;
            }
            auto res = search(slots, horizon, width, pref_x, 400.f, sp, st.latched_input, st.x,
                              st.y, f_off, f_until);
            if (res.depth < needed && f_until) // learned future too strict: own evidence only
                res =
                    search(slots, horizon, width, pref_x, 400.f, sp, st.latched_input, st.x, st.y);
            if (res.depth < needed && slots.size() > 1) // union too strict: latest evidence only
                res = search({slots.back()}, horizon, width, pref_x, 400.f, sp, st.latched_input,
                             st.x, st.y);
            if (res.depth < needed)
                break;
            const int slot = std::min<int>(int(slots.size()), kMaxTapes - 1);
            // Native verification. A death anywhere in the window is a counterexample;
            // on the last iteration re-rank diverse model leaves by native rollouts.
            const bool last_try = it == opt.cegar;
            int best_death = -1;
            std::vector<std::uint16_t> best_alt;
            std::vector<std::vector<std::uint16_t>> cands{res.moves};
            if (last_try) {
                cands = res.alts;
                // Bait-then-escape: keep the nominal path (so aimed/tracking hazards
                // resolve exactly as recorded) for T updates, then search an escape.
                for (int anchor_len = 8; anchor_len <= 48 && !nominal.empty(); anchor_len += 8) {
                    auto esc = search({0}, horizon, opt.beam, pref_x, 400.f, sp, st.latched_input,
                                      st.x, st.y, 0, 0, &nominal, anchor_len);
                    if (esc.depth >= horizon - 1)
                        cands.push_back(esc.moves);
                }
            }
            if (cands.empty())
                break;
            std::vector<std::uint16_t> last_plan;
            for (const auto &cand : cands) {
                auto cplan = expand(cand, frame_now);
                rollout(cplan, slot);
                last_plan = cplan;
                const Tape &tv = shared->tapes[slot];
                const int survived = tv.death ? tv.death : 1 << 20;
                if (survived > best_death) {
                    best_death = survived, best_alt = cand;
                    plan = cplan;
                }
                if (th08::plan::verified_window(tv, opt.rollout))
                    break;
            }
            if (last_plan != plan)
                rollout(plan, slot); // Charge the repeated verification and retain its actual tape.
            best_moves = best_alt;
            if (int(best_moves.size()) > opt.rollout)
                best_moves.resize(opt.rollout);
            const Tape &tv = shared->tapes[slot];
            if (th08::plan::verified_window(tv, opt.rollout)) {
                accepted = slot;
                break;
            }
            ++shared->cegar;
            if (int(slots.size()) < kMaxTapes - 1)
                slots.push_back(slot);
            else
                slots.back() = slot;
        }
        if (opt.quiet_retries && accepted < 0 && quiet_until < frame_now + opt.rollout) {
            // Quiet input reduces one source of RNG coupling. Each candidate still
            // owns a native future and must survive the entire requested window.
            const long saved = quiet_until;
            quiet_until = frame_now + opt.rollout + 32;
            const auto qplan = expand(nominal, frame_now);
            rollout(qplan, 0);
            const Tape &tq = shared->tapes[0];
            if (th08::plan::verified_window(tq, opt.rollout)) {
                accepted = 0, plan = qplan, best_moves = nominal;
            } else {
                auto res = search({0}, std::max(2, tq.frames), opt.beam, 192.f, 400.f, sp,
                                  st.latched_input, st.x, st.y);
                for (const auto &cand : res.alts) {
                    auto cplan = expand(cand, frame_now);
                    rollout(cplan, 1);
                    if (th08::plan::verified_window(shared->tapes[1], opt.rollout)) {
                        accepted = 1, plan = cplan, best_moves = cand;
                        break;
                    }
                }
            }
            if (accepted < 0)
                quiet_until = saved;
            else
                ++shared->quiet;
        }
        if (accepted < 0 && nominal_survived) {
            // The beam merges cells and may drop the nominal path; if the native
            // nominal survived the full window, restore its tape after quiet probes.
            rollout(nominal_plan, 0);
            if (!th08::plan::verified_window(shared->tapes[0], opt.rollout))
                throw std::runtime_error("repeated nominal rollout disagreed");
            accepted = 0;
            plan = nominal_plan;
            best_moves = nominal;
        }
        if (accepted < 0) {
            {
                const Tape &lt = shared->tapes[slots.back()];
                std::ofstream bl(fs::path(opt.out_dir) / "backtracks.tsv", std::ios::app);
                bl << frame_now << '\t' << st.stage << '\t' << st.spell << '\t' << depth << '\t'
                   << variant << '\t' << lt.death << '\t' << lt.death_kind << '\n';
                if (std::getenv("TH08_PLAN_DUMP")) { // geometry of the fatal update for diagnosis
                    std::ofstream dump(fs::path(opt.out_dir) /
                                       ("fatal_" + std::to_string(frame_now) + ".txt"));
                    dump << "player " << st.x << ' ' << st.y << " half " << sp.half.x << ' '
                         << sp.half.y << '\n';
                    for (int j = 1; j <= lt.frames; ++j) {
                        const auto &fj = lt.f[j];
                        dump << "frame " << j << " px " << fj.state.x << " py " << fj.state.y
                             << " bullets " << fj.nb << " lasers " << fj.nl << " regions " << fj.nr
                             << '\n';
                        for (int i = 0; i < fj.nl; ++i)
                            dump << "  laser c " << fj.l[i].cx << ' ' << fj.l[i].cy << " size "
                                 << fj.l[i].w << ' ' << fj.l[i].h << " origin " << fj.l[i].ox << ' '
                                 << fj.l[i].oy << " angle " << fj.l[i].angle << '\n';
                    }
                }
            }
            // Remember where and what killed this branch for the retrying ancestor.
            std::memcpy(&shared->future, &shared->tapes[slots.back()], sizeof(Tape));
            shared->future_base = frame_now;
            shared->future_valid = true;
            write_result("exhausted", st);
            return kBacktrack;
        }
        if (shared->future_valid && frame_now > shared->future_base + shared->future.frames)
            shared->future_valid = false;
        // Commit a verified prefix in this process; fork made it deterministic.
        const Tape &ta = shared->tapes[accepted];
        if (!th08::plan::verified_window(ta, opt.rollout))
            throw std::logic_error("attempted to commit an unverified rollout");
        const int k = std::min({opt.commit, ta.frames, int(opt.frame_limit - frame_now)});
        for (int j = 1; j <= k; ++j) {
            st = native_step(ta.f[j].input, NativeUse::Commit);
            history.push_back(ta.f[j].input);
            ++frame_now;
            if (frame_now > shared->max_frame)
                shared->max_frame = frame_now;
            const bool complete = is_complete(st);
            if (!th08::plan::same_observation(st, ta.f[j].state) || st.player_state == 2 ||
                complete != ta.f[j].complete) {
                std::fprintf(stderr, "determinism violation at %ld\n", frame_now);
                write_result("determinism_violation", st);
                return 1;
            }
            if (complete) {
                write_result("complete", st);
                return kOk;
            }
        }
        ++shared->commits;
        if (frame_now > shared->max_frame)
            shared->max_frame = frame_now;
        if (frame_now % 1200 < k) { // lightweight progress for long unattended runs
            std::ofstream pr(fs::path(opt.out_dir) / "progress.json");
            pr << "{\"frame\":" << frame_now << ",\"stage\":" << st.stage
               << ",\"score\":" << st.score << ",\"graze\":" << st.graze << ",\"depth\":" << depth
               << ",\"variant\":" << variant << ",\"backtracks\":" << shared->backtracks
               << ",\"cegar\":" << shared->cegar << ",\"rollouts\":" << shared->rollouts
               << ",\"elapsed\":" << std::chrono::duration<double>(Clock::now() - t_start).count()
               << "}\n";
        }
        shot_off = std::max(0, shot_off - k);
        nominal.assign(best_moves.begin() + std::min<std::size_t>(k, best_moves.size()),
                       best_moves.end());
        if (frame_now - last_checkpoint >= opt.checkpoint) {
            checkpoint(); // returns in the child; parent never returns
            last_checkpoint = frame_now;
            st = session->state();
        }
    }
}

} // namespace

int execute(int argc, char **argv) {
    opt = th08::plan::parse_options(argc, argv);
    opt.dat = fs::absolute(opt.dat).string();
    opt.out_dir = fs::absolute(opt.out_dir).string();
    if (!opt.verify.empty())
        opt.verify = fs::absolute(opt.verify).string();
    if (!opt.prefix.empty())
        opt.prefix = fs::absolute(opt.prefix).string();
    t_start = Clock::now();
    void *mem =
        mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
        throw std::runtime_error("planner shared allocation failed");
    shared = new (mem) Shared();
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free &&
                      std::atomic<long>::is_always_lock_free &&
                      std::atomic<bool>::is_always_lock_free,
                  "shared fork counters require lock-free atomics");
    std::fprintf(stderr, "planner shared capacity: %zu bytes, maximum rollout: %d updates\n",
                 sizeof(Shared), kMaxFrames);
    fs::create_directories(opt.out_dir);
    binary_sha256 = file_sha256("/proc/self/exe", "executing binary");
    dat_sha256 = file_sha256(opt.dat, "DAT");
    if (!opt.prefix.empty()) {
        th08::plan::PrefixParser parser(std::size_t(opt.frame_limit));
        prefix_sha256 = file_sha256(opt.prefix, "prefix", &parser);
        prefix_actions = parser.finish();
    }
    char scratch[] = "/tmp/th08-plan-XXXXXX";
    if (!mkdtemp(scratch))
        throw std::runtime_error("planner scratch creation failed");
    scratch_path = scratch;
    fs::current_path(scratch);
    th08::headless::Config config;
    config.dat_path = opt.dat.c_str();
    config.stage = opt.stage;
    config.spell = opt.spell;
    config.difficulty = opt.difficulty;
    config.seed = std::uint16_t(opt.seed);
    config.story = opt.story;
    config.shot = opt.shot;
    config.unlock_final_routes = opt.final_route != th08::plan::FinalRoute::Natural;
    if (!opt.verify.empty()) {
        // Fresh-process strict replay of a committed tape. Only non-spell runs
        // export a .rpy; spell-practice receipts intentionally leave rpy empty.
        // Hash and decode the same bytes; a receipt must identify the exact
        // immutable action input, independently of its private filesystem path.
        th08::plan::PrefixParser parser(std::numeric_limits<std::size_t>::max());
        actions_sha256 = file_sha256(opt.verify, "verification actions", &parser);
        const auto tape = parser.finish();
        session = new th08::headless::Session(config);
        th08::headless::State st = session->state();
        long deaths = 0, done_at = -1;
        for (std::size_t i = 0; i < tape.size(); ++i) {
            st = native_step(tape[i], NativeUse::Verify);
            if (st.player_state == 2)
                ++deaths;
            if (done_at < 0 && is_complete(st))
                done_at = long(i + 1);
        }
        const bool ok = deaths == 0 && st.deaths == 0 && done_at == long(tape.size());
        std::string rpy;
        if (ok && opt.spell < 0) {
            rpy = (fs::path(opt.out_dir) / "th8_verify.rpy").string();
            save_replay("th8_verify.rpy");
        }
        std::ofstream rep(fs::path(opt.out_dir) / "verify.json");
        rep.exceptions(std::ios::badbit | std::ios::failbit);
        rep << "{\"source_revision\":\"02ff72f29e7b4928f21c5a2fb0f57ffa0cec5415\","
            << "\"verified\":" << (ok ? "true" : "false") << ",\"frames\":" << tape.size()
            << ",\"complete_at\":" << done_at << ",\"death_updates\":" << deaths
            << ",\"score\":" << st.score << ",\"stage\":" << st.stage
            << ",\"final_spell\":" << st.spell
            << ",\"final_spell_active\":" << (st.spell_active ? "true" : "false")
            << ",\"rng_seed\":" << st.rng_seed << ",\"rng_draws\":" << st.rng_draws
            << ",\"native_update_attempts\":" << shared->native_attempts
            << ",\"native_updates\":" << shared->native_updates
            << ",\"native_calculation_updates\":" << shared->native_calculations
            << ",\"native_control_updates\":" << shared->native_controls
            << ",\"native_stage_transitions\":" << shared->native_stage_transitions
            << ",\"native_seconds_excluding_file_io\":" << double(shared->native_ns) / 1e9
            << ",\"rpy\":\"" << (rpy.empty() ? "" : "th8_verify.rpy") << "\",\"options\":";
        write_options(rep);
        write_hashes(rep);
        write_accounting(rep, st, accounting);
        rep << "}\n";
        rep.close();
        std::printf("verify %s\n", ok ? "ok" : "FAILED");
        std::fflush(stdout);
        cleanup_scratch();
        _exit(ok ? 0 : 2); // skip manager teardown (TitleScreen threads may follow)
    }
    // Root: subreaper that only collects the chain's verdict.
    if (prctl(PR_SET_CHILD_SUBREAPER, 1) < 0)
        throw std::runtime_error("planner subreaper setup failed");
    const pid_t first = fork();
    if (first < 0)
        throw std::runtime_error("planner initial fork failed");
    if (first == 0) {
        try {
            session = new th08::headless::Session(config);
            const int prefix_code = replay_prefix();
            if (prefix_code >= 0)
                _exit(prefix_code);
            checkpoint(); // depth-1 holder above the first worker
            _exit(run_worker());
        } catch (const std::exception &error) {
            std::fprintf(stderr, "planner runtime error: %s\n", error.what());
        } catch (...) {
            std::fprintf(stderr, "planner runtime error: unknown exception\n");
        }
        shared->fatal_error = true;
        try {
            write_result("runtime_error", session ? session->state() : th08::headless::State{});
        } catch (...) {
            // The root still reports the hard failure if writing is unavailable.
        }
        _exit(kRuntimeError);
    }
    int verdict = -1, status = 0;
    for (;;) {
        const pid_t pid = waitpid(-1, &status, 0);
        if (pid < 0) {
            if (errno == EINTR)
                continue;
            if (errno != ECHILD)
                shared->fatal_error = true;
            break;
        }
        const int code = WIFEXITED(status) ? WEXITSTATUS(status) : kRuntimeError;
        if (code == kReleased)
            continue;
        if (code != kOk && code != kBacktrack && code != kBudget && code != kPrefixRejected) {
            shared->fatal_error = true;
            verdict = kRuntimeError;
        } else if (verdict < 0 || (code == kOk && verdict != kRuntimeError))
            verdict = code;
    }
    cleanup_scratch();
    if (shared->fatal_error || verdict < 0)
        verdict = kRuntimeError;
    const char *outcome = verdict == kOk               ? "complete"
                          : verdict == kPrefixRejected ? "prefix_rejected"
                          : verdict == kBacktrack      ? "exhausted"
                          : verdict == kBudget         ? "budget"
                                                       : "runtime_error";
    if (!shared->report_written)
        write_result(outcome, {});
    // All holders and rollout children have now exited; include their final
    // aggregate costs even when the last worker's report preceded reaping.
    write_summary(outcome, shared->report_state, shared->report_frames, shared->report_accounting);
    std::printf("verdict %d\n", verdict);
    return verdict;
}

int main(int argc, char **argv) {
    try {
        return execute(argc, argv);
    } catch (const std::invalid_argument &error) {
        std::fprintf(stderr, "invalid planner arguments: %s\n", error.what());
        if (shared && !opt.verify.empty()) {
            try {
                std::ofstream rep(fs::path(opt.out_dir) / "verify.json");
                rep << "{\"verified\":false,\"outcome\":\"invalid_input\"";
                write_hashes(rep);
                rep << "}\n";
            } catch (...) {
            }
        } else if (shared && !opt.prefix.empty()) {
            try {
                write_summary("prefix_rejected", {}, 0);
            } catch (...) {
            }
        }
        cleanup_scratch();
        return 2;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "planner runtime error: %s\n", error.what());
        if (shared && !opt.out_dir.empty()) {
            shared->fatal_error = true;
            try {
                if (!opt.verify.empty()) {
                    std::ofstream rep(fs::path(opt.out_dir) / "verify.json");
                    rep << "{\"verified\":false,\"outcome\":\"runtime_error\",\"native_update_"
                           "attempts\":"
                        << shared->native_attempts
                        << ",\"native_updates\":" << shared->native_updates;
                    write_hashes(rep);
                    rep << "}\n";
                } else
                    write_result("runtime_error", {});
            } catch (...) {
            }
        }
        cleanup_scratch();
        return kRuntimeError;
    }
}
