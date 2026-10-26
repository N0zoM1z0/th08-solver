#include "th08/score_refine.hpp"
#include "ReplayManager.hpp"
#include "modern/headless/session.hpp"
#include "score_audit_report.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/shadow_native.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <unistd.h>

namespace {
namespace s = th08::shadow;
namespace h = th08::headless;
namespace p = th08::plan;
namespace fs = std::filesystem;
unsigned number(const char *value, unsigned maximum) {
    const auto n = p::integer<unsigned>(value, "score refinement option");
    if (n > maximum)
        throw std::runtime_error("score_refinement_option_out_of_range");
    return n;
}
const char *status_name(s::BranchStatus status) {
    switch (status) {
    case s::BranchStatus::Incomplete:
        return "incomplete";
    case s::BranchStatus::Horizon:
        return "tape_exhausted";
    case s::BranchStatus::Terminal:
        return "terminal";
    case s::BranchStatus::Death:
        return "death";
    case s::BranchStatus::Fault:
        return "fault";
    case s::BranchStatus::Timeout:
        return "timeout";
    case s::BranchStatus::CaptureFailure:
        return "capture_failure";
    }
    throw std::runtime_error("unknown_refinement_status");
}
void write_evaluation(std::ostream &out, const s::TapeEvaluation &e) {
    out << "{\"status\":\"" << status_name(e.status)
        << "\",\"score_displayed\":" << e.score_displayed()
        << ",\"all_captures\":" << (e.all_captures ? "true" : "false")
        << ",\"expected_captures\":" << e.expected_captures << ",\"captured\":" << e.captured
        << ",\"requested\":" << e.requested << ",\"attempts\":" << e.attempts
        << ",\"completed\":" << e.completed << ",\"charged_updates\":" << e.charged_updates
        << ",\"native_ns\":" << e.native_ns << ",\"file_io_ns\":" << e.file_io_ns
        << ",\"wall_ms\":" << e.wall_ms << '}';
}
int run(int argc, char **argv) {
    if (argc < 10 || (argc - 10) % 2)
        throw std::runtime_error(
            "usage: th08_score_refine DAT OUTPUT_DIR STAGE DIFFICULTY SHOT SEED STORY "
            "PRIOR_FINAL_A INPUT_TAPE [--family form|fire] [--trials N] [--window UPDATES] "
            "[--interval UPDATES] [--timeout-ms MS] [--wall-seconds SECONDS] "
            "[--target-score DISPLAYED]");
    const auto dat = fs::absolute(argv[1]).string();
    const auto output = fs::absolute(argv[2]);
    const auto input = fs::absolute(argv[9]);
    h::Config config;
    config.dat_path = dat.c_str();
    config.stage = int(number(argv[3], 8));
    config.difficulty = int(number(argv[4], 4));
    config.shot = int(number(argv[5], 11));
    config.seed = std::uint16_t(number(argv[6], 65535));
    config.story = number(argv[7], 1);
    config.prior_final_a_clear = number(argv[8], 1);
    config.input_profile = h::InputProfile::SampledGameplay;
    s::ScoreRefineOptions options;
    std::uint64_t target = 0;
    std::set<std::string> supplied;
    for (int i = 10; i < argc; i += 2) {
        const std::string key = argv[i], value = argv[i + 1];
        if (!supplied.insert(key).second)
            throw std::runtime_error("duplicate_score_refinement_option");
        if (key == "--family") {
            if (value != "form" && value != "fire")
                throw std::runtime_error("unknown_score_refinement_family");
            options.family = value == "form" ? s::ScoreEditFamily::Form : s::ScoreEditFamily::Fire;
        } else if (key == "--trials")
            options.trials = number(argv[i + 1], 10000);
        else if (key == "--window")
            options.window = number(argv[i + 1], 3600);
        else if (key == "--interval")
            options.interval = number(argv[i + 1], 400000);
        else if (key == "--timeout-ms")
            options.timeout_ms = number(argv[i + 1], 60000);
        else if (key == "--wall-seconds")
            options.wall_seconds = number(argv[i + 1], 7200);
        else if (key == "--target-score") {
            target = p::integer<std::uint64_t>(value, key);
            if (target > 9999999990ULL)
                throw std::runtime_error("score_refinement_target_out_of_range");
        } else
            throw std::runtime_error("unknown_score_refinement_option");
    }
    if (!options.window || !options.interval || !options.timeout_ms || !options.wall_seconds)
        throw std::runtime_error("empty_score_refinement_budget");
    std::ifstream tape(input);
    p::PrefixParser parser(400000);
    std::array<char, 65536> buffer;
    while (tape) {
        tape.read(buffer.data(), buffer.size());
        parser.append(buffer.data(), std::size_t(tape.gcount()));
    }
    if (!tape.eof())
        throw std::runtime_error("score_refinement_tape_read_failed");
    auto actions = parser.finish();
    const auto initial_inputs = actions.size();
    fs::create_directories(output.parent_path());
    if (!fs::create_directory(output))
        throw std::runtime_error("score_refinement_output_already_exists");
    fs::current_path(output);
    h::score_audit_enable();
    auto &session = *new h::Session(config);
    const auto result = s::refine_score_tape(session, actions, config.story, options);
    if (!result.complete) {
        std::ofstream report("result.json");
        report << "{\"schema_version\":1,\"evidence\":\"native-complete-tape-refinement\","
                  "\"outcome\":\"invalid_incumbent\",\"owner_updates\":0,\"wall_seconds\":"
               << result.wall_seconds << ",\"initial\":";
        write_evaluation(report, result.initial);
        report << "}\n";
        report.close();
        if (!report)
            throw std::runtime_error("score_refinement_rejection_report_failed");
        std::cerr << "refinement_requires_complete_nmnb_all_capture_incumbent\n";
        return 2;
    }
    std::ofstream final_tape("plan.actions"), audit("score-audit.json");
    for (auto action : actions)
        final_tape << action << '\n';
    const bool all = th08::report::write_score_audit(audit, h::score_audit(), true);
    final_tape.close();
    audit.close();
    if (!all || !final_tape || !audit)
        throw std::runtime_error("score_refinement_output_failed");
    const auto child = fork();
    if (child < 0)
        throw std::runtime_error("score_refinement_writer_fork_failed");
    if (!child) {
        th08::ReplayManager::SaveReplay("native.rpy", "ysopwnab");
        _exit(fs::exists("native.rpy") ? 0 : 1);
    }
    if (!p::wait_for_child(child).exited() || !fs::exists("native.rpy"))
        throw std::runtime_error("score_refinement_writer_failed");
    const bool objective_met = result.selected.score_displayed() > target;
    unsigned accepted = 0, deaths = 0, timeouts = 0, faults = 0;
    auto charged = result.initial.charged_updates, native_ns = result.initial.native_ns,
         io_ns = result.initial.file_io_ns;
    for (const auto &trial : result.trials) {
        accepted += trial.accepted;
        charged += trial.evaluation.charged_updates;
        native_ns += trial.evaluation.native_ns;
        io_ns += trial.evaluation.file_io_ns;
        deaths += trial.evaluation.status == s::BranchStatus::Death;
        timeouts += trial.evaluation.status == s::BranchStatus::Timeout;
        faults += trial.evaluation.status == s::BranchStatus::Fault;
    }
    std::ofstream report("result.json");
    report << "{\"schema_version\":1,\"evidence\":\"native-complete-tape-refinement\","
              "\"outcome\":\"complete\",\"online_fps_evidence\":false,\"replay_name\":\"ysopwnab\","
              "\"input_profile\":\"sampled-gameplay\",\"stage\":"
           << config.stage << ",\"difficulty\":" << config.difficulty << ",\"shot\":" << config.shot
           << ",\"seed\":" << config.seed << ",\"story\":" << (config.story ? "true" : "false")
           << ",\"prior_final_a_clear\":" << (config.prior_final_a_clear ? "true" : "false")
           << ",\"family\":\"" << (options.family == s::ScoreEditFamily::Form ? "form" : "fire")
           << "\",\"trial_limit\":" << options.trials << ",\"window\":" << options.window
           << ",\"interval\":" << options.interval << ",\"timeout_ms\":" << options.timeout_ms
           << ",\"wall_budget_seconds\":" << options.wall_seconds
           << ",\"initial_inputs\":" << initial_inputs << ",\"updates\":" << result.owner_updates
           << ",\"score_raw\":" << session.state().score
           << ",\"score_displayed\":" << result.selected.score_displayed()
           << ",\"target_score_displayed\":" << target
           << ",\"objective_met\":" << (objective_met ? "true" : "false")
           << ",\"all_visited_stage_spells_captured\":true,\"accepted\":" << accepted
           << ",\"unchanged\":" << result.unchanged << ",\"future_deaths\":" << deaths
           << ",\"timeouts\":" << timeouts << ",\"faults\":" << faults
           << ",\"branch_updates\":" << charged << ",\"branch_native_ns\":" << native_ns
           << ",\"branch_file_io_ns\":" << io_ns
           << ",\"owner_native_ns\":" << result.owner_native_ns
           << ",\"owner_file_io_ns\":" << result.owner_file_io_ns
           << ",\"wall_seconds\":" << result.wall_seconds
           << ",\"search_budget_exhausted\":" << (result.search_budget_exhausted ? "true" : "false")
           << ",\"initial\":";
    write_evaluation(report, result.initial);
    report << ",\"trials\":[";
    for (std::size_t i = 0; i < result.trials.size(); ++i) {
        const auto &trial = result.trials[i];
        report << (i ? "," : "") << "{\"first\":" << trial.first << ",\"window\":" << trial.window
               << ",\"edit\":\"" << s::score_edit_name(trial.edit)
               << "\",\"accepted\":" << (trial.accepted ? "true" : "false") << ",\"evaluation\":";
        write_evaluation(report, trial.evaluation);
        report << '}';
    }
    report << "],\"terminal_projection\":[";
    const auto terminal = s::capture(session);
    for (std::size_t i = 0; i < terminal.values.size(); ++i)
        report << (i ? "," : "") << terminal.values[i];
    report << "]}\n";
    report.close();
    if (!report)
        throw std::runtime_error("score_refinement_report_failed");
    std::cout << "complete " << result.selected.score_displayed() << " accepted=" << accepted << '/'
              << result.trials.size() << '\n';
    return target && !objective_met ? 5 : 0;
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
