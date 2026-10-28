#include "th08/online.hpp"
#include "th08/phase_archive.hpp"
#include "th08/phase_strategy.hpp"
#include <iostream>
#include <stdexcept>

void check(bool b, const char *s) {
    if (!b)
        throw std::runtime_error(s);
}
template <class F> void rejects(F f) {
    bool bad = false;
    try {
        f();
    } catch (const std::exception &) {
        bad = true;
    }
    check(bad, "accepted invalid phase evidence");
}
int main() try {
    using namespace th08::phase;
    const auto plan = parse("8:205:400:2:0:feedback:youkai,0:6:16:7:600:shoot:human");
    check(!plan.find(0, 6) && plan.find(0, 5) && !plan.find(1, 5),
          "phase selectors use actual stage/card");
    const auto &s = *plan.find(8, 204);
    check(s.bottom() == 432, "source row units");
    check(propose(s, 100, 432, true, 4096 | 64 | 1) == (4096 | 64 | 4),
          "feedback preserves movement/dialogue");
    check(propose(s, 100, 433, true, 5) == 0, "approach releases focus/fire");
    check(propose(*plan.find(0, 5), 600, 200, true, 4 | 32) == (32 | 1),
          "capture form and exact boundary");
    for (const auto *text :
         {"", "8:0:16:7:600:shoot:youkai", "8:205:400:3:0:feedback:youkai",
          "8:205:0:15:0:feedback:youkai", "8:205:16:7:0:bad:human", "8:205:16:7:0:shoot:human,",
          "8:205:16:7:0:shoot:human,8:205:16:7:0:shoot:human"})
        rejects([&] { parse(text); });
    auto config = th08::online::with_style("score-harvest");
    config.scoring_attack_pacing = true;
    config.phase_plan = std::make_shared<const Plan>(plan);
    th08::online::Policy policy(config);
    th08::online::Observation o;
    o.stage = 8;
    o.spell = 204;
    o.spell_active = true;
    o.y = 432;
    o.scoring.available = o.scoring.has_boss = o.scoring.can_switch_forms =
        o.scoring.extreme_youkai = true;
    o.scoring.spell_remaining = 100;
    check(policy.phase_input(o, 1) == 4, "owned policy schedule");
    o.spell_active = false;
    check(policy.phase_input(o, 1) == 1, "no schedule outside actual spell");
    check(parse("none").entries().empty(), "explicit default");
    auto make = [](const char *id, const char *family, std::uint64_t score, char world) {
        return Candidate{
            id,   "extra203", family, std::string(64, 'a'), std::string(64, world), score,
            true, true,       0};
    };
    auto a = make("high", "feedback", 100, '1'), b = make("low", "shoot", 1, '2'),
         a2 = make("second", "feedback", 99, '3');
    auto dead = make("dead", "other", 1000, '4');
    dead.no_miss = false;
    auto incomplete = make("partial", "other", 999, '5');
    incomplete.completed = false;
    auto selected = frontier({a, a2, b, dead, incomplete}, 2, 0);
    check(selected.size() == 2 && selected[0].id == "high" && selected[1].id == "low",
          "preserve strategy diversity despite scalar dominance");
    check(frontier({a, b}, 1, 1)[0].id == "low", "rotate bounded admission");
    auto alias = a;
    alias.id = "zalias";
    check(frontier({a, alias, b}, 8, 0).size() == 2, "dedupe exact result identity");
    auto retry = a;
    retry.id = "retry";
    retry.checkpoint_key = std::string(64, '9');
    check(frontier({a, retry}, 8, 0).size() == 2, "different realized worlds are not aliases");
    alias.score++;
    rejects([&] { frontier({a, alias}, 8, 0); });
    b.boundary = "story171";
    rejects([&] { frontier({a, b}, 2, 0); });
    check(frontier({dead, incomplete}, 2, 0).empty(),
          "failed candidates never certify archive entries");
    std::cout << "phase strategy/archive contracts pass\n";
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
