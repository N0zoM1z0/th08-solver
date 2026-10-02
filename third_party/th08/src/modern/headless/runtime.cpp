#include "runtime.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "modern/windows_runtime.hpp"

uint16_t th08_headless_input = 0;
uint64_t th08_headless_frame = 0;
uint64_t th08::headless::file_io_ns = 0;

namespace th08::headless {
namespace {
CollisionEvent observed_collision;
}
void clear_collision() {
    observed_collision = {};
}
CollisionEvent current_collision() {
    return observed_collision;
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
        for (int i = 0; i < 256; ++i)
            if (&g_BulletManager.lasers[i].position == owner_position) {
                observed_collision.laser_slot = i;
                break;
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
