#include "BulletManager.hpp"
#include "ItemManager.hpp"
#include "modern/headless/session.hpp"
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
namespace {
void check(bool valid, const char *message) {
    if (!valid)
        throw std::runtime_error(message);
}
class RunDirectory {
  public:
    RunDirectory() : previous(fs::current_path()) {
        char pattern[] = "/tmp/th08-laser-items-XXXXXX";
        const auto *created = mkdtemp(pattern);
        if (!created)
            throw std::runtime_error("cannot create native test directory");
        path = created;
        fs::current_path(path);
    }
    ~RunDirectory() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(path, error);
    }

  private:
    fs::path previous, path;
};
struct Case {
    float x, y, angle, start, end;
};

void laser_case(const Case &test, bool despawn, bool award) {
    using namespace th08;
    g_ItemManager.Initialize();
    for (unsigned i = 0; i < 1536; ++i)
        g_BulletManager.bullets[i].state = BULLET_STATE_UNUSED;
    std::memset(g_BulletManager.lasers, 0, sizeof(g_BulletManager.lasers));
    g_BulletManager.cancelItemType = ITEM_POINT_STAR;
    auto &laser = g_BulletManager.lasers[0];
    laser.inUse = 1;
    laser.state = LASER_STATE_ACTIVE;
    laser.position = Float3(test.x, test.y, 0);
    laser.angle = test.angle;
    laser.startOffset = test.start;
    laser.endOffset = test.end;
    laser.currentWidth = 8;
    laser.hitboxEndDelay = 40;
    laser.timer = 7;

    std::vector<Float3> expected;
    auto append = [&](float x, float y) {
        // SpawnItem rejects an out-of-range X before touching its pool cursor.
        if (x >= -64.f && x <= 448.f)
            expected.emplace_back(x, y, 0);
    };
    if (award) {
        if (despawn)
            append(test.x, test.y);
        for (float radius = test.start; radius < test.end; radius += 32.f)
            append(std::cos(test.angle) * radius + test.x, std::sin(test.angle) * radius + test.y);
    }
    if (despawn)
        g_BulletManager.DespawnBullets(8000, award);
    else
        g_BulletManager.RemoveAllBullets(award ? ITEM_STATE_AUTOCOLLECT : 4);

    const Item *item = g_ItemManager.itemListHead.next;
    for (const auto &position : expected) {
        check(item && item->isInUse && item->itemType == ITEM_POINT_STAR &&
                  item->state == ITEM_STATE_AUTOCOLLECT && item->currentPosition.z == 0.f,
              "laser cancellation produced the wrong item count/type");
        check(std::abs(item->currentPosition.x - position.x) <= 2e-5f &&
                  std::abs(item->currentPosition.y - position.y) <= 2e-5f,
              "laser cancellation did not place items along its angle");
        item = item->next;
    }
    check(!item, "laser cancellation emitted past its exclusive end offset or X bound");
    check(laser.state == LASER_STATE_DESPAWNING && int(laser.timer) == 0 &&
              laser.hitboxEndDelay == 0 && g_BulletManager.spawnSuppressionFrames == 10,
          "laser cancellation lifecycle changed while initializing trigonometry");
}
} // namespace

int main(int argc, char **argv) {
    try {
        check(argc == 2, "usage: th08_headless_laser_items DAT");
        const auto dat = fs::absolute(argv[1]).string();
        RunDirectory directory;
        th08::headless::Session session({dat.c_str(), 0, -1, 0, 0});
        constexpr float half_pi = 1.57079632679489661923f;
        const std::array<Case, 8> cases{{{100, 200, 0, 16, 80},
                                         {100, 200, half_pi, 32, 96},
                                         {100, 200, -half_pi, 32, 96},
                                         {100, 200, .37f, 0, 96},
                                         {440, 200, 0, 0, 96},
                                         {100, 200, half_pi * 2, 32, 96},
                                         {-60, 200, half_pi * 2, 0, 96},
                                         {100, 200, .37f, 32, 32}}};
        for (bool despawn : {false, true}) {
            for (const auto &test : cases)
                laser_case(test, despawn, true);
            laser_case(cases.front(), despawn, false);
        }
        std::cout << "{\"laser_cancel_cases\":18,\"item_geometry\":\"covered\","
                     "\"x_rejection\":\"covered\",\"lifecycle\":\"covered\"}\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
