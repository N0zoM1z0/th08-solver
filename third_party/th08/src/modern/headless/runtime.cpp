#include "runtime.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "modern/windows_runtime.hpp"
#include <array>

uint16_t th08_headless_input = 0;
uint64_t th08_headless_frame = 0;
uint64_t th08::headless::file_io_ns = 0;

namespace th08::headless {
namespace {
CollisionEvent observed_collision;
std::vector<LaserHitboxView> observed_laser_hitboxes;
std::vector<LethalRegionView> observed_lethal_regions;
struct LaserSnapshot {
    float origin_x = 0, origin_y = 0, angle = 0;
    bool in_use = false;
};
std::array<LaserSnapshot, 256> laser_snapshots;

int pooled_laser_slot(const Float3 *owner_position) {
    for (int i = 0; i < 256; ++i)
        if (&g_BulletManager.lasers[i].position == owner_position)
            return i;
    return -1;
}
} // namespace
void begin_update_observation() {
    observed_collision = {};
    observed_laser_hitboxes.clear();
    observed_lethal_regions.clear();
    for (int i = 0; i < 256; ++i) {
        const auto &laser = g_BulletManager.lasers[i];
        laser_snapshots[i] =
            laser.inUse ? LaserSnapshot{laser.position.x, laser.position.y, laser.angle, true}
                        : LaserSnapshot{};
    }
}
CollisionEvent current_collision() {
    return observed_collision;
}
const std::vector<LaserHitboxView> &current_laser_hitboxes() {
    return observed_laser_hitboxes;
}
const std::vector<LethalRegionView> &current_lethal_regions() {
    return observed_lethal_regions;
}
LaserMotionObservation current_laser_motion(int slot) {
    if (slot < 0 || slot >= 256)
        return {};
    const auto &before = laser_snapshots[slot];
    const auto &after = g_BulletManager.lasers[slot];
    if (!before.in_use || !after.inUse)
        return {};
    float angle_delta = after.angle - before.angle;
    constexpr float pi = 3.14159265358979323846f, tau = 2 * pi;
    while (angle_delta > pi)
        angle_delta -= tau;
    while (angle_delta < -pi)
        angle_delta += tau;
    return {after.position.x - before.origin_x, after.position.y - before.origin_y, angle_delta,
            true};
}
void prepare_observation_storage() {
    // Pooled lasers can submit two calls on a lifecycle transition, while active
    // enemies can add direct ECL calls. This avoids growth in measured scenes;
    // vector ownership still preserves every call if a later scene exceeds it.
    observed_laser_hitboxes.reserve(1536);
    // Trails can submit more regions than active enemies. Reserve only avoids
    // common reallocations; vector ownership retains every call without a cap.
    observed_lethal_regions.reserve(512);
}
void record_laser_hitbox(const Float3 &center, const Float3 &size, const Float3 &origin,
                         float angle, bool graze_enabled) {
    observed_laser_hitboxes.push_back({center.x, center.y, size.x, size.y, origin.x, origin.y,
                                       angle, pooled_laser_slot(&origin), graze_enabled});
}
void record_lethal_region(const Float3 &minimum, const Float3 &maximum) {
    observed_lethal_regions.push_back({{minimum.x, minimum.y, maximum.x, maximum.y}});
}
void record_collision(CollisionKind kind, const Float3 &pmin, const Float3 &pmax,
                      const Float3 &hmin, const Float3 &hmax, const Float3 *owner_position) {
    if (observed_collision.kind != CollisionKind::None)
        return;
    observed_collision.kind = kind;
    observed_collision.frame = th08_headless_frame;
    observed_collision.player = {pmin.x, pmin.y, pmax.x, pmax.y};
    observed_collision.hazard = {hmin.x, hmin.y, hmax.x, hmax.y};
    observed_collision.movement_input = g_GuiMessageInputCurrent;
    observed_collision.sampled_input = g_CurFrameInput;
    if (kind == CollisionKind::Bullet) {
        for (int i = 0; i < 1536; ++i) {
            const auto &bullet = g_BulletManager.bullets[i];
            if (&bullet.position != owner_position)
                continue;
            observed_collision.bullet_slot = i;
            observed_collision.vx = bullet.velocity.x;
            observed_collision.vy = bullet.velocity.y;
            observed_collision.active_transforms = bullet.activeTransformFlags;
            break;
        }
    } else if (kind == CollisionKind::Laser) {
        observed_collision.laser_slot = pooled_laser_slot(owner_position);
        if (!observed_laser_hitboxes.empty()) {
            const auto &hitbox = observed_laser_hitboxes.back();
            observed_collision.laser_hitbox_call = int(observed_laser_hitboxes.size() - 1);
            observed_collision.laser_center_x = hitbox.center_x;
            observed_collision.laser_center_y = hitbox.center_y;
            observed_collision.laser_full_width = hitbox.full_width;
            observed_collision.laser_full_height = hitbox.full_height;
            observed_collision.laser_origin_x = hitbox.origin_x;
            observed_collision.laser_origin_y = hitbox.origin_y;
            observed_collision.laser_angle = hitbox.angle;
        }
    }
}
} // namespace th08::headless

namespace th08::modern {
// WinMain is linked for legacy callers, but headless startup bypasses its window
// and file-writing paths. Calling that entry point is unsupported in this build.
bool ConfigureDataDirectory() {
    return false;
}
void InstallCrashReporter() {}
void LogArchiveRequest(const char *) {}
} // namespace th08::modern
