// Synchronous Native experiment for complete-duration style comparisons. It
// retains the H64 proposal/verification budgets and immutable eight-input lead;
// computation pauses this owner, so its throughput is not an online FPS result.
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
constexpr unsigned horizon = 64, lead = 8, budget_ms = 110;
struct Work {
    std::uint64_t searches = 0, candidates = 0, branch_updates = 0, proposal_updates = 0;
    std::uint64_t future_deaths = 0, timeouts = 0, nominal_adoptions = 0, fresh_adoptions = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0, guidance_ns = 0, overruns = 0;
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
s::Prediction search(h::Session &session, o::Policy &policy, const s::ActionBuffer &queue,
                     const s::Prediction &previous, s::NominalExtension &nominal,
                     s::Boundary boundary, Work &work) {
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
    if (nominal.eligible(previous, queue, lead, horizon)) {
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
    for (auto &inputs : candidates) {
        for (unsigned i = lead; i < inputs.size(); ++i)
            inputs[i] = std::uint16_t((inputs[i] & ~0x1000) |
                                      (((origin.sequence + i + 1) & 1) ? 0x1000 : 0));
        if (!remaining())
            break;
        auto p = s::predict_with_policy(session, policy, origin, inputs, horizon, boundary,
                                        std::max(1u, remaining()), true);
        work.branch(p);
        if (p.certifies()) {
            ++work.fresh_adoptions;
            return finish(std::move(p));
        }
    }
    return finish({});
}
unsigned number(const char *text, unsigned maximum) {
    const std::string value(text);
    std::size_t used = 0;
    const auto n = std::stoul(value, &used);
    if (used != value.size() || value.empty() || value.front() == '-' || n > maximum)
        throw std::runtime_error("invalid_native_style_argument");
    return unsigned(n);
}
int run(int argc, char **argv) {
    if (argc < 11 || (argc - 11) % 2 != 0)
        throw std::runtime_error("usage: th08_shadow_style DAT OUTPUT_DIR STAGE DIFFICULTY SHOT "
                                 "SEED STYLE MAX_UPDATES STORY WALL_SECONDS "
                                 "[--collect-items 0|1] [--aim-boss 0|1]");
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
    bool collection_override = false, aim_override = false;
    for (int i = 11; i < argc; i += 2) {
        const std::string key = argv[i];
        if (key == "--collect-items" && !collection_override) {
            profile.scoring_collection = number(argv[i + 1], 1);
            collection_override = true;
        } else if (key == "--aim-boss" && !aim_override) {
            profile.scoring_aim = number(argv[i + 1], 1);
            aim_override = true;
        } else {
            throw std::runtime_error("unknown_or_duplicate_native_score_control");
        }
    }
    profile.horizon = 24;
    profile.beam = 192;
    profile.diversity_cell = 16;
    profile.spawn_guard = 16;
    profile.reach_filter = profile.hard_phantoms = profile.motion_envelope = true;
    o::Policy policy(profile);
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
    o::Observation observation;
    o::capture_policy_memory(session, 0, observation, policy);
    policy.observe(observation);
    const auto begin = Clock::now();
    Work work;
    std::vector<std::uint16_t> actions;
    std::uint64_t last_search = 0, fallback = 0, moving = 0, focused = 0, changes = 0;
    std::uint64_t horizontal_reversals = 0, vertical_reversals = 0;
    int last_horizontal = 0, last_vertical = 0;
    std::uint64_t owner_native_ns = 0, owner_io_ns = 0;
    std::string outcome = "budget";
    for (unsigned update = 0; update < maximum_updates; ++update) {
        if (Clock::now() - begin >= std::chrono::seconds(seconds))
            break;
        if (actual.sequence - last_search >= 8 && queue.size() <= horizon - 8) {
            last_search = actual.sequence;
            auto p = search(session, policy, queue, previous, nominal, {config.story, -1}, work);
            if (p.certifies()) {
                if (!queue.replace(p))
                    throw std::runtime_error("native_style_certificate_rejected");
                previous = std::move(p);
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
           << ",\"difficulty\":" << config.difficulty << ",\"shot\":" << config.shot
           << ",\"seed\":" << config.seed << ",\"story\":" << (config.story ? "true" : "false")
           << ",\"prior_final_a_clear\":" << (config.prior_final_a_clear ? "true" : "false")
           << ",\"input_profile\":\"sampled-gameplay\",\"horizon\":64,\"lead\":8,"
              "\"search_budget_ms\":110,\"proposal_budget_ms\":60,\"max_updates\":"
           << maximum_updates << ",\"wall_budget_seconds\":" << seconds
           << ",\"updates\":" << actions.size() << ",\"score_raw\":" << h::score_audit().score_raw
           << ",\"score_displayed\":" << std::uint64_t(h::score_audit().score_raw) * 10
           << ",\"all_visited_stage_spells_captured\":" << (captures ? "true" : "false")
           << ",\"wall_seconds\":" << elapsed << ",\"searches\":" << work.searches
           << ",\"candidates\":" << work.candidates
           << ",\"nominal_adoptions\":" << work.nominal_adoptions
           << ",\"fresh_adoptions\":" << work.fresh_adoptions
           << ",\"fallback_updates\":" << fallback << ",\"future_deaths\":" << work.future_deaths
           << ",\"timeouts\":" << work.timeouts << ",\"budget_overruns\":" << work.overruns
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
