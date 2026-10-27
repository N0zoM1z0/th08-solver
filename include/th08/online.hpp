#pragma once
#include "th08/bullet_motion.hpp"
#include "th08/online_value.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace th08::online {
// Describes this policy library, independently of the native runtime/CLI build.
const char *build_architecture();

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
    // An owned source-state alternative, without an activation-time promise.
    bool alternate_motion = false;
    float alternate_vx = 0, alternate_vy = 0;
    // Current-program hypothesis for this incarnation, not a slot-lifetime
    // guarantee. One-based last potentially colliding update; zero is unknown.
    unsigned collision_until = 0;
    bool grazed = false;
    unsigned graze_ready_in = 0;
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
// Current-program child-pattern birth under a linear parent hypothesis. The
// speed bound covers every angle without sampling gameplay RNG. Coverage ends
// after the birth/first-following update, when the live observer can own children.
struct ChildBirth {
    unsigned update = 0;
    float x = 0, y = 0, max_speed = 0, width = 0, height = 0;
    int parent_slot = -1;
};
struct QueuedLaser {
    // Inclusive one-based lethal interval from the current observation boundary.
    // Geometry is an adapter hypothesis, not a complete future-world guarantee.
    unsigned first_update = 0, last_update = 0;
    LaserHitbox hitbox;
    float turn_hint = 0; // radians per batch, from literal current-program writes
};
enum class PickupKind { Point, Power };
struct ScorePickup {
    float x = 0, y = 0;
    PickupKind kind = PickupKind::Point;
};
// Current accepted-shot targets whose native HP loss is suppressed. Remaining
// reduction is a current timer, not a promise that the actor or state persists.
struct IncomeTarget {
    float x = 0, y = 0;
    int reduction_updates = 0;
};
// Current native scoring inputs only; no predicted drops, route script or RNG.
struct ScoreMemory {
    bool available = false, has_boss = false, auto_collect_eligible = false;
    bool can_switch_forms = false, survival_spell = false;
    bool spell_damageable = false, extreme_youkai = false;
    // Actual effective HP to the next callback and source update counts. Orb
    // value/bonus decay use displayed score units per orb/per native update.
    int capture_hp = 0, spell_remaining = 0, reduction_remaining = 0;
    int collected_orbs = 0;
    float orb_value = 100, bonus_decay = 0;
    float boss_x = 192, collection_line = 128, power = 0;
    // Player pickup half-height plus the source item box half-height. Point
    // pricing uses the item's Y, even when the player has crossed the line.
    float pickup_reach_y = 0;
    // Unconditional counts on currently damageable enemies, at current power;
    // periodic/random drops are omitted. X is a count-weighted current origin,
    // not a promise about RNG scatter, a future kill or a realized pickup.
    std::uint64_t impending_points = 0;
    float impending_x = 192;
    // Native refreshes this optional target from the current enemy pool on each
    // observation; no actor pointer or target identity survives into Policy.
    // Coordinates are playfield pixels (Y increases downward). X is the actor's
    // lane; Y is a proposed position 64px below it for upward player shots.
    // The default adapter only considers actors above the player. Vertical
    // following also admits lower actors if that shooting position is in bounds.
    // Drop count / (HP * travel penalty) ranks proposals, not expected cash or
    // guaranteed kills. Native rollouts still decide damage, safety and drops.
    bool has_drop_target = false;
    float drop_target_x = 192, drop_target_y = 400;
    // Current root has an attachment link. This is a form/aim cue only; no
    // child pointer, chain size or future reward is retained by the policy.
    bool drop_target_has_children = false;
    // Signed source timer frames left in the selected root's reduction window;
    // nonpositive means expired. The fixed-rate Native profile consumes one
    // per gameplay update. Used only by the optional chain prior.
    int drop_target_reduction_updates = 0;
    std::vector<ScorePickup> pickups;
    std::vector<IncomeTarget> income_targets;
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
    Vector graze_half;
    ScoreMemory scoring;
    std::vector<Bullet> bullets;
    std::vector<LaserState> lasers;
    std::vector<LaserHitbox> laser_hitboxes;
    std::vector<Box> lethal_regions;
    std::vector<Actor> actors;
    // Positive prefixes from the existing narrow current-cursor adapter. This
    // is partial coverage: zero records never proves that no shot will spawn.
    std::vector<QueuedShot> queued_shots;
    std::vector<ChildBirth> child_births;
    std::uint32_t unsupported_child_births = 0;
    std::uint32_t unsupported_lifetimes = 0;
    std::uint32_t unsupported_queued_shots = 0;
    std::vector<QueuedLaser> queued_lasers;
    std::uint32_t unsupported_queued_lasers = 0;
    // Source-decoded main-cursor pooled spawns, immediately before their first
    // laser update. Origin/later script behavior remain a snapshot hypothesis.
    std::vector<LaserState> queued_pooled_lasers;
    std::uint32_t unsupported_queued_pooled_lasers = 0;
    std::uint32_t turn_hint_records = 0, unsupported_turn_hints = 0;
};
// Heuristic proposal preferences, independent of collision and native safety.
// Defaults preserve the original cost expression and tie sequence.
struct StyleWeights {
    float margin_early = 40, margin_late = 10;
    int margin_split = 12;
    float anchor_weight = 0.02f, anchor_x = 192, anchor_y = 400;
    float anchor_x_min = 48, anchor_x_max = 336;
    float change_cost = 0.01f, move_cost = 0, focused_move_cost = 0, reversal_cost = 0;
    float edge_cost = 0, edge_margin = 32;
    // Once per forecast path for each of at most 64 nearby ungrazed bullets.
    // This is a contact proxy, not a forecast of native score or laser graze.
    float graze_reward = 0;
};
struct Configuration {
    StyleWeights style;
    bool scoring_collection = false, scoring_aim = false;
    bool scoring_survival_focus = false;
    bool scoring_survival_flow = false;
    bool scoring_attack_pacing = false;
    // Experimental: keep a measured profitable farm past its capture reserve.
    // Native callers must opt out of the all-capture goal separately.
    bool scoring_farm_to_timeout = false;
    // Opt-in Sample-phase proposal: hold fire while the focused shooting gauge
    // is charging, then release at the first observed extreme-youkai state.
    // Actual HP, carried shots and the ordinary capture reserve still govern.
    bool scoring_farm_charge_fire = false;
    bool scoring_hit_income = false;
    bool scoring_drop_target = false;
    // Requires drop_target. Allow its stage-only lane proposal to move downward
    // after an enemy's reduction window ends; pickup/hit-income goals still win.
    bool scoring_drop_follow = false;
    // Zero disables the parent-chain proposal. Positive values require
    // drop_target, exclude attached children from its lane candidates and
    // prefer focused shots at a current root while no pickup trip owns form.
    // The bias uses the same per-update cost units as the other form priors.
    float parent_chain_focus_bias = 0;
    float attack_focus_bias = 5;
    float attack_reserve_factor = 1.5f;
    unsigned attack_reserve_updates = 240;
    float survival_focus_bias = 0.01f;
    bool scoring_boss_collection = false;
    bool scoring_precollect = false;
    bool scoring_priced_collection = false, scoring_upper_stage = false;
    float collection_focus_bias = 0, economic_human_bias = 0;
    int horizon = 32, beam = 192;
    // Held-input updates beyond retained full-horizon endpoints; H + T <= 200.
    int tail_check = 0;
    int diversity_cell = 0;
    float grow = 0.02f, boss_follow = 0.25f;
    float risk_weight = 20, nowarn_weight = 30, focus_bias = 0;
    float spawn_guard = 0; // soft risk radius in pixels around current actor shot origins
    bool risk_hard = false, reach_filter = false, risk_stop_age = false;
    bool risk_phantoms = false;
    // Conservative proposal prior for fired collision-disabled bullets; no activation schedule.
    bool hard_phantoms = false;
    bool motion_envelope = false;
    bool child_births = false;
    bool scheduled_expiry = false;
    // Keep incoming direction where the next expansion repeats it and at
    // terminal endpoints whose conditional value can depend on the last input.
    bool retain_headings = false;
    float value_weight = 1;
    bool queued_shots = false, queued_lasers = false;
    bool held_fallback = false;
    // Recheck one previous proposal only when the new beam cannot fill its horizon.
    bool warm_fallback = false;
    bool spawn_clock = false;
    bool direct_turn_envelope = false;
    bool relative_turns = false;
    // Synchronous deterministic delay experiment, not a real background worker.
    // Full searches force the N committed movement/focus inputs before their
    // first new decision. Zero preserves the original synchronous policy.
    int planner_lag = 0;
    int executor_check = 32;
    int emergency_horizon = 32, emergency_beam = 48;
    bool lazy_index = false; // Delay unused frame indices; forecasts remain unchanged.
};
// General proposal controls: baseline, zen, expert, sniper, graze, score,
// score-survival, score-harvest, score-precollect and score-flow.
// Score adds current-item collection and literal boss aiming. No preference
// changes the horizon, beam, verification budgets or physics.
Configuration with_style(const std::string &name, Configuration configuration = {});
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
    std::uint64_t proposal_grid_expansions = 0;
    std::uint64_t heading_layers = 0, heading_keys = 0;
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
    std::uint64_t planner_searches = 0, plan_adoptions = 0, executor_checks = 0;
    std::uint64_t emergency_searches = 0, pending_dropped = 0, executor_invalidations = 0;
    double planner_ms = 0; // Full search only; emergency work remains executor cost.
    std::uint64_t indexed_forecast_frames = 0;
    std::uint64_t hard_phantom_frames = 0;
    std::uint64_t motion_envelope_frames = 0;
    std::uint64_t child_birth_frames = 0;
    // Positive lifetime hypotheses actually used in hard/soft projections,
    // and their pre-pruning bullet/update work. Unknown preview clocks keep
    // the full original projection; these counts do not imply full coverage.
    std::uint64_t lifetime_records = 0, lifetime_projected_frames = 0;
    std::uint64_t lifetime_skipped_frames = 0, unsupported_lifetimes = 0;
    std::uint64_t value_evaluations = 0;
    double value_ms = 0;
};
struct Decision {
    std::uint16_t action = 1;
    // Lag zero: search depth. Delayed mode: current checked execution depth,
    // capped at executor_check, including when the emergency search is longer.
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
    explicit Policy(Configuration configuration, RiskTables risk = {}, BranchValueModel value = {});
    bool needs_scoring_memory() const;
    bool needs_hit_income_memory() const;
    bool needs_drop_follow_memory() const;
    bool needs_parent_chain_memory() const;
    // Learned branch features have a fixed24-update schema. Native guidance
    // can use ordinary single-action decisions for other policy horizons.
    bool supports_value_proposals() const;
    // Fire intent after the latest observation, including collection preparation
    // and attack pacing. Movement recovery may preserve this owned goal.
    bool wants_fire() const;
    ~Policy();
    Policy(const Policy &) = delete;
    Policy &operator=(const Policy &) = delete;
    // Warm observer history from an explicitly recorded prefix, without choosing
    // actions or advancing the policy's search tie RNG. Re-observing the same
    // unchanged snapshot is allowed; deciding twice at one update is rejected.
    void observe(const Observation &observation);
    // Optional offline collection exports up to four owned retained H24 prefixes
    // and current/proposed features. No future native state is queried. Tracing
    // preserves the selected action and tie RNG; partial searches export none.
    Decision decide(const Observation &observation, DecisionTiming *timing = nullptr,
                    std::vector<ValueProposal> *proposals = nullptr);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace th08::online
