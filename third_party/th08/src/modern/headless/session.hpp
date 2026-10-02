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
    const std::vector<BulletView> &bullets();
    float focused_axis_speed() const;
    float focused_diagonal_speed() const;
    // A diagnostics projection of actor/script state, not a serialized world.
    std::uint64_t actor_digest() const;
    std::uint64_t file_io_time_ns() const;

  private:
    std::vector<BulletView> views_;
};
} // namespace th08::headless
