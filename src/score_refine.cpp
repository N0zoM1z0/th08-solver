#include "th08/score_refine.hpp"
#include "modern/headless/score_audit.hpp"
#include "modern/headless/session.hpp"
#include "th08/shadow_native.hpp"
#include <algorithm>
#include <chrono>
#include <tuple>

namespace th08::shadow {
namespace {
using Clock = std::chrono::steady_clock;
using Phase = std::tuple<int, int, std::size_t>;
Phase phase(const headless::State &state) {
    return {state.stage, state.spell_active ? state.spell : -1,
            headless::score_audit().spells.size()};
}
std::uint16_t edited(std::uint16_t input, ScoreEdit edit) {
    switch (edit) {
    case ScoreEdit::Focus:
        return std::uint16_t(input | 4);
    case ScoreEdit::Human:
        return std::uint16_t(input & ~4);
    case ScoreEdit::Fire:
        return std::uint16_t(input | 1);
    case ScoreEdit::ReleaseFire:
        return std::uint16_t(input & ~1);
    case ScoreEdit::BombHuman:
        return std::uint16_t((input | 2) & ~4);
    case ScoreEdit::BombFocus:
        return std::uint16_t(input | 6);
    }
    throw std::runtime_error("unknown_score_edit");
}
} // namespace
const char *score_edit_name(ScoreEdit edit) {
    switch (edit) {
    case ScoreEdit::Focus:
        return "focus";
    case ScoreEdit::Human:
        return "human";
    case ScoreEdit::Fire:
        return "fire";
    case ScoreEdit::ReleaseFire:
        return "release-fire";
    case ScoreEdit::BombHuman:
        return "bomb-human";
    case ScoreEdit::BombFocus:
        return "bomb-focus";
    }
    throw std::runtime_error("unknown_score_edit");
}
ScoreRefineResult refine_score_tape(headless::Session &session, std::vector<std::uint16_t> &inputs,
                                    bool story, const ScoreRefineOptions &options) {
    if ((options.family != ScoreEditFamily::Form && options.family != ScoreEditFamily::Fire &&
         options.family != ScoreEditFamily::Bomb) ||
        (options.family == ScoreEditFamily::Bomb && !options.allow_bombs) || inputs.empty() ||
        inputs.size() > 400000 || options.trials > 10000 || !options.window ||
        options.window > 3600 || !options.interval || options.interval > 400000 ||
        !options.timeout_ms || options.timeout_ms > 60000 || !options.wall_seconds ||
        options.wall_seconds > 7200 || options.first_update > 400000 || session.state().frame != 0)
        throw std::runtime_error("invalid_score_refinement_configuration");
    const auto begin = Clock::now();
    const auto expired = [&] {
        return Clock::now() - begin >= std::chrono::seconds(options.wall_seconds);
    };
    Packet origin;
    origin.state = capture(session);
    ScoreRefineResult result;
    const Boundary boundary{story, -1, options.require_all_captures, options.allow_bombs};
    result.initial = evaluate_tape(session, origin, inputs, 0, boundary, options.timeout_ms);
    if (!result.initial.certifies() || result.initial.completed != inputs.size()) {
        result.wall_seconds = std::chrono::duration<double>(Clock::now() - begin).count();
        return result;
    }
    result.selected = result.initial;
    auto previous_phase = phase(session.state());
    std::size_t phase_start = 0, next_trial = options.first_update;
    const auto edits = options.family == ScoreEditFamily::Form
                           ? std::array<ScoreEdit, 2>{ScoreEdit::Focus, ScoreEdit::Human}
                       : options.family == ScoreEditFamily::Fire
                           ? std::array<ScoreEdit, 2>{ScoreEdit::ReleaseFire, ScoreEdit::Fire}
                           : std::array<ScoreEdit, 2>{ScoreEdit::BombHuman, ScoreEdit::BombFocus};
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        const auto before = session.state();
        const auto current_phase = phase(before);
        if (current_phase != previous_phase) {
            previous_phase = current_phase;
            phase_start = i;
        }
        if (result.trials.size() < options.trials && i >= next_trial && i - phase_start >= 64) {
            next_trial = i + options.interval;
            origin.state = capture(session);
            for (auto edit : edits) {
                if (expired() || result.trials.size() >= options.trials) {
                    result.search_budget_exhausted = true;
                    break;
                }
                auto candidate = inputs;
                const auto end = std::min(inputs.size(), i + options.window);
                bool changed = false;
                for (auto index = i; index < end; ++index) {
                    const auto replacement = edited(candidate[index], edit);
                    changed = changed || replacement != candidate[index];
                    candidate[index] = replacement;
                }
                if (!changed) {
                    ++result.unchanged;
                    continue;
                }
                ScoreRefineTrial trial;
                trial.first = i;
                trial.window = end - i;
                trial.edit = edit;
                trial.evaluation =
                    evaluate_tape(session, origin, candidate, i, boundary, options.timeout_ms);
                const auto &evaluation = trial.evaluation;
                trial.accepted =
                    evaluation.certifies() &&
                    evaluation.expected_spells == result.initial.expected_spells &&
                    evaluation.terminal[Field::Stage] == result.initial.terminal[Field::Stage] &&
                    evaluation.score_displayed() > result.selected.score_displayed();
                if (trial.accepted) {
                    candidate.resize(i + evaluation.completed);
                    inputs = std::move(candidate);
                    result.selected = evaluation;
                }
                result.trials.push_back(std::move(trial));
            }
        }
        const auto io = session.file_io_time_ns();
        const auto start = Clock::now();
        const auto after = session.step(inputs[i]);
        const auto ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count();
        const auto io_ns = session.file_io_time_ns() - io;
        result.owner_file_io_ns += io_ns;
        result.owner_native_ns += std::uint64_t(ns) > io_ns ? std::uint64_t(ns) - io_ns : 0;
        ++result.owner_updates;
        headless::score_audit_observe();
        // These receipts track indices/epochs only. Current state is captured
        // at trial boundaries and after completion, not hashed every update.
        ++origin.sequence;
        origin.epoch += before.stage != after.stage;
        origin.input = inputs[i];
        if (after.player_state == 2 || after.deaths != 0 ||
            (!options.allow_bombs && headless::score_audit().bombs_used))
            throw std::runtime_error("refined_incumbent_execution_failed_nmnb");
        if (stage_terminal(story, after)) {
            if (i + 1 != inputs.size())
                throw std::runtime_error("refined_incumbent_has_trailing_inputs");
            break;
        }
    }
    const auto terminal = capture(session);
    if (!stage_terminal(story, session.state()) ||
        terminal.values != result.selected.terminal.values ||
        (options.require_all_captures &&
         !headless::score_capture_coverage(headless::score_audit(), true).all))
        throw std::runtime_error("refined_incumbent_terminal_disagrees_with_oracle");
    result.search_budget_exhausted =
        result.search_budget_exhausted || expired() || result.trials.size() >= options.trials;
    result.wall_seconds = std::chrono::duration<double>(Clock::now() - begin).count();
    result.complete = true;
    return result;
}
} // namespace th08::shadow
