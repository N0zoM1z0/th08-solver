#pragma once
#include "th08/bullet_motion.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace th08::online {

struct Vector {
    float x = 0, y = 0, z = 0;
    Vector operator+(const Vector &v) const {
        return {x + v.x, y + v.y, z + v.z};
    }
    Vector operator-(const Vector &v) const {
        return {x - v.x, y - v.y, z - v.z};
    }
    Vector operator/(float v) const {
        return {x / v, y / v, z / v};
    }
};
struct Box {
    float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
};
struct LaserHitbox {
    float cx = 0, cy = 0, w = 0, h = 0, ox = 0, oy = 0, angle = 0;
    int pooled_slot = -1;
};
struct Movement {
    float axis[2]{}, diag[2]{};
    float hmul = 1, vmul = 1, frm = 1;
    float minx = 0, miny = 0, maxx = 0, maxy = 0;
    Vector half;
};
// Current source state, owned by the observation. updates includes the last
// relative turn but stops before any subsequent/concurrent transform program.
struct RelativeTurn {
    unsigned updates = 0;
    float angle = 0, speed = 0;
    bullet::DirectionChange state;
};
struct Bullet {
    float x = 0, y = 0, vx = 0, vy = 0;
    std::uint16_t state = 0;
    int slot = -1, sprite = 0;
    float full_width = 0, full_height = 0;
    std::uint32_t active_transforms = 0;
    float vector_acceleration_x = 0, vector_acceleration_y = 0;
    int vector_acceleration_timer = 0, vector_acceleration_duration = 0;
    bool collision_disabled = false;
    RelativeTurn relative_turn;
    // One-based future animation-completion update; zero leaves timing unknown.
    // This owns no ANM pointer and does not certify activation transforms/motion.
    unsigned activation_update = 0;
};
struct LaserState {
    float endOffset = 0, startOffset = 0, startLength = 0, width = 0;
    Vector position;
    float angle = 0, speed = 0;
    int timer = 0, state = 0, startTime = 0, hitboxStartTime = 0, duration = 0;
    int despawnDuration = 0, hitboxEndDelay = 0;
    std::uint16_t flags = 0;
    float origin_delta_x = 0, origin_delta_y = 0, angle_delta = 0;
    bool motion_observed = false;
};
struct Actor {
    int slot = -1;
    float origin_x = 0, origin_y = 0;
    float delta_x = 0, delta_y = 0;
};
struct QueuedShot {
    unsigned update = 0, linear_updates = 0;
    float x = 0, y = 0, vx = 0, vy = 0, width = 0, height = 0;
};
struct QueuedLaser {
    // Inclusive one-based lethal interval from the current observation boundary.
    // Geometry is an adapter hypothesis, not a complete future-world guarantee.
    unsigned first_update = 0, last_update = 0;
    LaserHitbox hitbox;
    float turn_hint = 0; // radians per batch, from literal current-program writes
};

