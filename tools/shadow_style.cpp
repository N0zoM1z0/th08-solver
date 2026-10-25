// Synchronous Native experiment for complete-duration style comparisons. It
// retains the search budgets and immutable eight-input lead. Verification and
// publication horizons are explicit; computation pauses this owner, so its
// throughput is not an online FPS result.
#include "ReplayManager.hpp"
#include "modern/headless/session.hpp"
#include "score_audit_report.hpp"
#include "th08/online_native.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_dialogue.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_plan.hpp"
#include "th08/shadow_proposal.hpp"
#include <chrono>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace {
namespace s = th08::shadow;
namespace o = th08::online;
namespace h = th08::headless;
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
constexpr unsigned lead = 8;
struct SearchOptions {
    unsigned horizon = 64, publication = 64, budget_ms = 110;
    bool held_recovery = false, baseline_recovery = false;
};
struct Work {
    std::uint64_t searches = 0, candidates = 0, branch_updates = 0, proposal_updates = 0;
    std::uint64_t future_deaths = 0, timeouts = 0, nominal_adoptions = 0, fresh_adoptions = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0, guidance_ns = 0, overruns = 0;
    std::uint64_t held_attempts = 0, held_adoptions = 0;
    std::uint64_t safety_attempts = 0, safety_adoptions = 0;
    double total_ms = 0, maximum_ms = 0;
    void branch(const s::Prediction &p) {
        ++candidates;
        branch_updates += p.charged_updates;
        native_ns += p.native_ns;
        file_io_ns += p.file_io_ns;
        guidance_ns += p.guidance_ns;
        future_deaths += p.status == s::BranchStatus::Death;
        timeouts += p.status == s::BranchStatus::Timeout;
        if (p.status == s::BranchStatus::Fault)
            throw std::runtime_error("native_style_branch_fault");
    }
};
s::Prediction search(h::Session &session, o::Policy &policy, o::Policy &safety,
                     const s::ActionBuffer &queue, const s::Prediction &previous,
                     s::NominalExtension &nominal, s::Boundary boundary,
                     const SearchOptions &options, Work &work) {
    const auto horizon = options.horizon, publication = options.publication,
               budget_ms = options.budget_ms;
    const bool held_recovery = options.held_recovery;
    const auto begin = Clock::now();
    ++work.searches;
    const auto elapsed = [&] {
        return std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    };
    const auto remaining = [&] {
        const auto ms = elapsed();
        return ms < budget_ms ? std::max(1u, unsigned(budget_ms - ms)) : 0u;
    };
    const auto finish = [&](s::Prediction result) {
        const auto ms = elapsed();
        work.total_ms += ms;
        work.maximum_ms = std::max(work.maximum_ms, ms);
        work.overruns += ms > budget_ms;
        return result;
    };
    const auto &origin = queue.frontier();
    if (nominal.eligible(previous, queue, lead, publication)) {
        std::vector<std::uint16_t> warm;
        for (auto i = origin.sequence - previous.origin.sequence; i < previous.frames.size(); ++i)
            warm.push_back(std::uint16_t(previous.frames[i].input));
        auto p = s::predict_with_policy(session, policy, origin, warm, horizon, boundary, budget_ms,
                                        true);
        work.branch(p);
        nominal.observe_attempt(previous, p);
        if (p.certifies()) {
            ++work.nominal_adoptions;
            return finish(std::move(p));
        }
    }
    std::vector<std::uint16_t> prefix;
    auto last = std::uint16_t(origin.input);
    for (unsigned i = 0; i < lead; ++i) {
        const auto sequence = origin.sequence + i + 1;
        last = s::bridge::fallback(last, sequence, false);
        if (previous.certifies() && queue.size() > i && sequence > previous.origin.sequence &&
            sequence <= previous.origin.sequence + previous.frames.size())
            last = std::uint16_t(previous.frames[sequence - previous.origin.sequence - 1].input);
        prefix.push_back(last);
    }
    if (options.baseline_recovery && !previous.frames.empty() && remaining()) {
        ++work.safety_attempts;
        auto repair = s::predict_with_policy(session, safety, origin, prefix, horizon, boundary,
                                             remaining(), true, &policy);
        work.branch(repair);
        if (repair.certifies()) {
            ++work.fresh_adoptions;
            ++work.safety_adoptions;
            return finish(std::move(repair));
        }
    }
    if (!remaining())
        return finish({});
    const auto proposals = s::propose_after_lead(session, policy, origin, prefix, boundary,
                                                 std::min(60u, remaining()), true);
    work.proposal_updates += proposals.charged_updates;
    work.native_ns += proposals.native_ns;
    work.file_io_ns += proposals.file_io_ns;
    work.future_deaths += proposals.status == s::BranchStatus::Death;
    work.timeouts += proposals.status == s::BranchStatus::Timeout;
    if (proposals.status == s::BranchStatus::Fault)
        throw std::runtime_error("native_style_proposal_fault");
    std::vector<std::vector<std::uint16_t>> candidates;
    for (const auto &path : proposals.inputs) {
        auto inputs = prefix;
        inputs.insert(inputs.end(), path.begin(), path.end());
        candidates.push_back(std::move(inputs));
    }
    for (unsigned focus : {4u, 0u})
        for (unsigned direction : {0u, 16u, 32u, 64u, 128u, 80u, 144u, 96u, 160u}) {
            auto inputs = prefix;
            inputs.resize(lead + 24, std::uint16_t(1 | focus | direction));
            candidates.push_back(std::move(inputs));
        }
    const bool prefer_held = held_recovery && !previous.frames.empty();
    if (prefer_held)
        std::rotate(candidates.begin(), candidates.begin() + proposals.inputs.size(),
                    candidates.end());
    s::Prediction selected;
    const auto evaluate = [&](bool held) {
        for (auto &inputs : candidates) {
            for (unsigned i = lead; i < inputs.size(); ++i)
                inputs[i] = std::uint16_t((inputs[i] & ~0x1000) |
                                          (((origin.sequence + i + 1) & 1) ? 0x1000 : 0));
            if (!remaining())
                break;
            work.held_attempts += held;
            auto p = held ? s::predict_with_hold(session, origin, inputs, horizon, boundary,
                                                 std::max(1u, remaining()), true)
                          : s::predict_with_policy(session, policy, origin, inputs, horizon,
                                                   boundary, std::max(1u, remaining()), true);
            work.branch(p);
            if (p.certifies()) {
                ++work.fresh_adoptions;
                work.held_adoptions += held;
                selected = std::move(p);
                return;
            }
        }
    };
    evaluate(prefer_held);
    if (held_recovery && !selected.certifies())
        evaluate(!prefer_held);
    return finish(std::move(selected));
}
unsigned number(const char *text, unsigned maximum) {
    const std::string value(text);
    std::size_t used = 0;
    const auto n = std::stoul(value, &used);
    if (used != value.size() || value.empty() || value.front() == '-' || n > maximum)
        throw std::runtime_error("invalid_native_style_argument");
    return unsigned(n);
}
float preference(const char *text) {
    const std::string value(text);
    std::size_t used = 0;
    const float n = std::stof(value, &used);
    if (used != value.size() || !std::isfinite(n) || n < 0 || n > 10000)
        throw std::runtime_error("invalid_native_score_preference");
    return n;
}
int run(int argc, char **argv) {
    if (argc < 11 || (argc - 11) % 2 != 0)
        throw std::runtime_error(
            "usage: th08_shadow_style DAT OUTPUT_DIR STAGE DIFFICULTY SHOT "
            "SEED STYLE MAX_UPDATES STORY WALL_SECONDS "
            "[--collect-items 0|1] [--aim-boss 0|1] "
            "[--survival-focus 0|1] [--verify-horizon 64|128|256] "
            "[--publish-horizon 64|128] [--held-recovery 0|1] "
            "[--boss-collect 0|1] [--precollect 0|1] [--survival-bias VALUE] "
            "[--survival-flow 0|1] [--attack-pacing 0|1] "
            "[--attack-focus-bias VALUE] [--baseline-recovery 0|1] "
            "[--search-budget-ms VALUE] [--priced-collection 0|1] "
            "[--upper-stage 0|1] [--collection-bias VALUE] [--human-bias VALUE] "
            "[--capture-factor VALUE] [--capture-padding UPDATES]");
    const auto dat = fs::absolute(argv[1]).string();
    const auto output = fs::absolute(argv[2]);
    const std::string style = argv[7];
    const auto maximum_updates = number(argv[8], 400000), seconds = number(argv[10], 7200);
    if (!maximum_updates || !seconds)
        throw std::runtime_error("empty_native_style_budget");
    h::Config config;
    config.dat_path = dat.c_str();
    config.stage = int(number(argv[3], 8));
    config.difficulty = int(number(argv[4], 4));
    config.shot = int(number(argv[5], 11));
    config.seed = std::uint16_t(number(argv[6], 65535));
    config.story = number(argv[9], 1);
    config.prior_final_a_clear = config.story && config.stage != 8;
    config.input_profile = h::InputProfile::SampledGameplay;
    auto profile = o::with_style(style);
    unsigned horizon = 64, publication = 64;
    bool held_recovery = false;
    bool collection_override = false, aim_override = false, horizon_override = false;
    bool form_override = false;
    bool publication_override = false, held_override = false;
    bool boss_collection_override = false, precollect_override = false;
    bool survival_bias_override = false;
    bool survival_flow_override = false;
    bool pacing_override = false, attack_bias_override = false, recovery_override = false;
    bool budget_override = false, baseline_recovery = false;
    bool price_override = false, upper_override = false, collection_bias_override = false;
    bool human_bias_override = false;
    bool reserve_factor_override = false, reserve_updates_override = false;
    unsigned budget_ms = 110;
    for (int i = 11; i < argc; i += 2) {
        const std::string key = argv[i];
        if (key == "--collect-items" && !collection_override) {
            profile.scoring_collection = number(argv[i + 1], 1);
            collection_override = true;
        } else if (key == "--aim-boss" && !aim_override) {
            profile.scoring_aim = number(argv[i + 1], 1);
            aim_override = true;
        } else if (key == "--survival-focus" && !form_override) {
            profile.scoring_survival_focus = number(argv[i + 1], 1);
            form_override = true;
        } else if (key == "--verify-horizon" && !horizon_override) {
            horizon = number(argv[i + 1], 256);
            if (horizon != 64 && horizon != 128 && horizon != 256)
                throw std::runtime_error("unsupported_native_style_verification_horizon");
            horizon_override = true;
        } else if (key == "--publish-horizon" && !publication_override) {
            publication = number(argv[i + 1], 128);
            if (publication != 64 && publication != 128)
                throw std::runtime_error("unsupported_native_style_publication_horizon");
            publication_override = true;
        } else if (key == "--held-recovery" && !held_override) {
            held_recovery = number(argv[i + 1], 1);
            held_override = true;
        } else if (key == "--boss-collect" && !boss_collection_override) {
            profile.scoring_boss_collection = number(argv[i + 1], 1);
            boss_collection_override = true;
        } else if (key == "--precollect" && !precollect_override) {
            profile.scoring_precollect = number(argv[i + 1], 1);
            precollect_override = true;
        } else if (key == "--survival-bias" && !survival_bias_override) {
            std::size_t used = 0;
            profile.survival_focus_bias = std::stof(argv[i + 1], &used);
            if (used != std::string(argv[i + 1]).size() ||
                !std::isfinite(profile.survival_focus_bias) || profile.survival_focus_bias < 0 ||
                profile.survival_focus_bias > 10000)
                throw std::runtime_error("invalid_native_survival_bias");
            survival_bias_override = true;
        } else if (key == "--survival-flow" && !survival_flow_override) {
            profile.scoring_survival_flow = number(argv[i + 1], 1);
            survival_flow_override = true;
        } else if (key == "--attack-pacing" && !pacing_override) {
            profile.scoring_attack_pacing = number(argv[i + 1], 1);
            pacing_override = true;
        } else if (key == "--attack-focus-bias" && !attack_bias_override) {
            std::size_t used = 0;
            profile.attack_focus_bias = std::stof(argv[i + 1], &used);
            if (used != std::string(argv[i + 1]).size() ||
                !std::isfinite(profile.attack_focus_bias) || profile.attack_focus_bias < 0 ||
                profile.attack_focus_bias > 10000)
                throw std::runtime_error("invalid_native_attack_focus_bias");
            attack_bias_override = true;
        } else if (key == "--baseline-recovery" && !recovery_override) {
            baseline_recovery = number(argv[i + 1], 1);
            recovery_override = true;
        } else if (key == "--search-budget-ms" && !budget_override) {
            budget_ms = number(argv[i + 1], 1000);
            if (!budget_ms)
                throw std::runtime_error("invalid_native_search_budget");
            budget_override = true;
        } else if (key == "--capture-factor" && !reserve_factor_override) {
            profile.attack_reserve_factor = preference(argv[i + 1]);
            reserve_factor_override = true;
        } else if (key == "--capture-padding" && !reserve_updates_override) {
            profile.attack_reserve_updates = number(argv[i + 1], 10000);
            reserve_updates_override = true;
        } else if (key == "--priced-collection" && !price_override) {
            profile.scoring_priced_collection = number(argv[i + 1], 1);
            price_override = true;
        } else if (key == "--upper-stage" && !upper_override) {
            profile.scoring_upper_stage = number(argv[i + 1], 1);
            upper_override = true;
        } else if (key == "--collection-bias" && !collection_bias_override) {
            profile.collection_focus_bias = preference(argv[i + 1]);
            collection_bias_override = true;
        } else if (key == "--human-bias" && !human_bias_override) {
            profile.economic_human_bias = preference(argv[i + 1]);
            human_bias_override = true;
        } else {
            throw std::runtime_error("unknown_or_duplicate_native_score_control");
        }
    }
    if (publication > horizon)
        throw std::runtime_error("publication_exceeds_native_verification");
    profile.horizon = 24;
    profile.beam = 192;
    profile.diversity_cell = 16;
    profile.spawn_guard = 16;
    profile.reach_filter = profile.hard_phantoms = profile.motion_envelope = true;
    o::Policy policy(profile);
    o::Policy safety(o::with_style("baseline", profile));
    fs::create_directories(output.parent_path());
    if (!fs::create_directory(output))
        throw std::runtime_error("native_style_output_already_exists");
    fs::current_path(output);
    h::score_audit_enable();
    auto &session = *new h::Session(config);
    s::Packet actual;
    actual.kind = s::Kind::State;
    actual.state = s::capture(session);
    s::ActionBuffer queue(actual);
    s::Prediction previous;
    s::NominalExtension nominal;
    std::deque<s::Packet> history;
    o::Observation observation;
    o::capture_policy_memory(session, 0, observation, policy);
    policy.observe(observation);
    if (baseline_recovery)
        safety.observe(observation);
    const auto begin = Clock::now();
    Work work;
    std::vector<std::uint16_t> actions;
    std::uint64_t last_search = 0, fallback = 0, moving = 0, focused = 0, changes = 0;
    std::uint64_t retained_windows = 0, retained_inputs = 0;
    std::uint64_t horizontal_reversals = 0, vertical_reversals = 0;
    int last_horizontal = 0, last_vertical = 0;
    std::uint64_t owner_native_ns = 0, owner_io_ns = 0;
    std::string outcome = "budget";
    for (unsigned update = 0; update < maximum_updates; ++update) {
        if (Clock::now() - begin >= std::chrono::seconds(seconds))
            break;
        if (actual.sequence - last_search >= 8 && queue.size() <= publication - 8) {
            last_search = actual.sequence;
            const SearchOptions options{horizon, publication, budget_ms, held_recovery,
                                        baseline_recovery};
            auto p = search(session, policy, safety, queue, previous, nominal, {config.story, -1},
                            options, work);
            if (p.certifies()) {
                if (!queue.replace(p, {}, publication))
                    throw std::runtime_error("native_style_certificate_rejected");
                previous = std::move(p);
            } else if (horizon > publication && previous.certifies() && queue.size() >= lead &&
                       actual.sequence >= previous.origin.sequence) {
                const auto consumed = actual.sequence - previous.origin.sequence;
                if (consumed <= history.size()) {
                    const std::vector<s::Packet> executed(history.end() - consumed, history.end());
                    const auto pending = queue.size();
                    // Reuse only an earlier complete certificate. The failed
                    // extension contributes no prefix, even if it ran safely.
                    if (queue.extend(previous, executed, publication)) {
                        ++retained_windows;
                        retained_inputs += queue.size() - pending;
                    }
                }
            }
        }
        auto input = s::bridge::fallback(std::uint16_t(actual.input), actual.sequence + 1, false);
        if (queue.next())
            input = std::uint16_t(queue.next()->input);
        else {
            ++fallback;
            if (session.state().dialogue_present)
                input = s::dialogue_input(input, actual.sequence + 1);
        }
        const auto step_begin = Clock::now();
        const auto io_before = session.file_io_time_ns();
        const auto state = session.step(input);
        const auto io = session.file_io_time_ns() - io_before;
        const auto ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - step_begin).count();
        owner_io_ns += io;
        owner_native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
        s::advance_receipt(actual, input, s::capture(session));
        const bool certified = queue.next() != nullptr;
        if (!queue.confirm(actual) && certified)
            throw std::runtime_error("native_style_certificate_execution_mismatch");
        if (horizon > publication) {
            history.push_back(actual);
            if (history.size() > horizon)
                history.pop_front();
        }
        moving += bool(input & 0xf0);
        focused += bool(input & 4);
        changes += !actions.empty() && (input & 0xf4) != (actions.back() & 0xf4);
        const int horizontal = int(bool(input & 128)) - int(bool(input & 64));
        const int vertical = int(bool(input & 32)) - int(bool(input & 16));
        if (horizontal) {
            horizontal_reversals += horizontal * last_horizontal < 0;
            last_horizontal = horizontal;
        }
        if (vertical) {
            vertical_reversals += vertical * last_vertical < 0;
            last_vertical = vertical;
        }
        actions.push_back(input);
        h::score_audit_observe();
        if (state.player_state == 2 || state.deaths != 0 || actual.state[s::Field::BombsUsed]) {
            outcome = "nmnb_failure";
            break;
        }
        if (s::stage_terminal(config.story, state)) {
            outcome = "complete";
            break;
        }
        o::capture_policy_memory(session, actual.sequence, observation, policy);
        policy.observe(observation);
        if (baseline_recovery)
            safety.observe(observation);
    }
    const auto elapsed = std::chrono::duration<double>(Clock::now() - begin).count();
    std::ofstream tape("plan.actions");
    for (auto input : actions)
        tape << input << '\n';
    tape.close();
    std::ofstream audit("score-audit.json");
    const bool captures =
        th08::report::write_score_audit(audit, h::score_audit(), outcome == "complete");
    audit.close();
    if (!tape || !audit)
        throw std::runtime_error("native_style_output_failed");
    if (outcome == "complete") {
        const auto child = fork();
        if (child < 0)
            throw std::runtime_error("native_style_writer_fork_failed");
        if (!child) {
            th08::ReplayManager::SaveReplay("native.rpy", "STYLE   ");
            _exit(fs::exists("native.rpy") ? 0 : 1);
        }
        if (!th08::plan::wait_for_child(child).exited() || !fs::exists("native.rpy"))
            throw std::runtime_error("native_style_writer_failed");
    }
    std::ofstream report("result.json");
    report << "{\"schema_version\":1,\"evidence\":\"synchronous-native-style-experiment\","
              "\"online_fps_evidence\":false,\"style_revision\":1,\"style\":\""
           << style << "\",\"outcome\":\"" << outcome << "\",\"stage\":" << config.stage
           << ",\"score_goal_revision\":" << (policy.needs_scoring_memory() ? 1 : 0)
           << ",\"collect_items\":" << (profile.scoring_collection ? "true" : "false")
           << ",\"aim_boss\":" << (profile.scoring_aim ? "true" : "false")
           << ",\"survival_focus\":" << (profile.scoring_survival_focus ? "true" : "false")
           << ",\"form_goal_revision\":" << (profile.scoring_survival_focus ? 2 : 0)
           << ",\"survival_focus_bias\":" << profile.survival_focus_bias
           << ",\"survival_flow\":" << (profile.scoring_survival_flow ? "true" : "false")
           << ",\"flow_goal_revision\":" << (profile.scoring_survival_flow ? 1 : 0)
           << ",\"attack_pacing\":" << (profile.scoring_attack_pacing ? "true" : "false")
           << ",\"attack_pacing_revision\":" << (profile.scoring_attack_pacing ? 2 : 0)
           << ",\"attack_focus_bias\":" << profile.attack_focus_bias
           << ",\"attack_reserve_factor\":" << profile.attack_reserve_factor
           << ",\"attack_reserve_updates\":" << profile.attack_reserve_updates
           << ",\"boss_collect\":" << (profile.scoring_boss_collection ? "true" : "false")
           << ",\"precollect\":" << (profile.scoring_precollect ? "true" : "false")
           << ",\"priced_collection\":" << (profile.scoring_priced_collection ? "true" : "false")
           << ",\"upper_stage\":" << (profile.scoring_upper_stage ? "true" : "false")
           << ",\"collection_focus_bias\":" << profile.collection_focus_bias
           << ",\"economic_human_bias\":" << profile.economic_human_bias
           << ",\"difficulty\":" << config.difficulty << ",\"shot\":" << config.shot
           << ",\"seed\":" << config.seed << ",\"story\":" << (config.story ? "true" : "false")
           << ",\"prior_final_a_clear\":" << (config.prior_final_a_clear ? "true" : "false")
           << ",\"input_profile\":\"sampled-gameplay\",\"horizon\":" << horizon
           << ",\"publication_horizon\":" << publication
           << ",\"lead\":8,\"search_budget_ms\":" << budget_ms
           << ",\"proposal_budget_ms\":60,\"max_updates\":" << maximum_updates
           << ",\"wall_budget_seconds\":" << seconds << ",\"updates\":" << actions.size()
           << ",\"score_raw\":" << h::score_audit().score_raw
           << ",\"score_displayed\":" << std::uint64_t(h::score_audit().score_raw) * 10
           << ",\"all_visited_stage_spells_captured\":" << (captures ? "true" : "false")
           << ",\"wall_seconds\":" << elapsed << ",\"searches\":" << work.searches
           << ",\"candidates\":" << work.candidates
           << ",\"nominal_adoptions\":" << work.nominal_adoptions
           << ",\"fresh_adoptions\":" << work.fresh_adoptions
           << ",\"held_recovery\":" << (held_recovery ? "true" : "false")
           << ",\"held_attempts\":" << work.held_attempts
           << ",\"held_adoptions\":" << work.held_adoptions
           << ",\"baseline_recovery\":" << (baseline_recovery ? "true" : "false")
           << ",\"safety_attempts\":" << work.safety_attempts
           << ",\"safety_adoptions\":" << work.safety_adoptions
           << ",\"retained_windows\":" << retained_windows
           << ",\"retained_inputs\":" << retained_inputs << ",\"fallback_updates\":" << fallback
           << ",\"future_deaths\":" << work.future_deaths << ",\"timeouts\":" << work.timeouts
           << ",\"budget_overruns\":" << work.overruns
           << ",\"branch_updates\":" << work.branch_updates
           << ",\"proposal_updates\":" << work.proposal_updates
           << ",\"branch_native_ns\":" << work.native_ns
           << ",\"branch_file_io_ns\":" << work.file_io_ns
           << ",\"guidance_ns\":" << work.guidance_ns << ",\"owner_native_ns\":" << owner_native_ns
           << ",\"owner_file_io_ns\":" << owner_io_ns
           << ",\"planning_mean_ms\":" << (work.searches ? work.total_ms / work.searches : 0)
           << ",\"planning_max_ms\":" << work.maximum_ms << ",\"moving_updates\":" << moving
           << ",\"focused_updates\":" << focused << ",\"motion_input_changes\":" << changes
           << ",\"horizontal_reversals\":" << horizontal_reversals
           << ",\"vertical_reversals\":" << vertical_reversals << ",\"terminal_projection\":[";
    for (std::size_t i = 0; i < actual.state.values.size(); ++i)
        report << (i ? "," : "") << actual.state.values[i];
    report << "]}\n";
    report.close();
    if (!report)
        throw std::runtime_error("native_style_report_failed");
    std::cout << outcome << '\n';
    return outcome == "complete" ? 0 : 2;
}
} // namespace
int main(int argc, char **argv) {
    int code = 1;
    try {
        code = run(argc, argv);
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
    }
    std::cout.flush();
    std::cerr.flush();
    _exit(code);
}
