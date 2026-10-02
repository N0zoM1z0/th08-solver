#pragma once
#include <cstdint>
#include <vector>

namespace th08::headless {
struct Config {
    const char *dat_path = nullptr;
    int stage = 0, spell = -1, difficulty = 0;
    std::uint16_t seed = 0;
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
    float focused_axis_speed() const;
    float focused_diagonal_speed() const;
    // A diagnostics projection of actor/script state, not a serialized world.
    std::uint64_t actor_digest() const;
    std::uint64_t file_io_time_ns() const;
    CollisionEvent collision() const;

  private:
    std::vector<BulletView> views_;
    std::vector<LaserView> laser_views_;
};
} // namespace th08::headless