// Owned current-memory snapshot. The native adapter may reuse its vector capacity
// after decide() returns. No simulator, manager pointer or future rollout enters
// this interface. Current transform/lifetime fields are intentionally permitted.
// Coordinates must be finite and within +/-2^20; observations are consecutive
// native API updates, including control-only stage handoffs.
struct Observation {
    std::uint64_t update = 0;
    int stage = 0, spell = -1;
    bool spell_active = false, youkai = false;
    float x = 0, y = 0;
    std::uint16_t latched_input = 0;
    Movement movement;
    std::vector<Bullet> bullets;
    std::vector<LaserState> lasers;
    std::vector<LaserHitbox> laser_hitboxes;
    std::vector<Box> lethal_regions;
    std::vector<Actor> actors;
    // Positive prefixes from the existing narrow current-cursor adapter. This
    // is partial coverage: zero records never proves that no shot will spawn.
    std::vector<QueuedShot> queued_shots;
    std::uint32_t unsupported_queued_shots = 0;
    std::vector<QueuedLaser> queued_lasers;
    std::uint32_t unsupported_queued_lasers = 0;
    // Source-decoded main-cursor pooled spawns, immediately before their first
    // laser update. Origin/later script behavior remain a snapshot hypothesis.
    std::vector<LaserState> queued_pooled_lasers;
    std::uint32_t unsupported_queued_pooled_lasers = 0;
    std::uint32_t turn_hint_records = 0, unsupported_turn_hints = 0;
};
struct Configuration {
    int horizon = 32, beam = 192;
    // Held-input updates beyond retained full-horizon endpoints; H + T <= 200.
    int tail_check = 0;
    int diversity_cell = 0;
    float grow = 0.02f, boss_follow = 0.25f;
    float risk_weight = 20, nowarn_weight = 30, focus_bias = 0;
    float spawn_guard = 0; // soft risk radius in pixels around current actor shot origins
    bool risk_hard = false, reach_filter = false, risk_stop_age = false;
    bool risk_phantoms = false;
    bool queued_shots = false, queued_lasers = false;
    bool held_fallback = false;
    // Recheck one previous proposal only when the new beam cannot fill its horizon.
    bool warm_fallback = false;
    bool spawn_clock = false;
    bool direct_turn_envelope = false;
    bool relative_turns = false;
};
using SpawnGrid = std::array<float, 24 * 28>;
// Clipped 3x3 no-warning birth sums over 16-pixel cells. Preserve each center's
// row-major float addition order; the result owns its storage independently.
SpawnGrid spawn_neighborhood_sums(const SpawnGrid &source);
struct RiskTables {
    std::map<std::uint64_t, std::array<float, 4>> fine, segment, global;
    std::map<int, SpawnGrid> spawn;
    bool loaded = false;
};
inline std::uint64_t risk_key(long segment, long sprite, long size10, long speed, long age) {
    return (std::uint64_t(segment & 0xffff) << 48) | (std::uint64_t(sprite & 0xffff) << 32) |
           (std::uint64_t(size10 & 0xffff) << 16) | (std::uint64_t(speed & 0xff) << 8) |
           std::uint64_t(age & 0xff);
}
// Parse complete TSV rows before planning. Truncated/malformed rows fail; they
// must never consume columns from the following record or silently end a model.
RiskTables load_risk_tables(const std::string &directory);
struct SearchStatistics {
    std::uint64_t selections = 0, candidates = 0, bins = 0, retained_bins = 0;
    double selection_ms = 0;
    std::uint64_t held_paths = 0, held_updates = 0, held_selected = 0;
    std::uint64_t warm_paths = 0, warm_updates = 0, warm_selected = 0, warm_invalidated = 0;
    std::uint64_t turn_envelope_frames = 0;
    std::uint64_t relative_turn_frames = 0;
    std::uint64_t spawn_clock_frames = 0;
    std::uint64_t tail_nodes = 0, tail_paths = 0, tail_updates = 0, tail_selected = 0;
    unsigned tail_best_depth = 0;
    double tail_selection_ms = 0;
};
struct Decision {
    std::uint16_t action = 1;
    int depth = 0;
    SearchStatistics search;
};
// Optional monotonic-clock diagnostics. These disjoint policy phases exclude
// observation capture and native execution; spatial-selection time is nested
// within search. The caller owns the output, reset even if decide() rejects input.
struct DecisionTiming {
    std::uint64_t history_ns = 0, forecast_ns = 0, index_ns = 0;
    std::uint64_t search_ns = 0, fallback_ns = 0;
};
class Policy {
  public:
    explicit Policy(Configuration configuration, RiskTables risk = {});
    ~Policy();
    Policy(const Policy &) = delete;
    Policy &operator=(const Policy &) = delete;
    // Warm observer history from an explicitly recorded prefix, without choosing
    // actions or advancing the policy's search tie RNG. Re-observing the same
    // unchanged snapshot is allowed; deciding twice at one update is rejected.
    void observe(const Observation &observation);
    Decision decide(const Observation &observation, DecisionTiming *timing = nullptr);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace th08::online
