#include "test_support.hpp"
#include "th08/online_live.hpp"
#include <limits>
#include <thread>

namespace on = th08::online;
namespace live = th08::online::live;
using th08::test::check;
namespace {
template <class F> void rejects(F action, const char *message) {
    bool failed = false;
    try {
        action();
    } catch (const std::exception &) {
        failed = true;
    }
    check(failed, message);
}
on::Observation observation(std::uint64_t update) {
    on::Observation o;
    o.update = update;
    o.x = 192;
    o.y = 400;
    o.latched_input = 1;
    o.movement.axis[0] = 4;
    o.movement.axis[1] = 2;
    o.movement.diag[0] = 2.8284271f;
    o.movement.diag[1] = 1.4142135f;
    o.movement.minx = 0;
    o.movement.maxx = 384;
    o.movement.miny = 0;
    o.movement.maxy = 448;
    o.movement.half = {1, 1, 0};
    return o;
}
} // namespace
int main() {
    try {
        const live::Time epoch{};
        check(live::tick_offset(60) == std::chrono::seconds(1), "60 ticks drifted from one second");
        check(live::tick_offset(216000) == std::chrono::hours(1),
              "absolute one-hour schedule drifted");
        rejects([] { live::tick_offset(1000002); }, "unbounded live timestamp accepted");
        const auto deadline = epoch + live::tick_offset(2);
        check(!live::deadline_overrun(deadline, deadline, std::chrono::nanoseconds(0)),
              "exact deadline failed");
        check(live::deadline_overrun(deadline, deadline + std::chrono::nanoseconds(1),
                                     std::chrono::nanoseconds(0)),
              "late owner silently reset schedule");
        check(!live::deadline_overrun(deadline, deadline + std::chrono::microseconds(3),
                                      std::chrono::microseconds(4)),
              "explicit tolerance units changed");
        live::Reply reply;
        reply.update = 7;
        reply.epoch = 3;
        reply.finished = deadline;
        check(live::usable(reply, 7, 3, deadline), "on-time current reply rejected");
        check(!live::usable(reply, 8, 3, deadline) && !live::usable(reply, 7, 4, deadline),
              "stale observation/phase reply adopted");
        check(!live::usable(reply, 7, 3, deadline - std::chrono::nanoseconds(1)),
              "late policy reply adopted");
        live::ObservationQueue queue(2);
        queue.push({observation(0), 0, deadline});
        queue.push({observation(1), 0, deadline});
        rejects(
            [&] {
                queue.push({observation(2), 0, deadline});
            },
            "history queue silently dropped a frame");
        check(queue.pop().observation.update == 0 && queue.pop().observation.update == 1,
              "history queue reordered observations");
        rejects([&] { queue.pop(); }, "empty queue fabricated a snapshot");
        on::Configuration config;
        config.horizon = 24;
        config.beam = 8;
        on::Policy reference(config);
        reference.observe(observation(0));
        reference.observe(observation(1));
        const auto expected = reference.decide(observation(2));
        live::Worker worker(config, {}, {}, 4);
        worker.submit({observation(0), 0, live::Time{}});
        worker.submit({observation(1), 0, live::Time{}});
        const auto future = live::Clock::now() + std::chrono::seconds(5);
        worker.submit({observation(2), 1, future});
        std::optional<live::Reply> actual;
        const auto timeout = live::Clock::now() + std::chrono::seconds(2);
        while (!actual && live::Clock::now() < timeout) {
            actual = worker.take(2, 1, future);
            if (!actual)
                std::this_thread::yield();
        }
        check(actual && actual->decision.action == expected.action &&
                  actual->decision.depth == expected.depth,
              "expired history changed policy state or a late action was reused");
        rejects(
            [&] {
                worker.submit({observation(4), 1, future});
            },
            "missing observation silently skipped");
        worker.stop();
        const auto stats = worker.statistics();
        check(worker.failure().empty() && stats.observed_only == 2 && stats.decisions == 1 &&
                  stats.submitted == 3,
              "worker lost expired observation/cost accounting");
        check(stats.worker_ns >= stats.decision_ns + stats.observe_ns,
              "worker preparation or discarded work disappeared from cost");
        live::Worker unclaimed(config);
        unclaimed.submit({observation(0), 0, live::Clock::now() + std::chrono::seconds(5)});
        const auto unclaimed_timeout = live::Clock::now() + std::chrono::seconds(2);
        while (unclaimed.statistics().decisions == 0 && live::Clock::now() < unclaimed_timeout)
            std::this_thread::yield();
        unclaimed.stop();
        check(unclaimed.statistics().decisions == 1 &&
                  unclaimed.statistics().abandoned_replies == 1,
              "shutdown silently lost completed unused policy work");
        auto invalid = config;
        invalid.planner_lag = 2;
        rejects([&] { live::Worker unsupported(invalid); },
                "synchronous modeled lag masqueraded as async execution");
        live::Worker bad_worker(config);
        auto bad = observation(0);
        bad.x = std::numeric_limits<float>::quiet_NaN();
        bad_worker.submit({std::move(bad), 0, live::Clock::now() + std::chrono::seconds(5)});
        const auto error_timeout = live::Clock::now() + std::chrono::seconds(2);
        while (bad_worker.failure().empty() && live::Clock::now() < error_timeout)
            std::this_thread::yield();
        bad_worker.stop();
        const auto bad_stats = bad_worker.statistics();
        check(!bad_worker.failure().empty() && bad_stats.failed_requests == 1 &&
                  bad_stats.failed_ns > 0 && bad_stats.worker_ns >= bad_stats.failed_ns,
              "failed worker silently continued or discarded its execution cost");
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
