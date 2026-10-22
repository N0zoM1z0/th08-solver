#pragma once

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
struct Bullet {
    float x = 0, y = 0, vx = 0, vy = 0;
    std::uint16_t state = 0;
    int slot = -1, sprite = 0;
    float full_width = 0, full_height = 0;
    std::uint32_t active_transforms = 0;
    float vector_acceleration_x = 0, vector_acceleration_y = 0;
    int vector_acceleration_timer = 0, vector_acceleration_duration = 0;
    bool collision_disabled = false;
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
};
struct Configuration {
    int horizon = 32, beam = 192;
    float grow = 0.02f, boss_follow = 0.25f;
    float risk_weight = 20, nowarn_weight = 30, focus_bias = 0;
    bool risk_hard = false, reach_filter = false, risk_stop_age = false;
    bool risk_phantoms = false;
};
struct RiskTables {
    std::map<std::uint64_t, std::array<float, 4>> fine, segment, global;
    std::map<int, std::array<float, 24 * 28>> spawn;
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
struct Decision {
    std::uint16_t action = 1;
    int depth = 0;
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
    Decision decide(const Observation &observation);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace th08::online
