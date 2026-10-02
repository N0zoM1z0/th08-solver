#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <th08/spell_scenario.hpp>

namespace spell = th08::spell;
namespace fs = std::filesystem;
namespace {
const char *search_name(th08::solver::Status status) {
    using S = th08::solver::Status;
    switch (status) {
    case S::found:
        return "FOUND";
    case S::unsupported_dependency:
        return "UNSUPPORTED_DEPENDENCY";
    case S::invalidated:
        return "INVALIDATED";
    case S::invalid_argument:
        return "INVALID_ARGUMENT";
    case S::expansion_limit:
        return "EXPANSION_LIMIT";
    case S::search_exhausted:
        return "SEARCH_EXHAUSTED";
    case S::no_terminal_witness:
        return "NO_TERMINAL_WITNESS";
    }
    return "UNKNOWN";
}
void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
spell::World completed(const spell::Program &program, std::uint16_t gameplay,
                       std::uint16_t visual) {
    spell::World world(program, gameplay, visual);
    for (unsigned tick = 0; tick < spell::duration; ++tick)
        world.advance();
    world.finish_timeout();
    return world;
}
void verify(const spell::Program &program) {
    auto reference = completed(program, 0, 0);
    const auto &stats = reference.statistics();
    require(stats.frame == 1200 && stats.child_start == 162 && stats.shot_commands == 140 &&
                stats.allocated == 1120 && stats.timeout_entered && stats.spell_ended &&
                stats.boss_removed && stats.first_lethal == 171 &&
                stats.gameplay_draws_before_callback == 4 && stats.last_shot == 1197 &&
                stats.allocated == stats.retired + stats.despawning && stats.despawning > 0 &&
                reference.gameplay_rng().generation_count == 6,
            "ID179 full schedule/terminal regression");
    const auto visual = completed(program, 0, 1);
    require(visual.statistics().hazard_digest == stats.hazard_digest &&
                visual.statistics().visual_digest != stats.visual_digest &&
                visual.gameplay_rng().seed == reference.gameplay_rng().seed,
            "visual hook contaminated controlled gameplay stream");
    const auto gameplay = completed(program, 1, 0);
    require(gameplay.statistics().hazard_digest != stats.hazard_digest,
            "gameplay seed did not alter actual DAT random angles");
    spell::World original(program, 0, 0);
    for (unsigned tick = 0; tick < 231; ++tick)
        original.advance();
    spell::World fork = original;
    for (unsigned tick = 231; tick < 1200; ++tick) {
        original.advance();
        fork.advance();
        require(original.state_digest() == fork.state_digest(), "checkpoint fork diverged");
    }
    original.finish_timeout();
    fork.finish_timeout();
    require(original.state_digest() == reference.state_digest() &&
                fork.state_digest() == reference.state_digest(),
            "resumed world differs from uninterrupted world");
    auto unsupported = program;
    unsupported.ecl.subs[72].code[14].opcode = 177;
    bool rejected = false;
    try {
        spell::World world(unsupported);
        world.advance();
    } catch (const std::runtime_error &) {
        rejected = true;
    }
    require(rejected, "unknown gameplay opcode was silently acknowledged");
    const auto limited = spell::solve(program, {120, 30, 16, 0});
    require(limited.status == "SEARCH_LIMIT" &&
                limited.search_status == th08::solver::Status::expansion_limit &&
                limited.actions.empty() && limited.replayed,
            "zero-budget failure-prefix replay/report mismatch");
}
void write_case(const spell::Program &program, const char *name, const spell::Options &options,
                const fs::path &directory, std::ostream &summary, std::uint16_t seed,
                int baseline = -1) {
    const auto start = std::chrono::steady_clock::now();
    const auto result = baseline < 0 ? spell::solve(program, options, seed)
                                     : spell::baseline(program, baseline != 0, seed);
    const double milliseconds =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    const auto &s = result.statistics;
    std::ofstream route(directory / (std::string("spell179_") + name + "_seed" +
                                     std::to_string(seed) + "_route.tsv"));
    route.exceptions(std::ios::badbit | std::ios::failbit);
    route << std::setprecision(17) << "tick\taction_x\taction_y\tx\ty\n";
    for (std::size_t i = 0; i < result.actions.size(); ++i)
        route << i << '\t' << result.actions[i].x << '\t' << result.actions[i].y << '\t'
              << result.positions[i].x << '\t' << result.positions[i].y << '\n';
    summary << "{\"algorithm\":\"" << name << "\",\"gameplay_seed\":" << seed
            << ",\"horizon\":" << options.horizon << ",\"commit\":" << options.commit
            << ",\"beam\":" << options.beam
            << ",\"expansion_budget_per_plan\":" << options.expansion_budget
            << ",\"search_status\":\""
            << (baseline < 0 ? search_name(result.search_status) : "NOT_USED") << "\",\"status\":\""
            << result.status << "\",\"frames\":" << s.frame
            << ",\"requested_bullets\":" << s.shot_commands * 8
            << ",\"allocated_bullets\":" << s.allocated << ",\"retired_bullets\":" << s.retired
            << ",\"despawning_occupied_slots\":" << s.despawning
            << ",\"peak_lethal\":" << s.peak_lethal << ",\"replans\":" << result.replans
            << ",\"expansions\":" << result.expansions
            << ",\"collision_queries\":" << result.collision_queries
            << ",\"solve_and_regenerated_replay_ms\":" << milliseconds << ",\"hazard_digest\":\""
            << s.hazard_digest << "\",\"world_digest\":\"" << result.world_digest
            << "\",\"gameplay_rng_draws\":" << result.gameplay.generation_count
            << ",\"gameplay_draws_before_callback\":" << s.gameplay_draws_before_callback
            << ",\"visual_rng_draws\":" << result.visual.generation_count
            << ",\"callback_end_reached\":" << (s.spell_ended && s.boss_removed ? "true" : "false")
            << ",\"regenerated_unindexed_replay\":" << (result.replayed ? "true" : "false")
            << ",\"full_duration_replayed\":"
            << (result.replayed && result.status == "SURVIVED_CONTROLLED_TIMEOUT" ? "true"
                                                                                  : "false")
            << '}';
    if (result.replayed && !result.actions.empty()) {
        auto invalid = result;
        invalid.actions[0].x = 2;
        require(!spell::replay(program, invalid, seed), "replay accepted an illegal action");
        invalid = result;
        invalid.actions.pop_back();
        require(!spell::replay(program, invalid, seed), "replay accepted an incomplete route");
    }
    std::cout << name << " seed=" << seed << ": " << result.status << ", frames=" << s.frame
              << ", allocated=" << s.allocated << ", replay=" << result.replayed
              << ", ms=" << milliseconds << '\n';
}
} // namespace
int main(int argc, char **argv) try {
    require(argc == 3 || argc == 4,
            "Usage: th08_spell_cases th08.dat report-directory [--verify-only]");
    const spell::Program program(argv[1]);
    const fs::path directory = argv[2];
    fs::create_directories(directory);
    verify(program);
    if (argc == 4) {
        require(std::string(argv[3]) == "--verify-only", "unknown option");
        std::cout
            << "ID179 controlled world, terminal, RNG isolation and checkpoint checks passed\n";
        return 0;
    }
    std::ofstream summary(directory / "spell179_summary.json");
    summary.exceptions(std::ios::badbit | std::ios::failbit);
    summary << std::setprecision(17)
            << "{\"scope\":\"DAT-derived controlled complete survival segment; not retail capture "
               "or exact practice-wrapper equivalence\",\"profile\":\"ID179 Easy, supplied "
               "sub72 PC10 checkpoint, callback1, focused ply00a movement only, no shots/bombs/"
               "form changes, no graze-score-item feedback, isolated seeded visual hooks\","
               "\"visual_hook\":\"one separate-stream U32 per requested visual particle\","
               "\"digest_scope\":\"hazard trace and selected lifecycle state; not a full "
               "state-equivalence key\","
               "\"frame_convention\":\"route ticks are zero-based lethal phases; frames is "
               "completed action count\","
               "\"dat_sha256\":\""
            << program.dat_hash
            << "\",\"member\":\"ecldata7sp.ecl\","
               "\"member_sha256\":\""
            << program.ecl_hash << "\",\"anm_sha256\":\"" << program.anm_hash
            << "\",\"sht_sha256\":\"" << program.sht_hash
            << "\",\"spell_id\":179,\"sub\":72,\"pc\":10,\"offset\":56084,"
               "\"instruction_mask\":241,\"execution_mask\":1,\"duration_frames\":1200,"
               "\"initial_visual_seed\":0,\"cases\":[";
    bool first = true;
    for (std::uint16_t seed : {0, 1, 65535}) {
        if (!first)
            summary << ',';
        first = false;
        write_case(program, "stationary", {1, 1, 1, 0}, directory, summary, seed, 0);
        summary << ',';
        write_case(program, "greedy", {1, 1, 1, 0}, directory, summary, seed, 1);
        summary << ',';
        write_case(program, "full_horizon", {1200, 1200, 128, 2000000}, directory, summary, seed);
        summary << ',';
        write_case(program, "rolling", {180, 30, 128, 300000}, directory, summary, seed);
    }
    summary << "],\"retail_equivalent_complete_spells\":0}\n";
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
