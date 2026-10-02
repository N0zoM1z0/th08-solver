#pragma once
#include <cstdint>
#include <vector>

namespace th08::headless {
struct Config {
    const char *dat_path = nullptr;
    int stage = 0, spell = -1, difficulty = 0;
    std::uint16_t seed = 0;
};
struct RelativeDirectionView {
    float angle = 0, speed = 0, turn_angle = 0, turn_speed = 0;
    int timer = 0, interval = 0;
    // Includes the final direction-change update, but never crosses into a
    // subsequent transform or concurrently enabled transform-program record.
    unsigned updates = 0;
};
struct BoundaryBounceView {
    float angle = 0, speed = 0, sprite_width = 0, sprite_height = 0;
    unsigned remaining = 0;
    bool supported = false;
};
struct WaitVectorView {
    float acceleration_x = 0, acceleration_y = 0;
    unsigned wait_updates = 0, acceleration_updates = 0;
    // Includes WAIT expiry and the final acceleration-clear update's movement.
    unsigned updates = 0;
};
struct BulletView {
    float x, y, vx, vy;
    std::uint16_t state;
    int slot;
    float full_width, full_height;
    std::uint32_t active_transforms;
    // Source-owned state for the active vector-acceleration opcode. Values are
    // copied after the native update; no transform program is executed here.
    float vector_acceleration_x, vector_acceleration_y;
    int vector_acceleration_timer, vector_acceleration_duration;
    // Number of upcoming native updates whose velocity is guaranteed unchanged
    // across the active WAIT and any proven terminal child-spawn boundary.
    int wait_linear_updates;
    RelativeDirectionView relative_direction;
    BoundaryBounceView boundary_bounce;
    WaitVectorView wait_vector;
};
// Raw source-owned laser state after an update. Consumers may forecast existing
// lasers, but newly spawned/aimed lasers still belong to the next native update.
struct LaserView {
    float origin_x, origin_y, angle;
    float start_offset, end_offset, start_length, width, speed;
    int start_time, hitbox_start_time, duration, despawn_duration, hitbox_end_delay;
    int timer, slot;
    std::uint16_t flags;
    std::uint8_t state;
    // Native changes observed across the most recent update for this same pool
    // slot. These are evidence about the past update, not a promised future.
    float origin_delta_x, origin_delta_y, angle_delta;
    bool motion_observed;
};
// Exact geometry submitted to Player::CalcLaserHitbox during one native update.
// Center/size use the source function's rotated comparison coordinates; origin
// and angle map a world-space player position into those coordinates.
struct LaserHitboxView {
    float center_x, center_y, full_width, full_height;
    float origin_x, origin_y, angle;
    int pooled_slot;
    bool graze_enabled;
};
// A read-only post-update cursor into one active native ECL context. Operands
// remain raw: consumers must not assume that a flagged value is constant or
// that linear lookahead crosses a control-flow instruction.
struct EclContextView {
    int enemy_index, child_slot, sub_id, time;
    int next_time, next_opcode, next_offset;
    int secondary_time, pending_subroutine, active_interpolations, per_frame_ex;
    std::uint8_t difficulty_mask;
    std::uint16_t operand_flags;
    std::uint32_t enemy_flags;
    bool difficulty_enabled, has_raw_int0, has_parent;
    int raw_int0;
    float position_x, position_y, position_offset_x, position_offset_y;
    float velocity_x, velocity_y, rotation, rotation_velocity;
    float variable0, variable1;
};
// A decoded opcode-114 instruction that the native ECL interpreter will execute
// on the next update. Unsupported previews are retained so an enabled solver
// adapter can stop instead of silently omitting an action-dependent future.
struct ImminentLaserSpawnView {
    bool supported = false, suppressed = false;
    int enemy_index = -1, opcode = 0;
    float origin_x = 0, origin_y = 0, angle = 0;
    float start_offset = 0, end_offset = 0, start_length = 0, width = 0, speed = 0;
    int start_time = 0, hitbox_start_time = 0, duration = 0, despawn_duration = 0;
    int hitbox_end_delay = 0;
    std::uint16_t flags = 0;
};
// One bullet at its first native collision update, produced by a currently
// visible future ECL shot. update is one-based from the observation boundary.
// Unsupported entries preserve the source encounter so an enabled case
// adapter can stop instead of treating unknown emission as empty space.
enum class UpcomingBulletSpawnFailure {
    None,
    InstructionSequence,
    DynamicContext,
    EnemyMotion,
    Operands,
    TransformProgram,
    SpriteGeometry,
    OffscreenCull,
    PoolCapacity,
};
struct UpcomingBulletSpawnView {
    bool supported = false, suppressed = false;
    UpcomingBulletSpawnFailure failure = UpcomingBulletSpawnFailure::None;
    int enemy_index = -1, opcode = 0;
    unsigned update = 0, linear_updates = 0;
    float x = 0, y = 0, vx = 0, vy = 0;
    float full_width = 0, full_height = 0;
};
enum class CollisionKind { None, Bullet, LethalRegion, Laser };
struct Bounds {
    float left = 0, top = 0, right = 0, bottom = 0;
};
// Captured at the successful original collision test, before death feedback.
// Laser bounds are in its rotated test coordinates; other bounds are world pixels.
struct CollisionEvent {
    CollisionKind kind = CollisionKind::None;
    std::uint64_t frame = 0;
    Bounds player, hazard;
    int bullet_slot = -1, laser_slot = -1;
    float vx = 0, vy = 0;
    std::uint32_t active_transforms = 0;
    int laser_hitbox_call = -1;
    float laser_center_x = 0, laser_center_y = 0;
    float laser_full_width = 0, laser_full_height = 0;
    float laser_origin_x = 0, laser_origin_y = 0, laser_angle = 0;
    std::uint16_t movement_input = 0, sampled_input = 0;
};
struct State {
    std::uint64_t frame = 0;
    float x = 0, y = 0, deaths = 0;
    int stage = 0, player_state = 0, bullets = 0, enemies = 0, spell = -1;
    bool spell_active = false, retry_menu = false, stage_complete = false;
    std::uint16_t rng_seed = 0;
    std::uint32_t rng_draws = 0;
    std::uint32_t score = 0;
    int graze = 0, gauge = 0;
    float lives = 0;
    float hurt_half_x = 0, hurt_half_y = 0;
    // Recording mode latches movement input after Player/BulletManager update.
    std::uint16_t latched_input = 0, sampled_input = 0;
};
// Upstream managers are pointer-rich process globals. Exactly one noncopyable
// session may be created per process; fresh processes provide fresh replay.
// No world snapshot/branch independence is claimed by this adapter.
class Session {
  public:
    explicit Session(const Config &);
    ~Session();
    Session(const Session &) = delete;
    Session &operator=(const Session &) = delete;
    State step(std::uint16_t input);
    State state() const;
    // Reused snapshot storage; its contents are invalidated by the next call.
    const std::vector<BulletView> &bullets();
    // Reused independently from bullets(); invalidated by the next lasers() call.
    const std::vector<LaserView> &lasers();
    // Calls observed in the most recent native update; invalidated by step().
    const std::vector<LaserHitboxView> &laser_hitboxes() const;
    // Reused post-update ECL cursor storage; invalidated by the next call.
    const std::vector<EclContextView> &ecl_contexts();
    // Reused source-owned spawn previews; this read never resolves RNG operands.
    const std::vector<ImminentLaserSpawnView> &imminent_laser_spawns();
    // Reused bounded ECL bullet previews. Only a linear current cursor, known
    // enemy motion and deterministic non-aimed patterns are exported.
    const std::vector<UpcomingBulletSpawnView> &upcoming_bullet_spawns(unsigned horizon);
    float focused_axis_speed() const;
    float focused_diagonal_speed() const;
    // A diagnostics projection of actor/script state, not a serialized world.
    std::uint64_t actor_digest() const;
    std::uint64_t file_io_time_ns() const;
    CollisionEvent collision() const;

  private:
    std::vector<BulletView> views_;
    std::vector<LaserView> laser_views_;
    std::vector<EclContextView> ecl_views_;
    std::vector<ImminentLaserSpawnView> laser_spawn_views_;
    std::vector<UpcomingBulletSpawnView> bullet_spawn_views_;
};
} // namespace th08::headless
