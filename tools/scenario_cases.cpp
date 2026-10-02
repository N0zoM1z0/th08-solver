#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <th08/scenario.hpp>

namespace scene = th08::scenario;
namespace {
struct Arguments {
    std::string scenario = "all", strategy = "all", format = "json", output;
    std::uint32_t duration = 7200, visual_draws = 2;
    std::uint16_t seed = 1;
    scene::RunOptions options;
};
std::uint64_t integer(const std::string &text, std::uint64_t maximum) {
    std::size_t consumed = 0;
    if (text.empty() || text.front() == '-')
        throw std::invalid_argument("expected unsigned integer: " + text);
    const auto value = std::stoull(text, &consumed, 0);
    if (consumed != text.size() || value > maximum)
        throw std::invalid_argument("integer out of range: " + text);
    return value;
}
float real(const std::string &text) {
    std::size_t consumed = 0;
    const auto value = std::stof(text, &consumed);
    if (consumed != text.size() || !std::isfinite(value))
        throw std::invalid_argument("expected finite number: " + text);
    return value;
}
std::string hex(std::uint64_t value) {
    std::ostringstream text;
    text << std::hex << std::setfill('0') << std::setw(16) << value;
    return text.str();
}
struct Record {
    std::shared_ptr<const scene::Definition> definition;
    scene::RunOptions options;
    scene::RunResult result;
};
void json_record(std::ostream &out, const Record &record, const Arguments &args) {
    const auto &r = record.result;
    const auto &o = record.options;
    out << "    {\"scenario\":\"" << record.definition->name << "\",\"strategy\":\""
        << scene::name(o.strategy) << "\",\"seed\":" << args.seed
        << ",\"visual_seed\":" << std::uint16_t(args.seed ^ 0xa5a5U)
        << ",\"visual_draws_per_frame\":" << args.visual_draws
        << ",\"duration_frames\":" << record.definition->duration() << ",\"horizon\":" << o.horizon
        << ",\"commit_frames\":" << o.commit_frames << ",\"beam\":" << o.beam
        << ",\"expansions_per_plan\":" << o.expansions_per_plan << ",\"goal\":[" << o.goal.x << ','
        << o.goal.y << ']' << ",\"recover_goal\":" << (o.recover_goal ? "true" : "false")
        << ",\"outcome\":\"" << scene::name(r.outcome) << "\",\"search_status\":\""
        << (o.strategy == scene::Strategy::rolling_beam ? scene::name(r.search_status) : "not_run")
        << "\",\"executed_frames\":" << r.completed_frames
        << ",\"survived_frames\":" << r.completed_frames - (r.death_frame ? 1 : 0)
        << ",\"death_frame\":" << r.death_frame << ",\"search_stop_frame\":" << r.search_stop_frame
        << ",\"decisions\":" << r.decisions << ",\"expansions\":" << r.expansions
        << ",\"collision_queries\":" << r.collision_queries
        << ",\"duplicate_successors\":" << r.duplicate_successors
        << ",\"peak_decision_expansions\":" << r.peak_decision_expansions
        << ",\"recovery_attempts\":" << r.recovery_attempts
        << ",\"recovery_successes\":" << r.recovery_successes
        << ",\"recovery_target_queries\":" << r.recovery_target_queries
        << ",\"recovery_initial_expansions\":" << r.recovery_initial_expansions
        << ",\"recovery_retry_expansions\":" << r.recovery_retry_expansions
        << ",\"forecast_frames\":" << r.forecast_frames
        << ",\"peak_model_frames\":" << r.peak_model_frames
        << ",\"peak_model_bullet_references\":" << r.peak_model_bullet_references
        << ",\"peak_live_bullets\":" << r.peak_live_bullets
        << ",\"emitted_bullets\":" << r.emitted_bullets
        << ",\"emission_events\":" << r.emission_events
        << ",\"gameplay_draws\":" << r.gameplay_draws << ",\"visual_draws\":" << r.visual_draws
        << ",\"final_player\":[" << r.player.x << ',' << r.player.y << ']' << ",\"world_digest\":\""
        << hex(r.digests.world) << "\",\"route_digest\":\"" << hex(r.route_digest)
        << "\",\"event_digest\":\"" << hex(r.digests.events) << "\",\"gameplay_rng_digest\":\""
        << hex(r.digests.gameplay_rng) << "\",\"visual_rng_digest\":\"" << hex(r.digests.visual_rng)
        << "\",\"action_tape_bytes\":" << r.actions.size()
        << ",\"fresh_unindexed_replay_verified\":" << (r.replay_verified ? "true" : "false")
        << ",\"generation_ms\":" << r.generation_ms << ",\"search_ms\":" << r.search_ms
        << ",\"recovery_initial_ms\":" << r.recovery_initial_ms
        << ",\"recovery_target_ms\":" << r.recovery_target_ms
        << ",\"recovery_retry_ms\":" << r.recovery_retry_ms
        << ",\"execution_ms\":" << r.execution_ms << ",\"replay_ms\":" << r.replay_ms
        << ",\"total_ms\":" << r.total_ms << ",\"phases\":[";
    for (std::size_t i = 0; i < record.definition->phases.size(); ++i) {
        if (i)
            out << ',';
        const auto &phase = record.definition->phases[i];
        out << "{\"pattern\":\"" << scene::name(phase.pattern)
            << "\",\"end_frame\":" << phase.end_frame << '}';
    }
    out << "],\"transitions\":[";
    for (std::size_t i = 0; i < r.transitions.size(); ++i) {
        if (i)
            out << ',';
        const auto &t = r.transitions[i];
        out << "{\"frame\":" << t.frame << ",\"phase\":" << t.phase
            << ",\"carried_bullets\":" << t.carried_bullets
            << ",\"gameplay_draws_before\":" << t.gameplay_draws
            << ",\"visual_draws_before\":" << t.visual_draws << '}';
    }
    out << "],\"actions\":[";
    for (std::size_t i = 0; i < r.actions.size(); ++i) {
        if (i)
            out << ',';
        out << unsigned(r.actions[i]);
    }
    out << "]}";
}
void output(std::ostream &out, const std::vector<Record> &records, const Arguments &args) {
    out << std::setprecision(9);
    if (args.format == "json") {
        out << "{\n  \"schema\":1,\n  \"classification\":\"synthetic-controlled-scenario\",\n"
               "  \"retail_equivalent\":false,\n"
               "  \"dependency\":\"candidate-independent emitters only\",\n"
               "  \"visual_policy\":\"separate seeded next_u16 hook; no visual consumer "
               "reconstruction\",\n"
               "  \"frame_convention\":\"1-based collision/death frames; phase endpoints are "
               "exclusive update indices\",\n"
               "  \"action_encoding\":\"(y+1)*3+x+1; x,y in {-1,0,1}; 4=stay; JSON includes the "
               "executed action tape\",\n"
               "  \"timing\":\"generation includes committed simulation and lookahead; search "
               "includes planner validation and optional recovery; recovery timings are subsets "
               "of search; replay is fresh unindexed simulation; no timed file "
               "I/O\",\n"
               "  \"counterexample\":\"late-gate exposes center-seeking finite-beam failure; "
               "--goal-x 24 supplies an explicit escape witness; --recover-goal 1 retries "
               "toward a forecast-derived refuge, with no completeness claim\",\n"
               "  \"recovery_policy\":\"off by default; at most one retry after search_exhausted; "
               "nearest 8-pixel grid point free throughout lookahead; capped retry beam shares "
               "the original expansion budget per decision; target queries counted separately\",\n"
               "  \"results\":[\n";
        for (std::size_t i = 0; i < records.size(); ++i) {
            if (i)
                out << ",\n";
            json_record(out, records[i], args);
        }
        out << "\n  ]\n}\n";
        return;
    }
    out << "classification\tscenario\tstrategy\tseed\tvisual_draws_per_frame\tduration_frames"
           "\thorizon\tcommit_frames\tbeam\texpansions_per_plan\tgoal_x\tgoal_y\trecover_goal"
           "\toutcome"
           "\tsearch_status\texecuted_frames\tsurvived_frames\tdeath_frame\tsearch_stop_frame"
           "\tdecisions\texpansions\tcollision_queries\tduplicate_successors"
           "\tpeak_decision_expansions\trecovery_attempts\trecovery_successes"
           "\trecovery_target_queries\trecovery_initial_expansions\trecovery_retry_expansions"
           "\tforecast_frames\tpeak_model_frames\tpeak_model_bullet_references"
           "\tpeak_live_bullets\temitted_bullets\temission_events\tgameplay_draws\tvisual_draws"
           "\tworld_digest\troute_digest\tevent_digest\tgameplay_rng_digest\tvisual_rng_digest"
           "\treplay_verified\tgeneration_ms\tsearch_ms\trecovery_initial_ms\trecovery_target_ms"
           "\trecovery_retry_ms\texecution_ms\treplay_ms\ttotal_ms\n";
    for (const auto &record : records) {
        const auto &r = record.result;
        const auto &o = record.options;
        out << "synthetic-controlled\t" << record.definition->name << '\t'
            << scene::name(o.strategy) << '\t' << args.seed << '\t' << args.visual_draws << '\t'
            << record.definition->duration() << '\t' << o.horizon << '\t' << o.commit_frames << '\t'
            << o.beam << '\t' << o.expansions_per_plan << '\t' << o.goal.x << '\t' << o.goal.y
            << '\t' << o.recover_goal << '\t' << scene::name(r.outcome) << '\t'
            << (o.strategy == scene::Strategy::rolling_beam ? scene::name(r.search_status)
                                                            : "not_run")
            << '\t' << r.completed_frames << '\t' << r.completed_frames - (r.death_frame ? 1 : 0)
            << '\t' << r.death_frame << '\t' << r.search_stop_frame << '\t' << r.decisions << '\t'
            << r.expansions << '\t' << r.collision_queries << '\t' << r.duplicate_successors << '\t'
            << r.peak_decision_expansions << '\t' << r.recovery_attempts << '\t'
            << r.recovery_successes << '\t' << r.recovery_target_queries << '\t'
            << r.recovery_initial_expansions << '\t' << r.recovery_retry_expansions << '\t'
            << r.forecast_frames << '\t' << r.peak_model_frames << '\t'
            << r.peak_model_bullet_references << '\t' << r.peak_live_bullets << '\t'
            << r.emitted_bullets << '\t' << r.emission_events << '\t' << r.gameplay_draws << '\t'
            << r.visual_draws << '\t' << hex(r.digests.world) << '\t' << hex(r.route_digest) << '\t'
            << hex(r.digests.events) << '\t' << hex(r.digests.gameplay_rng) << '\t'
            << hex(r.digests.visual_rng) << '\t' << r.replay_verified << '\t' << r.generation_ms
            << '\t' << r.search_ms << '\t' << r.recovery_initial_ms << '\t' << r.recovery_target_ms
            << '\t' << r.recovery_retry_ms << '\t' << r.execution_ms << '\t' << r.replay_ms << '\t'
            << r.total_ms << '\n';
    }
}
} // namespace
int main(int argc, char **argv) {
    try {
        Arguments args;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--help") {
                std::cout
                    << "Offline synthetic continuous-scenario benchmark; no game or DAT required.\n"
                       "--scenario all|relay|lane-switch|late-gate --strategy "
                       "all|stationary|greedy|rolling-beam\n"
                       "--seed 0..65535 --duration 3..10000000 --horizon 1..4096 --commit FRAMES\n"
                       "--beam 1..4096 --budget EXPANSIONS_PER_PLAN --goal-x X --goal-y Y\n"
                       "--recover-goal 0|1 (rolling beam only; retries share the decision budget)\n"
                       "--visual-draws 0..32 --format json|tsv --output PATH\n"
                       "Defaults: all profiles/strategies, seed 1, 7200 frames, horizon 120, "
                       "commit 30, beam 128, goal recovery off.\n"
                       "Exit 0 includes reported collision/search_limit outcomes; invalid input or "
                       "replay disagreement exits 1.\n";
                return 0;
            }
            if (++i == argc)
                throw std::invalid_argument("missing value for " + option);
            const std::string value = argv[i];
            if (option == "--scenario")
                args.scenario = value;
            else if (option == "--strategy")
                args.strategy = value;
            else if (option == "--format")
                args.format = value;
            else if (option == "--output")
                args.output = value;
            else if (option == "--seed")
                args.seed = std::uint16_t(integer(value, 65535));
            else if (option == "--duration")
                args.duration = std::uint32_t(integer(value, 10000000));
            else if (option == "--visual-draws")
                args.visual_draws = std::uint32_t(integer(value, 32));
            else if (option == "--horizon")
                args.options.horizon = std::uint32_t(integer(value, 4096));
            else if (option == "--commit")
                args.options.commit_frames = std::uint32_t(integer(value, 4096));
            else if (option == "--beam")
                args.options.beam = std::size_t(integer(value, 4096));
            else if (option == "--budget")
                args.options.expansions_per_plan = integer(value, UINT64_MAX);
            else if (option == "--goal-x")
                args.options.goal.x = real(value);
            else if (option == "--goal-y")
                args.options.goal.y = real(value);
            else if (option == "--recover-goal")
                args.options.recover_goal = integer(value, 1) != 0;
            else
                throw std::invalid_argument("unknown option: " + option);
        }
        if (args.format != "json" && args.format != "tsv")
            throw std::invalid_argument("format must be json or tsv");
        std::vector<scene::Strategy> strategies;
        if (args.strategy == "all" || args.strategy == "stationary")
            strategies.push_back(scene::Strategy::stationary);
        if (args.strategy == "all" || args.strategy == "greedy")
            strategies.push_back(scene::Strategy::greedy);
        if (args.strategy == "all" || args.strategy == "rolling-beam")
            strategies.push_back(scene::Strategy::rolling_beam);
        if (strategies.empty())
            throw std::invalid_argument("unknown strategy: " + args.strategy);
        const auto names = args.scenario == "all" ? scene::profile_names()
                                                  : std::vector<std::string>{args.scenario};
        std::vector<Record> records;
        for (const auto &name : names) {
            auto definition = scene::profile(name, args.duration, args.visual_draws);
            const auto initial = scene::checkpoint(definition, args.seed);
            for (const auto strategy : strategies) {
                auto options = args.options;
                options.strategy = strategy;
                records.push_back({definition, options, scene::run(initial, options)});
            }
        }
        if (args.output.empty()) {
            output(std::cout, records, args);
        } else {
            std::ofstream file(args.output);
            file.exceptions(std::ios::failbit | std::ios::badbit);
            output(file, records, args);
        }
    } catch (const std::exception &error) {
        std::cerr << "scenario benchmark: " << error.what() << '\n';
        return 1;
    }
}
