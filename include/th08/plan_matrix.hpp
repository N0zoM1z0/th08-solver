#pragma once

#include "th08/plan_contracts.hpp"
#include <array>

namespace th08::plan {

inline constexpr const char *shot_names[] = {
    "reimu-yukari", "marisa-alice", "sakuya-remilia", "youmu-yuyuko", "reimu", "yukari",
    "marisa",       "alice",        "sakuya",         "remilia",      "youmu", "yuyuko"};
inline constexpr const char *difficulty_names[] = {"easy", "normal", "hard", "lunatic", "extra"};

struct MatrixCase {
    int shot, difficulty;
    FinalRoute route;
    int stage() const {
        return difficulty == 4 ? 8 : 0;
    }
    int final_stage() const {
        return difficulty == 4 ? 8 : route == FinalRoute::A ? 6 : 7;
    }
    int fourth_stage() const {
        // Original AdvanceToNextStage groups teams and their two solo members.
        const int team = shot < 4 ? shot : (shot - 4) / 2;
        return team == 0 || team == 3 ? 4 : 3;
    }
};

inline std::array<MatrixCase, 108> full_run_matrix() {
    std::array<MatrixCase, 108> cases{};
    std::size_t next = 0;
    for (int shot = 0; shot < 12; ++shot) {
        for (int difficulty = 0; difficulty < 4; ++difficulty)
            for (const auto route : {FinalRoute::A, FinalRoute::B})
                cases[next++] = {shot, difficulty, route};
        cases[next++] = {shot, 4, FinalRoute::Natural};
    }
    return cases;
}

} // namespace th08::plan
