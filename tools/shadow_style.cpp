// Synchronous Native experiment for complete-duration style comparisons. It
// records explicit search budgets, lead and verification/publication horizons.
// Computation pauses this owner, so its throughput is not an online FPS result.
#include "ReplayManager.hpp"
#include "modern/headless/session.hpp"
#include "score_audit_report.hpp"
#include "th08/online_native.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_prefix.hpp"
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
// Parent-owner output only: branch proposals never append to this stream.
// Fork children inherit its descriptor but do not access this object. The tape
// is append-only, while the small metadata file is replaced after each flush.
// A resource kill may leave extra trailing inputs beyond the committed byte
// count. Neither file certifies safety, completion or a capture outcome: a
// resumed candidate must reexecute its declared prefix from the original entry.
class ExecutedInputCheckpoint {
    unsigned interval_;
    std::ofstream tape_;
    std::uint64_t updates_ = 0, saved_updates_ = 0, saved_bytes_ = 0, io_ns_ = 0;

    void flush() {
        tape_.flush();
        const auto bytes = tape_.tellp();
        if (!tape_ || bytes == std::streampos(-1))
            throw std::runtime_error("native_style_checkpoint_tape_failed");
        std::ofstream metadata("checkpoint.json.tmp");
        metadata << "{\"schema_version\":1,\"evidence\":\"executed-input-prefix-only\","
                    "\"gameplay_certificate\":false,\"saved_updates\":"
                 << updates_ << ",\"saved_bytes\":" << std::uint64_t(bytes)
                 << ",\"interval_updates\":" << interval_
                 << ",\"tape\":\"checkpoint.inputs.actions\","
                    "\"resume\":\"reexecute-from-original-entry\"}\n";
        metadata.close();
        if (!metadata)
            throw std::runtime_error("native_style_checkpoint_metadata_failed");
        // The rename commits a count only after its entire input prefix has
        // reached the file. This is process-termination recovery, without an
        // fsync-based machine-crash durability claim.
        fs::rename("checkpoint.json.tmp", "checkpoint.json");
        saved_updates_ = updates_;
        saved_bytes_ = std::uint64_t(bytes);
    }

