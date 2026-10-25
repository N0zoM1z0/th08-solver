#include "th08/shadow_recorded.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Supervisor.hpp"
#include "ZunMath.hpp"
#include "modern/headless/runtime.hpp"
#include "th08/plan_beam.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_dialogue.hpp"
#include "th08/shadow_native.hpp"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <new>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
namespace th08::shadow {
namespace {
namespace fs = std::filesystem;
namespace h = th08::headless;
using Clock = std::chrono::steady_clock;
using th08::Float3;
using th08::plan::RolloutStatus;
// One bounded recording per refinement; no pointer-rich Native state is copied.
constexpr int kMaxFrames = 128, kMaxBullets = 2048, kMaxLasers = 640, kMaxRegions = 512,
              kMaxTapes = 4;
struct Box {
    float x0, y0, x1, y1; // native inclusive bounds (min, max)
};
struct Laser {
    float cx, cy, w, h, ox, oy, angle; // CalcLaserHitbox(position, size, origin, angle)
};
struct FrameTape {
    int nb, nl, nr;
    Box b[kMaxBullets];
    Laser l[kMaxLasers];
    Box r[kMaxRegions];
};
struct Tape : th08::plan::RolloutSummary {
    FrameTape f[kMaxFrames + 1]; // 1-based
};

struct SearchResult {
    std::vector<std::uint16_t> moves;
    std::vector<std::vector<std::uint16_t>> alts;
    int depth = 0;
};

struct Counters {
    std::uint64_t attempts = 0, completed = 0, native_ns = 0, file_io_ns = 0, capture_ns = 0;
};
class TapeInputs final : public RolloutGuidance {
    h::Session &world_;
    const Packet origin_;
    const std::vector<std::uint16_t> &inputs_;
    bool dialogue_;
    Packet current_;

