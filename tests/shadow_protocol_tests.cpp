#include "th08/shadow_compare.hpp"
#include "th08/shadow_dialogue.hpp"
#include "th08/shadow_plan.hpp"
#include "th08/shadow_protocol.hpp"
#include "th08/shadow_start.hpp"
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#include "th08/shadow_bridge.hpp"
#endif
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace s = th08::shadow;
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
template <class F> void rejects(F run) {
    bool rejected = false;
    try {
        run();
    } catch (const std::runtime_error &) {
        rejected = true;
    }
    check(rejected, "invalid wire accepted");
}
int main() try {
    s::StartRecord start;
    start.seed = 65535;
    start.maximum_updates = 350000;
    start.story = true;
    start.shot = 1;
    start.difficulty = 3;
    const auto legacy = s::encode_start(start);
    check(legacy.find("TH08_SHADOW_START_V1 ") == 0, "legacy start format changed");
    std::istringstream legacy_input(legacy);
    const auto legacy_copy = s::read_start(legacy_input);
    check(s::encode_start(legacy_copy) == legacy && !legacy_copy.advance_dialogue &&
              !legacy_copy.prior_final_a_clear,
          "legacy recording acquired new inputs/history");
    start.prior_final_a_clear = start.advance_dialogue = true;
    const auto extended = s::encode_start(start);
    std::istringstream extended_input(extended);
    const auto copy = s::read_start(extended_input);
    check(extended.find("TH08_SHADOW_START_V2 ") == 0 && copy.prior_final_a_clear &&
              copy.advance_dialogue && s::encode_start(copy) == extended,
          "named history/dialogue profile lost in the start record");
    rejects([&] {
        auto bad = start;
        bad.story = false;
        s::encode_start(bad);
    });
    rejects([&] {
        auto bad = start;
        bad.shot = 4;
        s::encode_start(bad);
    });
    rejects([&] {
        auto bad = start;
        bad.row_bytes--;
        s::encode_start(bad);
    });
    rejects([&] {
        std::istringstream bad(extended + "extra");
        s::read_start(bad);
    });
    rejects([&] {
        auto text = extended;
        text.replace(text.find("228 1 1"), 7, "228 2 1");
        std::istringstream bad(text);
        s::read_start(bad);
    });
    unsigned rising_edges = 0;
    std::uint16_t previous = 0x10f5;
    for (unsigned sequence = 1; sequence <= 16; ++sequence) {
        const auto input = s::dialogue_input(previous, sequence);
        rising_edges += !(previous & 1) && (input & 1);
        check((input & 0xf4) == 0xf4 && !(input & 2) && bool(input & 0x1000) == bool(sequence & 1),
              "dialogue advance changed movement/focus, sampled a bomb or lost Ctrl cadence");
        previous = input;
    }
    check(rising_edges == 2, "held shoot did not create periodic Z rising edges");
    s::Packet p;
    p.sequence = 0x123456789abcdef0ULL;
    p.epoch = 0x1020304050607080ULL;
    p.input = 0x1045;
    p.state[s::Field::X] = 0x80000000u;
    p.state[s::Field::Spell] = 0xffffffffu;
    auto bytes = s::encode(p);
    check(bytes[16] == 0xf0 && bytes[23] == 0x12, "wire byte order changed");
    auto q = s::decode(bytes.data(), bytes.size());
    check(!s::difference(p, q), "wire lost scalar or float bits");
    rejects([&] { s::decode(bytes.data(), bytes.size() - 1); });
    bytes[4] = 2;
    rejects([&] { s::decode(bytes.data(), bytes.size()); });
    rejects([&] {
        p.input = 2;
        s::encode(p);
    });
    p.input = q.input;
    q.state[s::Field::RngSeed] = 1;
    check(std::strcmp(s::difference(p, q), "rng_seed") == 0, "first divergence lost");
    q = p;
    q.epoch++;
    check(std::strcmp(s::difference(p, q), "epoch") == 0, "stale epoch accepted");
    s::Packet receipt;
    auto after = receipt.state;
    after[s::Field::Stage] = 1;
    s::advance_receipt(receipt, 1, after);
    check(receipt.sequence == 1 && receipt.epoch == 1 &&
              receipt.state[s::Field::CalculationLow] == 0,
          "control-only handoff conflated with gameplay calculation");
    s::advance_receipt(receipt, 0x41, after);
    check(receipt.sequence == 2 && receipt.epoch == 1 && receipt.input == 0x41,
          "ordinary receipt changed epoch or actual input");
    s::Packet origin;
    s::Prediction prediction;
    prediction.origin = origin;
    prediction.requested = prediction.attempts = prediction.completed = 3;
    prediction.status = s::BranchStatus::Horizon;
    for (unsigned i = 0; i < 3; ++i) {
        auto next = prediction.frames.empty() ? origin : prediction.frames.back();
        s::advance_receipt(next, 0x41, next.state);
        prediction.frames.push_back(next);
    }
    s::ActionBuffer queue(origin);
    check(queue.replace(prediction) && queue.size() == 3, "verified native plan rejected");
    check(queue.confirm(prediction.frames[0]) && queue.size() == 2,
          "execution frontier did not advance");
    check(!queue.replace(prediction), "late plan accepted without actual prefix receipts");
    check(queue.replace(prediction, {prediction.frames[0]}), "matching late plan rejected");
    auto changed = prediction.frames[0];
    changed.input = 1;
    check(!queue.replace(prediction, {changed}),
          "changed executed input was retroactively replaced");
    auto failed = prediction;
    failed.status = s::BranchStatus::Death;
    check(!failed.certifies() && !queue.replace(failed, {prediction.frames[0]}),
          "failed rollout certified a prefix");
    failed.status = s::BranchStatus::Timeout;
    check(!failed.certifies(), "interrupted rollout certified a prefix");
    auto fallback = prediction.frames[1];
    fallback.input = 1;
    check(!queue.confirm(fallback) && !queue.next() && queue.frontier().sequence == 2,
          "fallback did not invalidate the obsolete plan");
    rejects([&] { queue.confirm(prediction.frames[0]); });
    auto nominal = prediction;
    while (nominal.frames.size() < 32) {
        auto next = nominal.frames.back();
        s::advance_receipt(next, 0x41, next.state);
        nominal.frames.push_back(next);
    }
    nominal.requested = nominal.attempts = nominal.completed = nominal.frames.size();
    s::ActionBuffer buffered(origin);
    s::NominalExtension extension;
    check(!extension.refresh_due(127, 128) && extension.refresh_due(128, 128) &&
              !extension.refresh_due(128, 0),
          "bounded nominal reuse changed its refresh boundary or legacy disabled profile");
    extension.observe_refresh(128);
    check(!extension.refresh_due(129, 128) && extension.refresh_due(256, 128),
          "a failed periodic refresh could starve ordinary reuse on every next origin");
    check(buffered.replace(nominal) && extension.eligible(nominal, buffered, 8),
          "complete nominal continuation was unavailable for reuse");
    auto dead_tail = nominal;
    dead_tail.status = s::BranchStatus::Death;
    extension.observe_attempt(nominal, dead_tail);
    check(buffered.confirm(nominal.frames[0]) && buffered.confirm(nominal.frames[1]) &&
              buffered.size() == 30 && !extension.eligible(nominal, buffered, 8),
          "advancing the owner retried a failed nominal tail or discarded its verified queue");
    auto fresh = nominal;
    fresh.origin = buffered.frontier();
    for (auto &frame : fresh.frames)
        frame.sequence += fresh.origin.sequence;
    check(buffered.replace(fresh) && extension.eligible(fresh, buffered, 8),
          "newly verified alternative could not enable nominal reuse");
    auto timeout_tail = fresh;
    timeout_tail.status = s::BranchStatus::Timeout;
    extension.observe_attempt(fresh, timeout_tail);
    check(buffered.confirm(fresh.frames[0]) && !extension.eligible(fresh, buffered, 8),
          "interrupted nominal extension consumed the next fresh search budget");
    s::ActionBuffer bounded(origin);
    s::NominalExtension bounded_extension;
    check(bounded.replace(nominal, {}, 16) && bounded.size() == 16 &&
              bounded_extension.eligible(nominal, bounded, 8, 16),
          "long certificate could not publish a bounded leading window");
    check(bounded.confirm(nominal.frames[0]) && bounded.replace(nominal, {nominal.frames[0]}, 16) &&
              bounded.size() == 15,
          "bounded publication lost its executed-prefix check");
    auto invalid_tail = nominal;
    invalid_tail.frames.back().sequence++;
    check(!bounded.replace(invalid_tail, {nominal.frames[0]}, 16),
          "bounded publication skipped validation of the unpublished tail");
    auto fatal_tail = nominal;
    fatal_tail.status = s::BranchStatus::Death;
    check(!bounded.replace(fatal_tail, {nominal.frames[0]}, 16) && bounded.size() == 15,
          "failed long rollout certified its surviving prefix");
    check(!bounded.extend(fatal_tail, {nominal.frames[0]}, 16) &&
              !bounded.extend(invalid_tail, {nominal.frames[0]}, 16) && bounded.size() == 15,
          "invalid longer future extended the live queue");
    check(bounded.extend(nominal, {nominal.frames[0]}, 16) && bounded.size() == 16,
          "successful unpublished future was discarded after a tail-extension failure");
    for (unsigned i = 1; i < 16; ++i)
        check(bounded.confirm(nominal.frames[i]), "retained certificate rewrote pending inputs");
    check(bounded.next() && bounded.next()->sequence == 17 && nominal.completed == 32,
          "retained window did not pass the original publication boundary or changed costs");
    s::Packet live, native;
    live.state[s::Field::BulletLow] = 17;
    check(!s::compare_live(live, native, false).failure &&
              s::compare_live(live, native, false).numeric_words == 1 &&
              s::compare_live(live, native, true).failure,
          "harmless digest drift stopped behavioral play or disappeared from exact diagnostics");
    live.state[s::Field::RngSeed] = 17;
    check(std::strcmp(s::compare_live(live, native, false).failure, "rng_seed") == 0,
          "behavioral play silently substituted actual gameplay RNG");
    live.state[s::Field::RngSeed] = 0;
    const float small_error = 1.f / 128.f, large_error = 1.f;
    std::memcpy(&live.state[s::Field::X], &small_error, sizeof(float));
    check(!s::compare_live(live, native, false).failure,
          "subpixel player drift became a behavioral failure");
    std::memcpy(&live.state[s::Field::X], &large_error, sizeof(float));
    check(std::strcmp(s::compare_live(live, native, false).failure, "x") == 0,
          "material player displacement was hidden by numeric tolerance");
    s::Packet retired, previous_spell;
    previous_spell.state[s::Field::Spell] = 9;
    check(!s::compare_live(retired, previous_spell, false).failure &&
              s::compare_live(retired, previous_spell, false).inactive_words == 1,
          "inactive spell metadata stopped continuous play");
    retired.state[s::Field::SpellActive] = previous_spell.state[s::Field::SpellActive] = 1;
    check(std::strcmp(s::compare_live(retired, previous_spell, false).failure, "spell") == 0,
          "active gameplay spell mismatch was hidden as stale metadata");
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    namespace b = s::bridge;
    b::CalculationCursor cursor{0, 123};
    check(cursor.observe(1, 123, false) == b::UpdateEvent::Control && cursor.frame == 0 &&
              cursor.observe(1, 0, false) == b::UpdateEvent::None &&
              cursor.observe(1, 1, true) == b::UpdateEvent::Calculation,
          "asynchronous loading swallowed a control-only stage handoff");
    check(cursor.observe(1, 3, true) == b::UpdateEvent::Unsupported,
          "unexplained skipped gameplay calculations were silently accepted");
    std::array<std::uint32_t, b::map_words> mapping{};
    b::Plan plan;
    plan.generation = 2;
    plan.origin = 10;
    plan.first_change = 19;
    plan.count = 32;
    plan.inputs.fill(0x45);
    b::publish(mapping.data(), plan);
    b::Plan private_copy;
    check(b::read(mapping.data(), private_copy) && private_copy.inputs[31] == 0x45,
          "32/64-bit plan lost an action");
    check(b::timely(private_copy, 10) && b::timely(private_copy, 17) &&
              !b::timely(private_copy, 18) && !b::timely(private_copy, 9),
          "late or future-origin command changed the executed frontier");
    b::Plan old_plan = private_copy;
    check(b::compatible_lead(private_copy, old_plan, 0x45, 12, true),
          "identical immutable lead was rejected");
    auto incompatible = private_copy;
    incompatible.inputs[4] = 0x85;
    check(!b::compatible_lead(incompatible, old_plan, 0x45, 12, true),
          "new plan changed a queued action before its declared first change");
    rejects([&] { b::publish(mapping.data(), plan); });
    auto invalid = plan;
    invalid.generation = 4;
    invalid.inputs[0] = 2;
    rejects([&] { b::publish(mapping.data(), invalid); });
    b::save(mapping.data() + b::slot(plan.generation), 3);
    b::Plan torn;
    check(!b::read(mapping.data(), torn), "partially written plan became active");
    plan.generation = 6; // Reuses the original slot, after its owner copied it.
    plan.inputs.fill(0x85);
    b::publish(mapping.data(), plan);
    check(b::read(mapping.data(), torn) && torn.inputs[0] == 0x85 && private_copy.inputs[0] == 0x45,
          "new publication edited the owner's private plan");
    plan.generation = 8;
    plan.count = b::capacity;
    plan.inputs.back() = 0x41;
    b::publish(mapping.data(), plan);
    check(b::read(mapping.data(), torn) && torn.count == 128 && torn.inputs[127] == 0x41,
          "full 128-update native certificate was truncated at the action bridge");
    check(b::fallback(0x10f5, 4, false) == 0xf5 && b::fallback(0x45, 5, false) == 0x1045 &&
              b::fallback(0x45, 5, true) == 0x45,
          "declared fallback changed movement/focus or sampled a bomb");
    receipt.state[s::Field::Score] = 317919386;
    receipt.state[s::Field::StageComplete] = 1;
    receipt.state[s::Field::ActorLow] = 0xffffffffu;
    const auto trace = b::trace(receipt, 0x123456789abcdef0ULL, 2, 3);
    check(!s::difference(receipt, s::decode(trace.data(), s::Bytes{}.size())) &&
              trace[s::Bytes{}.size()] == 0xf0 && trace[s::Bytes{}.size() + 7] == 0x12,
          "timed journal changed packet bytes or clock units");
    const auto decoded_trace = b::decode_trace(trace.data(), trace.size());
    check(!s::difference(receipt, decoded_trace.packet) &&
              decoded_trace.elapsed_ns == 0x123456789abcdef0ULL && decoded_trace.calc_ns == 2 &&
              decoded_trace.observer_ns == 3,
          "trace decoder shifted gameplay fields into the timing trailer");
    rejects([&] { b::decode_trace(trace.data(), trace.size() - 1); });
    auto bad_trace = trace;
    bad_trace[4] = 2;
    rejects([&] { b::decode_trace(bad_trace.data(), bad_trace.size()); });
#endif
    std::cout << "shadow protocol contracts passed\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