  public:
    explicit ExecutedInputCheckpoint(unsigned interval) : interval_(interval) {
        if (interval_) {
            const auto begin = Clock::now();
            tape_.open("checkpoint.inputs.actions", std::ios::binary | std::ios::trunc);
            if (!tape_)
                throw std::runtime_error("native_style_checkpoint_open_failed");
            io_ns_ +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
        }
    }
    void append(std::uint16_t input) {
        if (!interval_)
            return;
        const auto begin = Clock::now();
        tape_ << input << '\n';
        ++updates_;
        if (updates_ % interval_ == 0)
            flush();
        io_ns_ +=
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
    }
    void finish() {
        if (interval_) {
            const auto begin = Clock::now();
            if (updates_ != saved_updates_)
                flush();
            tape_.close();
            if (!tape_)
                throw std::runtime_error("native_style_checkpoint_close_failed");
            io_ns_ +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count();
        }
    }
    std::uint64_t saved_updates() const {
        return saved_updates_;
    }
    std::uint64_t saved_bytes() const {
        return saved_bytes_;
    }
    std::uint64_t io_ns() const {
        return io_ns_;
    }
};
// Durations are owned Native inputs, including control-only stage handoffs.
// This is a search/publication schedule; it never owns or resets gameplay state.
struct SearchProfile {
    unsigned horizon, publication, replan;
};
struct SearchOptions {
    unsigned horizon = 64, publication = 64, budget_ms = 110;
    bool held_recovery = false, baseline_recovery = false;
    bool capture_recovery = false;
    unsigned lead = 8, nominal_warm_updates = 0;
    unsigned economic_proposals = 0;
    bool allow_bombs = false;
    unsigned economic_movement = 0;
};
struct Work {
    std::uint64_t searches = 0, candidates = 0, branch_updates = 0, proposal_updates = 0;
    std::uint64_t future_deaths = 0, timeouts = 0, nominal_adoptions = 0, fresh_adoptions = 0;
    std::uint64_t native_ns = 0, file_io_ns = 0, guidance_ns = 0, overruns = 0;
    std::uint64_t held_attempts = 0, held_adoptions = 0;
    std::uint64_t safety_attempts = 0, safety_adoptions = 0;
    std::uint64_t capture_failures = 0;
    std::uint64_t economic_attempts = 0, economic_adoptions = 0;
    std::uint64_t motion_economic_attempts = 0, motion_economic_adoptions = 0;
    double total_ms = 0, maximum_ms = 0;
    void branch(const s::Prediction &p) {
        ++candidates;
        branch_updates += p.charged_updates;
        native_ns += p.native_ns;
        file_io_ns += p.file_io_ns;
        guidance_ns += p.guidance_ns;
        future_deaths += p.status == s::BranchStatus::Death;
        capture_failures += p.status == s::BranchStatus::CaptureFailure;
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
               budget_ms = options.budget_ms, lead = options.lead;
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
        // Keep the certified incumbent while comparing up to four fire/form
        // alternatives. Optional movement alternatives follow in the same window.
        // Cash at this horizon is a proposal ranking, never a terminal-score proof.
        const auto compare_start = Clock::now();
        if (options.economic_proposals && result.certifies() && result.frames.size() > lead + 32 &&
            !session.state().dialogue_present) {
            const auto seed_inputs = result.frames;
            unsigned tried = 0;
            for (const unsigned mode : {1u, 5u, 0u, 4u}) {
                if (!remaining() ||
                    std::chrono::duration<double, std::milli>(Clock::now() - compare_start)
                            .count() >= 2000)
                    break;
                std::vector<std::uint16_t> proposal;
                bool differs = false;
                for (unsigned i = 0; i < lead + 32; ++i) {
                    auto input = std::uint16_t(seed_inputs[i].input);
                    if (i >= lead) {
                        const auto changed = std::uint16_t((input & ~5u) | mode);
                        differs |= changed != input;
                        input = changed;
                    }
                    proposal.push_back(input);
                }
                if (!differs)
                    continue;
                if (tried++ >= options.economic_proposals)
                    break;
                const auto timeout_ms = remaining();
                if (!timeout_ms)
                    break;
                ++work.economic_attempts;
                auto alternative =
                    s::predict_with_policy(session, policy, queue.frontier(), proposal, horizon,
                                           boundary, timeout_ms, true);
                work.branch(alternative);
                if (!alternative.certifies())
                    continue;
                const auto &a = alternative.frames.back().state;
                const auto &b = result.frames.back().state;
                // Stable ties retain the incumbent. Never compare partial futures.
                if (a[s::Field::Score] > b[s::Field::Score]) {
                    ++work.economic_adoptions;
                    result = std::move(alternative);
                }
            }
        }
        // Opt-in active-spell movement comparison. Each pulse preserves the
        // incumbent's fire/form bits and immutable lead, then lets ordinary
        // guidance continue in its own action-dependent Native future. A
        // higher horizon cash value is only a proposal ranking, not a promise
        // of capture or higher final score. Failed/partial futures never compete.
        // Fire/form comparisons run first; both kinds share the same 2000ms
        // admission window and total search budget. An admitted future may use
        // the remaining total budget, as before. Zero preserves the old path.
        constexpr unsigned motion_updates = 16;
        if (options.economic_movement && result.certifies() &&
            result.frames.size() > lead + motion_updates && session.state().spell_active &&
            !session.state().dialogue_present) {
            const auto seed_inputs = result.frames;
            unsigned tried = 0;
            // Stable order: up, down, left, right. Equal cash retains the
            // incumbent, so enabling a smaller count is a declared search change.
            for (const unsigned direction : {16u, 32u, 64u, 128u}) {
                if (!remaining() ||
                    std::chrono::duration<double, std::milli>(Clock::now() - compare_start)
                            .count() >= 2000)
                    break;
                std::vector<std::uint16_t> proposal;
                bool differs = false;
                for (unsigned i = 0; i < lead + motion_updates; ++i) {
                    auto input = std::uint16_t(seed_inputs[i].input);
                    if (i >= lead) {
                        const auto changed = std::uint16_t((input & ~0xf0u) | direction);
                        differs |= changed != input;
                        input = changed;
                    }
                    proposal.push_back(input);
                }
                if (!differs)
                    continue;
                if (tried++ >= options.economic_movement)
                    break;
                const auto timeout_ms = remaining();
                if (!timeout_ms)
                    break;
                ++work.motion_economic_attempts;
                auto alternative =
                    s::predict_with_policy(session, policy, queue.frontier(), proposal, horizon,
                                           boundary, timeout_ms, true);
                work.branch(alternative);
                if (alternative.certifies() && alternative.frames.back().state[s::Field::Score] >
                                                   result.frames.back().state[s::Field::Score]) {
                    ++work.motion_economic_adoptions;
                    result = std::move(alternative);
                }
            }
        }
        const auto ms = elapsed();
        work.total_ms += ms;
        work.maximum_ms = std::max(work.maximum_ms, ms);
        work.overruns += ms > budget_ms;
        return result;
    };
    const auto &origin = queue.frontier();
    if (nominal.eligible(previous, queue, lead, publication)) {
        std::vector<std::uint16_t> warm;
        for (auto i = origin.sequence - previous.origin.sequence;
             i < previous.frames.size() &&
             (!options.nominal_warm_updates || warm.size() < options.nominal_warm_updates);
             ++i)
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
        auto repair =
            s::predict_with_policy(session, safety, origin, prefix, horizon, boundary, remaining(),
                                   true, options.capture_recovery ? nullptr : &policy);
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
    work.capture_failures += proposals.status == s::BranchStatus::CaptureFailure;
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
    if (options.allow_bombs && !selected.certifies()) {
        // X is a one-update proposal after the immutable lead. The original
        // runtime decides inventory, lockouts, capture loss and invulnerability.
        // It never turns an incomplete ordinary future into a certificate.
        for (unsigned focus : {0u, 4u}) {
            if (!remaining())
                break;
            auto inputs = prefix;
            inputs.push_back(std::uint16_t(3u | focus));
            auto p = s::predict_with_policy(session, policy, origin, inputs, horizon, boundary,
                                            remaining(), true);
            work.branch(p);
            if (p.certifies()) {
                ++work.fresh_adoptions;
                selected = std::move(p);
                break;
            }
        }
    }
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
            "[--survival-focus 0|1] [--verify-horizon 64|128|256|512|1024|2048] "
            "[--publish-horizon 64|128|256|512|1024|2048] [--held-recovery 0|1] "
            "[--boss-collect 0|1] [--precollect 0|1] [--survival-bias VALUE] "
            "[--survival-flow 0|1] [--attack-pacing 0|1] [--farm-to-timeout 0|1] "
            "[--upper-flow 0|1] [--spell-horizon 0|64|128|256|512|1024|2048] "
            "[--farm-charge-fire 0|1] [--phase-plan none|ROWS] [--stop-after-card 0..256] "
            "[--phase-recovery-prior 0|1] "
            "[--attack-focus-bias VALUE] [--baseline-recovery 0|1] "
            "[--search-budget-ms VALUE] [--lead-updates 1..32] "
            "[--nominal-warm-updates 0|UPDATES] [--replan-updates UPDATES] "
            "[--policy-horizon 2..200] [--policy-beam 1..8192] [--verify-captures 0|1] "
            "[--priced-collection 0|1] "
            "[--upper-stage 0|1] [--collection-bias VALUE] [--human-bias VALUE] "
            "[--capture-factor VALUE] [--capture-padding UPDATES] [--hit-income 0|1] "
            "[--drop-target 0|1] [--drop-follow 0|1] [--parent-chain-bias VALUE] "
            "[--economic-proposals 0..4] [--economic-movement 0..4] "
            "[--recovery-style baseline|expert|zen] [--recovery-fire preserve|capture] "
            "[--replay-name NAME] [--allow-bombs 0|1] [--require-all-captures 0|1] "
            "[--target-score DISPLAYED] "
            "[--checkpoint-updates 0|64..400000] "
            "[--prefix-actions FILE --prefix-updates UPDATES]");
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
    unsigned spell_horizon = 0;
    bool spell_horizon_override = false;
    bool held_recovery = false;
    bool collection_override = false, aim_override = false, horizon_override = false;
    bool form_override = false;
    bool publication_override = false, held_override = false;
    bool boss_collection_override = false, precollect_override = false;
    bool survival_bias_override = false;
    bool survival_flow_override = false;
    bool upper_flow_override = false;
    bool pacing_override = false, attack_bias_override = false, recovery_override = false;
    bool timeout_farm_override = false;
    bool charge_fire_override = false;
    bool budget_override = false, baseline_recovery = false;
    bool price_override = false, upper_override = false, collection_bias_override = false;
    bool human_bias_override = false;
    bool reserve_factor_override = false, reserve_updates_override = false;
    bool hit_income_override = false, drop_target_override = false;
    bool drop_follow_override = false;
    bool parent_chain_override = false;
    bool recovery_style_override = false, recovery_fire_override = false;
    std::string recovery_style = "baseline", recovery_fire = "preserve";
    std::string replay_name = "ysopwnab";
    bool name_override = false, captures_override = false, target_override = false;
    bool require_captures = false;
    bool allow_bombs = false, bombs_override = false;
    std::uint64_t target_score = 0;
    fs::path prefix_path;
    unsigned prefix_updates = 0;
    bool prefix_count_override = false;
    unsigned checkpoint_updates = 0;
    bool checkpoint_override = false;
    unsigned budget_ms = 110;
    unsigned lead_updates = 8, nominal_warm_updates = 0;
    unsigned replan_updates = 8;
    bool lead_override = false, warm_override = false, interval_override = false;
    unsigned policy_horizon = 24, policy_beam = 192;
    bool policy_horizon_override = false, policy_beam_override = false;
    bool verify_captures = false, verify_captures_override = false;
    std::string phase_text;
    bool phase_override = false, stop_override = false;
    bool phase_recovery_prior = false, phase_recovery_override = false;
    unsigned stop_card = 0;
    unsigned economic_proposals = 0;
    bool economic_override = false;
    unsigned economic_movement = 0;
    bool movement_override = false;
    for (int i = 11; i < argc; i += 2) {
        const std::string key = argv[i];
        if (key == "--phase-plan" && !phase_override) {
            phase_text = argv[i + 1];
            profile.phase_plan =
                std::make_shared<const th08::phase::Plan>(th08::phase::parse(phase_text));
            phase_override = true;
        } else if (key == "--phase-recovery-prior" && !phase_recovery_override) {
            phase_recovery_prior = number(argv[i + 1], 1);
            phase_recovery_override = true;
        } else if (key == "--stop-after-card" && !stop_override) {
            stop_card = number(argv[i + 1], 256);
            stop_override = true;
        } else if (key == "--collect-items" && !collection_override) {
            profile.scoring_collection = number(argv[i + 1], 1);
            collection_override = true;
        } else if (key == "--aim-boss" && !aim_override) {
            profile.scoring_aim = number(argv[i + 1], 1);
            aim_override = true;
        } else if (key == "--survival-focus" && !form_override) {
            profile.scoring_survival_focus = number(argv[i + 1], 1);
            form_override = true;
        } else if (key == "--verify-horizon" && !horizon_override) {
            horizon = number(argv[i + 1], 2048);
            if (horizon < 64 || (horizon & (horizon - 1)))
                throw std::runtime_error("unsupported_native_style_verification_horizon");
            horizon_override = true;
        } else if (key == "--publish-horizon" && !publication_override) {
            publication = number(argv[i + 1], 2048);
            if (publication < 64 || (publication & (publication - 1)))
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
        } else if (key == "--farm-to-timeout" && !timeout_farm_override) {
            profile.scoring_farm_to_timeout = number(argv[i + 1], 1);
            timeout_farm_override = true;
        } else if (key == "--upper-flow" && !upper_flow_override) {
            profile.scoring_upper_flow = number(argv[i + 1], 1);
            upper_flow_override = true;
        } else if (key == "--farm-charge-fire" && !charge_fire_override) {
            profile.scoring_farm_charge_fire = number(argv[i + 1], 1);
            charge_fire_override = true;
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
            budget_ms = number(argv[i + 1], 60000);
            if (!budget_ms)
                throw std::runtime_error("invalid_native_search_budget");
            budget_override = true;
        } else if (key == "--lead-updates" && !lead_override) {
            lead_updates = number(argv[i + 1], 32);
            if (!lead_updates)
                throw std::runtime_error("empty_native_style_lead");
            lead_override = true;
        } else if (key == "--nominal-warm-updates" && !warm_override) {
            nominal_warm_updates = number(argv[i + 1], 2048);
            warm_override = true;
        } else if (key == "--checkpoint-updates" && !checkpoint_override) {
            checkpoint_updates = number(argv[i + 1], 400000);
            if (checkpoint_updates && checkpoint_updates < 64)
                throw std::runtime_error("invalid_native_checkpoint_interval");
            checkpoint_override = true;
        } else if (key == "--replan-updates" && !interval_override) {
            replan_updates = number(argv[i + 1], 2048);
            if (!replan_updates)
                throw std::runtime_error("empty_native_replan_interval");
            interval_override = true;
        } else if (key == "--spell-horizon" && !spell_horizon_override) {
            spell_horizon = number(argv[i + 1], 2048);
            if (spell_horizon && (spell_horizon < 64 || (spell_horizon & (spell_horizon - 1))))
                throw std::runtime_error("unsupported_native_spell_verification_horizon");
            spell_horizon_override = true;
        } else if (key == "--policy-horizon" && !policy_horizon_override) {
            policy_horizon = number(argv[i + 1], 200);
            if (policy_horizon < 2)
                throw std::runtime_error("empty_native_policy_horizon");
            policy_horizon_override = true;
        } else if (key == "--policy-beam" && !policy_beam_override) {
            policy_beam = number(argv[i + 1], 8192);
            if (!policy_beam)
                throw std::runtime_error("empty_native_policy_beam");
            policy_beam_override = true;
        } else if (key == "--verify-captures" && !verify_captures_override) {
            verify_captures = number(argv[i + 1], 1);
            verify_captures_override = true;
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
        } else if (key == "--hit-income" && !hit_income_override) {
            profile.scoring_hit_income = number(argv[i + 1], 1);
            hit_income_override = true;
        } else if (key == "--economic-proposals" && !economic_override) {
            economic_proposals = number(argv[i + 1], 4);
            economic_override = true;
        } else if (key == "--economic-movement" && !movement_override) {
            economic_movement = number(argv[i + 1], 4);
            movement_override = true;
        } else if (key == "--drop-target" && !drop_target_override) {
            profile.scoring_drop_target = number(argv[i + 1], 1);
            drop_target_override = true;
        } else if (key == "--drop-follow" && !drop_follow_override) {
            profile.scoring_drop_follow = number(argv[i + 1], 1);
            drop_follow_override = true;
        } else if (key == "--parent-chain-bias" && !parent_chain_override) {
            std::size_t used = 0;
            profile.parent_chain_focus_bias = std::stof(argv[i + 1], &used);
            if (used != std::string(argv[i + 1]).size() ||
                !std::isfinite(profile.parent_chain_focus_bias) ||
                profile.parent_chain_focus_bias < 0 || profile.parent_chain_focus_bias > 10000)
                throw std::runtime_error("invalid_native_parent_chain_bias");
            parent_chain_override = true;
        } else if (key == "--recovery-style" && !recovery_style_override) {
            recovery_style = argv[i + 1];
            if (recovery_style != "baseline" && recovery_style != "expert" &&
                recovery_style != "zen")
                throw std::runtime_error("invalid_native_recovery_style");
            recovery_style_override = true;
        } else if (key == "--recovery-fire" && !recovery_fire_override) {
            recovery_fire = argv[i + 1];
            if (recovery_fire != "preserve" && recovery_fire != "capture")
                throw std::runtime_error("invalid_native_recovery_fire");
            recovery_fire_override = true;
        } else if (key == "--replay-name" && !name_override) {
            replay_name = argv[i + 1];
            if (replay_name.empty() || replay_name.size() > 8 ||
                replay_name.find_first_not_of(
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-") !=
                    std::string::npos)
                throw std::runtime_error(
                    "native_replay_name_requires_1_to_8_ascii_identifier_bytes");
            name_override = true;
        } else if (key == "--allow-bombs" && !bombs_override) {
            allow_bombs = number(argv[i + 1], 1);
            bombs_override = true;
        } else if (key == "--require-all-captures" && !captures_override) {
            require_captures = number(argv[i + 1], 1);
            captures_override = true;
        } else if (key == "--target-score" && !target_override) {
            const std::string text = argv[i + 1];
            std::size_t used = 0;
            target_score = std::stoull(text, &used);
            if (used != text.size() || target_score > 9999999990ULL)
                throw std::runtime_error("invalid_native_displayed_score_target");
            target_override = true;
        } else if (key == "--prefix-actions" && prefix_path.empty()) {
            prefix_path = fs::absolute(argv[i + 1]);
        } else if (key == "--prefix-updates" && !prefix_count_override) {
            prefix_updates = number(argv[i + 1], maximum_updates);
            prefix_count_override = true;
        } else {
            throw std::runtime_error("unknown_or_duplicate_native_score_control");
        }
    }
    if (publication > horizon)
        throw std::runtime_error("publication_exceeds_native_verification");
    if (lead_updates >= publication || replan_updates > publication ||
        (nominal_warm_updates &&
         (nominal_warm_updates < lead_updates || nominal_warm_updates > horizon)))
        throw std::runtime_error("native_warm_prefix_must_cover_lead_within_horizon");
    if (!baseline_recovery && (recovery_style_override || recovery_fire_override))
        throw std::runtime_error("recovery_controls_require_enabled_recovery");
    if (phase_recovery_prior &&
        (!baseline_recovery || !profile.phase_plan || profile.phase_plan->entries().empty()))
        throw std::runtime_error("phase_recovery_prior_requires_phase_plan_and_recovery");
    if (policy_horizon + lead_updates - 1 > horizon || (verify_captures && !require_captures))
        throw std::runtime_error("native_policy_or_capture_goal_exceeds_declared_scope");
    if (profile.scoring_farm_to_timeout &&
        (!profile.scoring_attack_pacing || require_captures || verify_captures))
        throw std::runtime_error("timeout_farming_requires_optional_captures_and_pacing");
    const SearchProfile road_profile{horizon, publication, replan_updates};
    // Zero leaves the original schedule and queue lifetime exactly intact.
    // An enabled spell profile owns its H/2 publication and H/4 retry cadence;
    // explicit road controls do not silently become spell controls.
    const SearchProfile spell_profile{spell_horizon ? spell_horizon : horizon,
                                      spell_horizon ? spell_horizon / 2 : publication,
                                      spell_horizon ? spell_horizon / 4 : replan_updates};
    if (spell_horizon && (lead_updates >= spell_profile.publication ||
                          policy_horizon + lead_updates - 1 > spell_profile.horizon ||
                          (nominal_warm_updates && nominal_warm_updates > spell_profile.horizon)))
        throw std::runtime_error("native_spell_profile_exceeds_declared_scope");
    const bool has_prefix_file = !prefix_path.empty();
    if (has_prefix_file != prefix_count_override)
        throw std::runtime_error("native_prefix_requires_actions_and_update_count");
    std::vector<std::uint16_t> prefix;
    if (!prefix_path.empty()) {
        std::ifstream stream(prefix_path);
        th08::plan::PrefixParser parser(400000, allow_bombs);
        std::array<char, 65536> buffer;
        while (stream) {
            stream.read(buffer.data(), buffer.size());
            parser.append(buffer.data(), std::size_t(stream.gcount()));
        }
        if (!stream.eof())
            throw std::runtime_error("native_prefix_read_failed");
        prefix = parser.finish();
        if (prefix.size() < prefix_updates)
            throw std::runtime_error("native_prefix_shorter_than_requested");
    }
    profile.horizon = int(policy_horizon);
    profile.beam = int(policy_beam);
    profile.diversity_cell = 16;
    profile.spawn_guard = 16;
    profile.reach_filter = profile.hard_phantoms = profile.motion_envelope = true;
    o::Policy policy(profile);
    auto recovery_profile = o::with_style(recovery_style, profile);
    // Recovery keeps its own movement/fire controller and never inherits the
    // full farm schedule. The optional capture prior reproduces the legacy
    // focused escape using core identity, without adding scoring memory or
    // changing recovery's pacing. Native still checks every escape candidate.
    recovery_profile.phase_plan.reset();
    recovery_profile.phase_capture_prior = phase_recovery_prior ? profile.phase_plan : nullptr;
    o::Policy safety(recovery_profile);
    fs::create_directories(output.parent_path());
    if (!fs::create_directory(output))
        throw std::runtime_error("native_style_output_already_exists");
    fs::current_path(output);
    h::score_audit_enable();
    auto &session = *new h::Session(config);
    s::Packet actual;
    actual.kind = s::Kind::State;
    actual.state = s::capture(session);
    s::ActionBuffer queue(actual, allow_bombs);
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
    ExecutedInputCheckpoint checkpoint(checkpoint_updates);
    std::uint64_t last_search = 0, fallback = 0, moving = 0, focused = 0, changes = 0;
    std::uint64_t retained_windows = 0, retained_inputs = 0;
    std::uint64_t horizontal_reversals = 0, vertical_reversals = 0;
    int last_horizontal = 0, last_vertical = 0;
    std::uint64_t owner_native_ns = 0, owner_io_ns = 0;
    std::string outcome = "budget";
    struct PhaseWork {
        std::uint64_t actual_inputs = 0, planned_inputs = 0, fire_inputs = 0, focus_inputs = 0;
        int entry_gauge = 0, exit_gauge = 0;
    };
    std::vector<PhaseWork> phase_work(profile.phase_plan ? profile.phase_plan->entries().size()
                                                         : 0);
    bool profile_initialized = false, previous_spell_origin = false;
    std::uint64_t profile_switches = 0, discarded_pending_inputs = 0;
    std::uint64_t spell_owner_inputs = 0, road_owner_inputs = 0;
    for (unsigned update = 0; update < maximum_updates; ++update) {
        if (Clock::now() - begin >= std::chrono::seconds(seconds))
            break;
        const bool spell_origin = session.state().spell_active;
        const auto &selected = spell_horizon && spell_origin ? spell_profile : road_profile;
        const unsigned effective_horizon = selected.horizon;
        const unsigned effective_publication = selected.publication;
        const unsigned effective_replan = selected.replan;
        bool force_profile_search = false;
        if (update >= prefix_updates) {
            if (spell_horizon && (!profile_initialized || previous_spell_origin != spell_origin)) {
                if (profile_initialized) {
                    ++profile_switches;
                    discarded_pending_inputs += queue.size();
                    // Offline future packets have not been executed. Restart
                    // planning at the exact current receipt when its search
                    // profile changes. Preserve Native actors, RNG, bullets,
                    // lifecycle and policy observer memory without modification.
                    queue = s::ActionBuffer(actual, allow_bombs);
                    previous = {};
                    nominal = {};
                    history.clear();
                }
                profile_initialized = true;
                previous_spell_origin = spell_origin;
                force_profile_search = true;
            }
        }
        // Offline execution may consume a larger verified block. With no
        // certificate available, retain the original at-most-eight-update
        // retry spacing rather than blindly holding through the long interval.
        const auto interval = queue.next() ? effective_replan : std::min(8u, effective_replan);
        if (update >= prefix_updates &&
            (force_profile_search || (actual.sequence - last_search >= interval &&
                                      queue.size() <= effective_publication - effective_replan))) {
            last_search = actual.sequence;
            const SearchOptions options{
                effective_horizon, effective_publication, budget_ms,
                held_recovery,     baseline_recovery,     recovery_fire == "capture",
                lead_updates,      nominal_warm_updates,  economic_proposals,
                allow_bombs,       economic_movement};
            auto p = search(session, policy, safety, queue, previous, nominal,
                            {config.story, -1, verify_captures, allow_bombs}, options, work);
            if (p.certifies()) {
                if (!queue.replace(p, {}, effective_publication))
                    throw std::runtime_error("native_style_certificate_rejected");
                previous = std::move(p);
            } else if (effective_horizon > effective_publication && previous.certifies() &&
                       queue.size() >= lead_updates &&
                       actual.sequence >= previous.origin.sequence) {
                const auto consumed = actual.sequence - previous.origin.sequence;
                if (consumed <= history.size()) {
                    const std::vector<s::Packet> executed(history.end() - consumed, history.end());
                    const auto pending = queue.size();
                    // Reuse only an earlier complete certificate. The failed
                    // extension contributes no prefix, even if it ran safely.
                    if (queue.extend(previous, executed, effective_publication)) {
                        ++retained_windows;
                        retained_inputs += queue.size() - pending;
                    }
                }
            }
        }
        auto input = s::bridge::fallback(std::uint16_t(actual.input), actual.sequence + 1, false);
        // Checkpoint inputs execute literally and warm both observers below.
        // The same owned world/RNG/actors continue into fresh planning.
        if (update < prefix_updates)
            input = prefix[update];
        else if (queue.next())
            input = std::uint16_t(queue.next()->input);
        else {
            ++fallback;
            if (session.state().dialogue_present)
                input = s::dialogue_input(input, actual.sequence + 1);
        }
        // Count only actually executed owner inputs. Prefix inputs are retained
        // observations, not decisions made by this new strategy schedule.
        int phase_index = -1;
        if (profile.phase_plan && session.state().spell_active) {
            const auto &entries = profile.phase_plan->entries();
            for (std::size_t i = 0; i < entries.size(); ++i)
                if (entries[i].stage == session.state().stage &&
                    entries[i].card == session.state().spell + 1)
                    phase_index = int(i);
            if (phase_index >= 0) {
                auto &m = phase_work[std::size_t(phase_index)];
                if (!m.actual_inputs)
                    m.entry_gauge = static_cast<std::int32_t>(actual.state[s::Field::Gauge]);
                ++m.actual_inputs;
                m.planned_inputs += update >= prefix_updates;
                m.fire_inputs += bool(input & 1u);
                m.focus_inputs += bool(input & 4u);
            }
        }
        const int stage_before = int(actual.state[s::Field::Stage]);
        const auto step_begin = Clock::now();
        const auto io_before = session.file_io_time_ns();
        const auto state = session.step(input);
        const auto io = session.file_io_time_ns() - io_before;
        const auto ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - step_begin).count();
        owner_io_ns += io;
        owner_native_ns += std::uint64_t(ns) > io ? std::uint64_t(ns) - io : 0;
        s::advance_receipt(actual, input, s::capture(session), allow_bombs);
        const bool certified = queue.next() != nullptr;
        if (!queue.confirm(actual) && certified)
            throw std::runtime_error("native_style_certificate_execution_mismatch");
        if (effective_horizon > effective_publication) {
            history.push_back(actual);
            if (history.size() > effective_horizon)
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
        // Count actual owned inputs after receipt agreement, excluding the
        // literal prefix. Planning attempts and discarded futures are separate.
        if (update >= prefix_updates)
            (spell_origin ? spell_owner_inputs : road_owner_inputs)++;
        h::score_audit_observe();
        if (phase_index >= 0)
            phase_work[std::size_t(phase_index)].exit_gauge =
                static_cast<std::int32_t>(actual.state[s::Field::Gauge]);
        // Store the actual consumed input after receipt agreement, including
        // any later-declared failing input. A flushed count is not a certificate.
        checkpoint.append(input);
        if (state.player_state == 2 || state.deaths != 0 ||
            (!allow_bombs && actual.state[s::Field::BombsUsed])) {
            outcome = allow_bombs ? "no_miss_failure" : "nmnb_failure";
            break;
        }
        if (verify_captures && s::capture_goal_failed(config.story, stage_before, state)) {
            outcome = "capture_failure";
            break;
        }
        if (stop_card) {
            const auto &spells = h::score_audit().spells;
            const auto stop = std::find_if(spells.begin(), spells.end(), [&](const auto &r) {
                return r.spell + 1 == int(stop_card) && r.outcome != h::SpellAuditOutcome::Open;
            });
            if (stop != spells.end()) {
                outcome = update + 1 < prefix_updates ? "prefix_terminal_before_requested_boundary"
                          : stop->outcome == h::SpellAuditOutcome::Abandoned ? "phase_abandoned"
                                                                             : "phase_boundary";
                break;
            }
        }
        if (s::stage_terminal(config.story, state)) {
            outcome = update + 1 < prefix_updates ? "prefix_terminal_before_requested_boundary"
                                                  : "complete";
            break;
        }
        o::capture_policy_memory(session, actual.sequence, observation, policy);
        policy.observe(observation);
        if (baseline_recovery)
            safety.observe(observation);
    }
    checkpoint.finish();
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
    const bool export_eligible = outcome == "complete" && (!require_captures || captures);
    const bool objective_met = export_eligible && session.state().score * 10ULL > target_score;
    if (export_eligible) {
        const auto child = fork();
        if (child < 0)
            throw std::runtime_error("native_style_writer_fork_failed");
        if (!child) {
            th08::ReplayManager::SaveReplay("native.rpy", replay_name.c_str());
            _exit(fs::exists("native.rpy") ? 0 : 1);
        }
        if (!th08::plan::wait_for_child(child).exited() || !fs::exists("native.rpy"))
            throw std::runtime_error("native_style_writer_failed");
    }
    std::ofstream report("result.json");
    report << "{\"phase_recovery_prior\":" << (phase_recovery_prior ? "true" : "false")
           << ",\"stop_after_card\":" << stop_card << ",\"phase_strategies\":[";
    if (profile.phase_plan) {
        const auto &entries = profile.phase_plan->entries();
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const auto &p = entries[i];
            const auto &m = phase_work[i];
            report << (i ? "," : "") << "{\"stage\":" << p.stage << ",\"card\":" << p.card
                   << ",\"top\":" << p.top << ",\"rows\":" << p.rows << ",\"reserve\":" << p.reserve
                   << ",\"continuous_fire\":"
                   << (p.fire == th08::phase::Fire::Continuous ? "true" : "false")
                   << ",\"focused_capture\":"
                   << (p.capture == th08::phase::Capture::Youkai ? "true" : "false")
                   << ",\"actual_inputs\":" << m.actual_inputs
                   << ",\"planned_inputs\":" << m.planned_inputs
                   << ",\"fire_inputs\":" << m.fire_inputs << ",\"focus_inputs\":" << m.focus_inputs
                   << ",\"entry_gauge\":" << m.entry_gauge << ",\"exit_gauge\":" << m.exit_gauge
                   << "}";
        }
    }
    report << "],";
    report << "\"schema_version\":1,\"evidence\":\"synchronous-native-style-experiment\","
              "\"online_fps_evidence\":false,\"style_revision\":1,\"style\":\""
           << style << "\",\"outcome\":\"" << outcome << "\",\"stage\":" << config.stage
           << ",\"replay_name\":\"" << replay_name << '"'
           << ",\"require_all_captures\":" << (require_captures ? "true" : "false")
           << ",\"allow_bombs\":" << (allow_bombs ? "true" : "false")
           << ",\"verify_captures\":" << (verify_captures ? "true" : "false")
           << ",\"target_score_displayed\":" << target_score
           << ",\"objective_met\":" << (objective_met ? "true" : "false")
           << ",\"replay_exported\":" << (export_eligible ? "true" : "false")
           << ",\"score_goal_revision\":" << (policy.needs_scoring_memory() ? 1 : 0)
           << ",\"collect_items\":" << (profile.scoring_collection ? "true" : "false")
           << ",\"aim_boss\":" << (profile.scoring_aim ? "true" : "false")
           << ",\"survival_focus\":" << (profile.scoring_survival_focus ? "true" : "false")
           << ",\"form_goal_revision\":" << (profile.scoring_survival_focus ? 2 : 0)
           << ",\"survival_focus_bias\":" << profile.survival_focus_bias
           << ",\"survival_flow\":" << (profile.scoring_survival_flow ? "true" : "false")
           << ",\"flow_goal_revision\":" << (profile.scoring_survival_flow ? 1 : 0)
           << ",\"upper_flow\":" << (profile.scoring_upper_flow ? "true" : "false")
           << ",\"flow_grid_revision\":"
           << ((profile.scoring_survival_flow || profile.scoring_attack_pacing)
                   ? (profile.scoring_upper_flow ? 2 : 1)
                   : 0)
           << ",\"flow_grid_min_y\":" << (profile.scoring_upper_flow ? 16 : 208)
           << ",\"attack_pacing\":" << (profile.scoring_attack_pacing ? "true" : "false")
           << ",\"attack_pacing_revision\":" << (profile.scoring_attack_pacing ? 2 : 0)
           << ",\"farm_to_timeout\":" << (profile.scoring_farm_to_timeout ? "true" : "false")
           << ",\"farm_to_timeout_revision\":" << (profile.scoring_farm_to_timeout ? 1 : 0)
           << ",\"farm_charge_fire\":" << (profile.scoring_farm_charge_fire ? "true" : "false")
           << ",\"farm_charge_fire_revision\":" << (profile.scoring_farm_charge_fire ? 1 : 0)
           << ",\"hit_income\":" << (profile.scoring_hit_income ? "true" : "false")
           << ",\"drop_target\":" << (profile.scoring_drop_target ? "true" : "false")
           << ",\"drop_target_revision\":" << (profile.scoring_drop_target ? 1 : 0)
           << ",\"drop_follow\":" << (profile.scoring_drop_follow ? "true" : "false")
           << ",\"drop_follow_revision\":" << (profile.scoring_drop_follow ? 1 : 0)
           << ",\"parent_chain_focus_bias\":" << profile.parent_chain_focus_bias
           << ",\"parent_chain_revision\":" << (profile.parent_chain_focus_bias > 0 ? 2 : 0)
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
           << ",\"publication_horizon\":" << publication << ",\"lead\":" << lead_updates
           << ",\"nominal_warm_updates\":" << nominal_warm_updates
           << ",\"replan_updates\":" << replan_updates << ",\"search_budget_ms\":" << budget_ms
           << ",\"policy_horizon\":" << policy_horizon << ",\"policy_beam\":" << policy_beam
           << ",\"proposal_budget_ms\":60,\"max_updates\":" << maximum_updates
           << ",\"wall_budget_seconds\":" << seconds << ",\"updates\":" << actions.size()
           << ",\"prefix_updates_requested\":" << prefix_updates << ",\"prefix_updates_executed\":"
           << std::min<std::size_t>(prefix_updates, actions.size())
           << ",\"prefix_inputs_available\":" << prefix.size()
           << ",\"checkpoint_interval_updates\":" << checkpoint_updates
           << ",\"checkpoint_saved_updates\":" << checkpoint.saved_updates()
           << ",\"checkpoint_saved_bytes\":" << checkpoint.saved_bytes()
           << ",\"checkpoint_io_ns\":" << checkpoint.io_ns()
           << ",\"score_raw\":" << h::score_audit().score_raw
           << ",\"score_displayed\":" << std::uint64_t(h::score_audit().score_raw) * 10
           << ",\"bombs_used\":" << h::score_audit().bombs_used << ",\"no_miss\":"
           << (session.state().deaths == 0 && session.state().player_state != 2 ? "true" : "false")
           << ",\"captured_spells\":"
           << h::score_capture_coverage(h::score_audit(), outcome == "complete").captured.size()
           << ",\"all_visited_stage_spells_captured\":" << (captures ? "true" : "false")
           << ",\"wall_seconds\":" << elapsed << ",\"searches\":" << work.searches
           << ",\"candidates\":" << work.candidates
           << ",\"nominal_adoptions\":" << work.nominal_adoptions
           << ",\"fresh_adoptions\":" << work.fresh_adoptions
           << ",\"economic_compare_revision\":" << (economic_proposals ? 1 : 0)
           << ",\"economic_proposals\":" << economic_proposals
           << ",\"economic_compare_slice_ms\":" << (economic_proposals ? 2000 : 0)
           << ",\"economic_attempts\":" << work.economic_attempts
           << ",\"economic_adoptions\":" << work.economic_adoptions
           << ",\"economic_movement\":" << economic_movement
           << ",\"economic_movement_revision\":" << (economic_movement ? 1 : 0)
           << ",\"spell_profile_revision\":" << (spell_horizon ? 1 : 0)
           << ",\"road_verify_horizon\":" << road_profile.horizon
           << ",\"road_publication\":" << road_profile.publication
           << ",\"road_replan\":" << road_profile.replan
           << ",\"spell_horizon_requested\":" << spell_horizon
           << ",\"spell_verify_horizon\":" << spell_profile.horizon
           << ",\"spell_publication\":" << spell_profile.publication
           << ",\"spell_replan\":" << spell_profile.replan
           << ",\"profile_switches\":" << profile_switches
           << ",\"discarded_pending_inputs\":" << discarded_pending_inputs
           << ",\"spell_owner_inputs\":" << spell_owner_inputs
           << ",\"road_owner_inputs\":" << road_owner_inputs
           << ",\"motion_economic_attempts\":" << work.motion_economic_attempts
           << ",\"motion_economic_adoptions\":" << work.motion_economic_adoptions
           << ",\"held_recovery\":" << (held_recovery ? "true" : "false")
           << ",\"held_attempts\":" << work.held_attempts
           << ",\"held_adoptions\":" << work.held_adoptions
           << ",\"baseline_recovery\":" << (baseline_recovery ? "true" : "false")
           << ",\"recovery_style\":\"" << recovery_style << '"' << ",\"recovery_fire\":\""
           << recovery_fire << '"' << ",\"safety_attempts\":" << work.safety_attempts
           << ",\"safety_adoptions\":" << work.safety_adoptions
           << ",\"retained_windows\":" << retained_windows
           << ",\"retained_inputs\":" << retained_inputs << ",\"fallback_updates\":" << fallback
           << ",\"future_deaths\":" << work.future_deaths << ",\"timeouts\":" << work.timeouts
           << ",\"future_capture_failures\":" << work.capture_failures
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
    if (outcome == "capture_failure")
        return 3;
    if (outcome != "complete")
        return 2;
    if (require_captures && !captures)
        return 3;
    return target_override && !objective_met ? 5 : 0;
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