  public:
    TapeInputs(h::Session &w, const Packet &o, const std::vector<std::uint16_t> &in, bool dialogue)
        : world_(w), origin_(o), inputs_(in), dialogue_(dialogue) {}
    void observe(const Packet &p) override {
        current_ = p;
    }
    std::uint16_t next_input() override {
        if (dialogue_ && world_.state().dialogue_present)
            return dialogue_input(std::uint16_t(current_.input), current_.sequence + 1);
        return inputs_.at(std::size_t(current_.sequence - origin_.sequence));
    }
};
} // namespace
struct RecordedPlanner::Impl {
    struct Storage {
        Tape tapes[kMaxTapes];
        Counters counters[kMaxTapes];
    } * shared;
    struct Controls {
        int diversity_cell = 16, spell = -1;
    } opt;
    RecordedOptions options;
    RecordedWork work;
    Clock::time_point deadline;
    using Node = th08::plan::BeamNode;
    static constexpr int kSeenBits = 18, kSeenSize = 1 << kSeenBits;
    static constexpr std::uint16_t kDirs[9] = {0, 16, 32, 64, 128, 80, 144, 96, 160};
    std::vector<std::uint32_t> seen_key = std::vector<std::uint32_t>(kSeenSize),
                               seen_gen = std::vector<std::uint32_t>(kSeenSize);
    std::uint32_t generation = 0;
    // Proposal randomness is independent of every Native gameplay RNG consumer.
    std::uint64_t rng_state = 88172645463325252ULL;
    std::uint64_t xrand() {
        rng_state ^= rng_state << 13;
        rng_state ^= rng_state >> 7;
        rng_state ^= rng_state << 17;
        return rng_state;
    }
    explicit Impl(RecordedOptions cfg) : options(cfg) {
        if (cfg.horizon < 2 || cfg.horizon > kMaxFrames || cfg.beam < 1 || cfg.beam > 8192 ||
            cfg.refinements < 1 || cfg.refinements > kMaxTapes || cfg.diversity_cell < 0 ||
            cfg.diversity_cell > 512 || cfg.verification_horizon > cfg.horizon ||
            cfg.verification_horizon == 1)
            throw std::runtime_error("invalid_recorded_planner_options");
        opt.diversity_cell = cfg.diversity_cell;
        if (!options.verification_horizon)
            options.verification_horizon = options.horizon;
        void *mem = mmap(nullptr, sizeof(Storage), PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (mem == MAP_FAILED)
            throw std::runtime_error("recorded_planner_allocation_failed");
        shared = new (mem) Storage;
    }
    ~Impl() {
        munmap(shared, sizeof(Storage));
    }
    unsigned remaining() const {
        const auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
        return ms > 0 ? unsigned(ms) : 0;
    }
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
    static constexpr float kCell = 16.f, kGx0 = -64.f, kGy0 = -64.f;
    static constexpr int kGW = 34, kGH = 38;
    static constexpr float kMargin = 1.5f;
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

    bool record_frame(FrameTape &f) {
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
        return true;
    }
    SearchResult search(const std::vector<int> &slots, int horizon, int beam_width, float pref_x,
                        float pref_y, const Speeds &sp, std::uint16_t latched, float x0, float y0,
                        const std::vector<std::uint16_t> &anchor) {
        ++work.searches;
        const int own = horizon;
        std::vector<std::vector<FrameIndex>> ix(slots.size(), std::vector<FrameIndex>(horizon + 1));
        for (std::size_t s = 0; s < slots.size(); ++s)
            for (int j = 1; j <= own && j <= shared->tapes[slots[s]].frames; ++j)
                build_index(shared->tapes[slots[s]].f[j], sp.half, ix[s][j]);
        auto test = [&](int j, float x, float y, bool margin) {
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
            if (Clock::now() >= deadline) {
                ++work.search_timeouts;
                return {};
            }
            auto &cur = layers[j];
            auto &next = layers[j + 1];
            ++generation;
            const bool branch = j < 16 || (j % 2) == 0; // coarser decisions far ahead
            for (int pi = 0; pi < int(cur.size()); ++pi) {
                if ((pi % 16) == 0 && Clock::now() >= deadline) {
                    ++work.search_timeouts;
                    return {};
                }
                const Node &p = cur[pi];
                for (int f = 0; f < 2; ++f)
                    for (int d = 0; d < 9; ++d) {
                        std::uint16_t a = std::uint16_t(kDirs[d] | (f ? 4 : 0));
                        if (j - 1 <
                            int(anchor.size())) { // follow the nominal until the anchor ends
                            if (a != (anchor[std::size_t(j - 1)] & 0xF4))
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
                th08::plan::retain_spatial_beam(next, std::size_t(beam_width), opt.diversity_cell);
            } else {
                if (int(next.size()) > beam_width) {
                    std::nth_element(
                        next.begin(), next.begin() + beam_width, next.end(),
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

    BranchStatus forecast(h::Session &world, const Packet &origin,
                          const std::vector<std::uint16_t> &inputs, std::size_t forced, int slot,
                          Boundary boundary) {
        auto &t = shared->tapes[slot];
        t.frames = 0;
        t.death = 0;
        t.status = RolloutStatus::Running;
        auto &counter = shared->counters[slot];
        counter = {};
        ++work.forecasts;
        char scratch[] = "/tmp/th08-recorded-branch-XXXXXX";
        if (!mkdtemp(scratch))
            throw std::runtime_error("recorded_scratch_failed");
        struct Cleanup {
            fs::path directory;
            ~Cleanup() {
                std::error_code ignored;
                fs::remove_all(directory, ignored);
            }
        } cleanup{scratch};
        for (const char *name : {"score.dat", "th08.cfg"})
            if (fs::is_regular_file(name))
                fs::copy_file(name, cleanup.directory / name);
        const auto parent = getpid();
        const auto child = fork();
        if (child < 0)
            throw std::runtime_error("recorded_fork_failed");
        if (!child) {
            if (prctl(PR_SET_PDEATHSIG, SIGKILL) || getppid() != parent)
                _exit(2);
            try {
                fs::current_path(cleanup.directory);
                h::record_bullet_boxes = true;
                const float initial_deaths = world.state().deaths;
                bool spell_seen = world.state().spell_active;
                std::uint16_t actual = std::uint16_t(origin.input);
                for (unsigned i = 0; i < inputs.size(); ++i) {
                    auto input = inputs[i];
                    if (i >= forced && options.advance_dialogue && world.state().dialogue_present)
                        input = dialogue_input(actual, origin.sequence + i + 1);
                    ++counter.attempts;
                    const auto began = Clock::now();
                    const auto io_before = world.file_io_time_ns();
                    h::State st;
                    try {
                        st = world.step(input);
                    } catch (...) {
                        const auto io = world.file_io_time_ns() - io_before;
                        counter.file_io_ns += io;
                        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                            Clock::now() - began)
                                            .count();
                        counter.native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
                        throw;
                    }
                    const auto ns =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - began)
                            .count();
                    const auto io = world.file_io_time_ns() - io_before;
                    counter.file_io_ns += io;
                    counter.native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
                    ++counter.completed;
                    const auto capture_began = Clock::now();
                    if (!record_frame(t.f[i + 1])) {
                        t.status = RolloutStatus::Capacity;
                        _exit(1);
                    }
                    counter.capture_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(
                                              Clock::now() - capture_began)
                                              .count();
                    t.frames = int(i + 1);
                    actual = input;
                    if (st.player_state == 2 || st.deaths != initial_deaths) {
                        t.death = int(i + 1);
                        t.status = RolloutStatus::Death;
                        break;
                    }
                    spell_seen = spell_seen || st.spell_active;
                    if (stage_terminal(boundary.story, st) ||
                        (boundary.spell >= 0 && spell_seen && !st.spell_active)) {
                        t.status = RolloutStatus::Terminal;
                        break;
                    }
                    if (i + 1 == inputs.size())
                        t.status = RolloutStatus::Horizon;
                }
            } catch (...) {
                t.status = RolloutStatus::StepError;
                _exit(1);
            }
            _exit(0);
        }
        int status = 0;
        bool timed_out = false, wait_failed = false;
        for (;;) {
            const auto result = waitpid(child, &status, WNOHANG);
            if (result == child)
                break;
            if (result < 0) {
                if (errno == EINTR)
                    continue;
                wait_failed = true;
                break;
            }
            if (Clock::now() >= deadline) {
                timed_out = true;
                break;
            }
            usleep(500);
        }
        if (timed_out || wait_failed) {
            kill(child, SIGKILL);
            while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
            }
        }
        work.attempts += counter.attempts;
        work.completed += counter.completed;
        work.charged += (timed_out || wait_failed) ? inputs.size() : counter.attempts;
        work.native_ns += counter.native_ns;
        work.file_io_ns += counter.file_io_ns;
        work.capture_ns += counter.capture_ns;
        if (timed_out) {
            ++work.forecast_timeouts;
            return BranchStatus::Timeout;
        }
        if (wait_failed || !WIFEXITED(status) || WEXITSTATUS(status) ||
            th08::plan::failed_rollout(t, int(inputs.size())))
            return BranchStatus::Fault;
        work.forecast_deaths += t.status == RolloutStatus::Death;
        return BranchStatus::Horizon;
    }
    std::vector<std::vector<std::uint16_t>> proposals(h::Session &world, const Packet &origin,
                                                      const std::vector<std::uint16_t> &lead,
                                                      const std::vector<int> &slots, unsigned goal,
                                                      std::uint64_t goal_seed) {
        int depth = 0;
        for (int slot : slots)
            depth = std::max(depth, shared->tapes[slot].frames);
        const auto speeds = read_speeds();
        const auto state = world.state();
        // The same fixed field regions apply to every stage, spell, seed and team.
        constexpr float goals[9][2] = {{192, 400}, {48, 400}, {336, 400}, {48, 240}, {192, 240},
                                       {336, 240}, {48, 96},  {192, 96},  {336, 96}};
        const auto primary_rng = rng_state;
        if (goal)
            rng_state = goal_seed ^ (std::uint64_t(goal) * 0x9e3779b97f4a7c15ULL);
        const auto began = Clock::now();
        auto result = search(slots, depth, int(options.beam), goals[goal][0], goals[goal][1],
                             speeds, state.latched_input, state.x, state.y, lead);
        work.search_ns +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - began).count();
        if (goal)
            rng_state = primary_rng;
        std::vector<std::vector<std::uint16_t>> output;
        if (result.depth <= int(lead.size() + 1))
            return output;
        auto paths = goal ? std::vector<std::vector<std::uint16_t>>{result.moves} : result.alts;
        for (const auto &moves : paths) {
            auto inputs = lead;
            for (unsigned i = lead.size(); i < options.horizon; ++i) {
                const auto action = moves[std::min<std::size_t>(i, moves.size() - 1)];
                inputs.push_back(
                    std::uint16_t((action & 0xf4) | 1 |
                                  ((opt.spell < 0 && ((origin.sequence + i + 1) & 1)) ? 4096 : 0)));
            }
            output.push_back(std::move(inputs));
        }
        work.paths += output.size();
        return output;
    }
    RecordedResult solve(h::Session &world, const Packet &origin,
                         const std::vector<std::uint16_t> &lead, std::vector<std::uint16_t> nominal,
                         Boundary boundary, unsigned budget_ms) {
        if (!budget_ms || budget_ms > 10000 || lead.empty() || lead.size() >= options.horizon ||
            lead.size() >= options.verification_horizon || nominal.size() != options.horizon ||
            !std::equal(lead.begin(), lead.end(), nominal.begin()))
            throw std::runtime_error("invalid_recorded_planner_request");
        for (auto input : nominal)
            input_value(input);
        if (capture(world).values != origin.state.values)
            throw std::runtime_error("recorded_planner_origin_mismatch");
        if (world.state().player_state == 2 || world.state().deaths != 0)
            throw std::runtime_error("recorded_planner_after_nmnb_failure");
        unsigned threads = 0;
        for (const auto &entry : fs::directory_iterator("/proc/self/task")) {
            (void)entry;
            ++threads;
        }
        if (threads != 1)
            throw std::runtime_error("recorded_planner_requires_single_thread");
        const auto began = Clock::now();
        deadline = began + std::chrono::milliseconds(budget_ms);
        work = {};
        opt.spell = boundary.spell;
        RecordedResult output;
        std::vector<int> slots;
        for (unsigned round = 0; round < options.refinements && remaining() &&
                                 !output.selected.certifies() && !output.native_fault;
             ++round) {
            const auto forecast_status =
                forecast(world, origin, nominal, lead.size(), int(round), boundary);
            if (forecast_status == BranchStatus::Fault)
                output.native_fault = true;
            if (forecast_status != BranchStatus::Horizon)
                break;
            slots.push_back(int(round));
            std::vector<std::uint16_t> best = nominal;
            std::size_t longest = 0;
            const auto goal_seed = rng_state;
            for (unsigned goal = 0; goal < (options.region_goals ? 9u : 1u) && remaining() &&
                                    !output.selected.certifies() && !output.native_fault;
                 ++goal) {
                const auto paths = proposals(world, origin, lead, slots, goal, goal_seed);
                for (const auto &path : paths) {
                    const auto budget = remaining();
                    if (!budget)
                        break;
                    const std::vector<std::uint16_t> verified_inputs(
                        path.begin(), path.begin() + options.verification_horizon);
                    auto prediction =
                        predict_recorded_tape(world, origin, verified_inputs, lead.size(), boundary,
                                              budget, options.advance_dialogue);
                    if (prediction.status == BranchStatus::Fault)
                        output.native_fault = true;
                    if (prediction.completed > longest) {
                        longest = prediction.completed;
                        best = path;
                    }
                    if (prediction.certifies())
                        output.selected = prediction;
                    output.checks.push_back(std::move(prediction));
                    if (output.selected.certifies() || output.native_fault)
                        break;
                }
            }
            nominal = std::move(best);
        }
        output.work = work;
        output.wall_ms = std::chrono::duration<double, std::milli>(Clock::now() - began).count();
        return output;
    }
};
Prediction predict_recorded_tape(h::Session &world, const Packet &origin,
                                 const std::vector<std::uint16_t> &inputs, std::size_t forced,
                                 Boundary boundary, unsigned budget_ms, bool advance_dialogue) {
    if (!forced || forced > inputs.size())
        throw std::runtime_error("invalid_recorded_forced_lead");
    for (auto input : inputs)
        input_value(input);
    TapeInputs guidance(world, origin, inputs, advance_dialogue);
    const std::vector<std::uint16_t> lead(inputs.begin(), inputs.begin() + forced);
    return predict_guided(world, origin, lead, inputs.size(), guidance, boundary, budget_ms);
}
RecordedPlanner::RecordedPlanner(RecordedOptions options) : impl_(new Impl(options)) {}
RecordedPlanner::~RecordedPlanner() = default;
RecordedResult RecordedPlanner::solve(h::Session &world, const Packet &origin,
                                      const std::vector<std::uint16_t> &lead,
                                      const std::vector<std::uint16_t> &nominal, Boundary boundary,
                                      unsigned budget_ms) {
    return impl_->solve(world, origin, lead, nominal, boundary, budget_ms);
}
} // namespace th08::shadow
