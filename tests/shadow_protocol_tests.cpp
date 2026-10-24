#include "th08/shadow_compare.hpp"
#include "th08/shadow_plan.hpp"
#include "th08/shadow_protocol.hpp"
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
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    namespace b = s::bridge;
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
    check(b::fallback(0x10f5, 4, false) == 0xf5 && b::fallback(0x45, 5, false) == 0x1045 &&
              b::fallback(0x45, 5, true) == 0x45,
          "declared fallback changed movement/focus or sampled a bomb");
    const auto trace = b::trace(receipt, 0x123456789abcdef0ULL, 2, 3);
    check(!s::difference(receipt, s::decode(trace.data(), s::Bytes{}.size())) &&
              trace[s::Bytes{}.size()] == 0xf0 && trace[s::Bytes{}.size() + 7] == 0x12,
          "timed journal changed packet bytes or clock units");
#endif
    std::cout << "shadow protocol contracts passed\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
