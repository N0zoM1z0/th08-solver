#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace th08::phase {
enum class Fire { Feedback, Continuous };
enum class Capture { Human, Youkai };
// A phase is selected by actual Native stage and one-based spell card, never a
// replay-row ordinal. Geometry is in source pixels; rows are spaced32px apart.
// The reserve is source gameplay updates, not milliseconds. This is a proposal
// schedule: it owns no simulation, RNG, HP, option anchor or safety certificate.
struct Strategy {
    int stage = 0, card = 1, top = 208, rows = 7;
    unsigned reserve = 600;
    Fire fire = Fire::Feedback;
    Capture capture = Capture::Youkai;
    int bottom() const {
        return top + 32 * (rows - 1);
    }
};
class Plan {
  public:
    explicit Plan(std::vector<Strategy> strategies);
    const Strategy *find(int stage, int zero_based_spell) const;
    const std::vector<Strategy> &entries() const {
        return entries_;
    }

  private:
    std::vector<Strategy> entries_;
};
// Compact CLI grammar: stage:card:top:rows:reserve:feedback|shoot:human|youkai,
// repeated with commas. "none" is the explicit empty/default plan. Parse fully
// before creating a Native session; malformed/duplicate selectors fail closed.
Plan parse(const std::string &text);
std::uint16_t propose(const Strategy &, int remaining, float player_y, bool extreme_youkai,
                      std::uint16_t movement_input);
} // namespace th08::phase
