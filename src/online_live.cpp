#include "th08/online_live.hpp"
#include <algorithm>
#include <cfenv>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace th08::online::live {
namespace {
std::uint64_t elapsed(Time begin) {
    return std::uint64_t(
        std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count());
}
} // namespace
std::chrono::nanoseconds tick_offset(std::uint64_t updates) {
    if (updates > 1000001)
        throw std::runtime_error("live_tick_capacity");
    return std::chrono::nanoseconds(updates * 1000000000ULL / 60);
}
bool deadline_overrun(Time deadline, Time now, std::chrono::nanoseconds tolerance) {
    if (tolerance.count() < 0)
        throw std::runtime_error("invalid_live_tolerance");
    return now > deadline + tolerance;
}
bool usable(const Reply &reply, std::uint64_t update, std::uint64_t epoch, Time deadline) {
    return reply.update == update && reply.epoch == epoch && reply.finished <= deadline;
}
ObservationQueue::ObservationQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity == 0 || capacity > 64)
        throw std::runtime_error("invalid_live_queue_capacity");
}
void ObservationQueue::push(Request request) {
    if (queue_.size() == capacity_)
        throw std::runtime_error("live_observation_queue_capacity");
    queue_.push_back(std::move(request));
}
Request ObservationQueue::pop() {
    if (queue_.empty())
        throw std::runtime_error("empty_live_observation_queue");
    auto request = std::move(queue_.front());
    queue_.pop_front();
    return request;
}
bool ObservationQueue::empty() const {
    return queue_.empty();
}
std::size_t ObservationQueue::size() const {
    return queue_.size();
}

struct Worker::Impl {
    mutable std::mutex mutex;
    std::condition_variable condition;
    ObservationQueue queue;
    std::optional<Reply> reply;
    Statistics stats;
    std::string error;
    bool stopping = false, stop_accounted = false;
    std::uint64_t next_update = 0;
    Policy policy;
    std::fenv_t environment;
    std::thread thread;
    Impl(Configuration config, RiskTables risk, BranchValueModel model, std::size_t capacity)
        : queue(capacity), policy(config, std::move(risk), std::move(model)) {
        if (config.planner_lag || config.warm_fallback || config.held_fallback || config.tail_check)
            throw std::runtime_error("unsupported_live_policy_configuration");
        if (std::fegetenv(&environment) != 0)
            throw std::runtime_error("unsupported_live_float_environment");
        thread = std::thread([this] { run(); });
    }
    void run() {
        Time begin;
        bool working = false;
        try {
            if (std::fesetenv(&environment) != 0)
                throw std::runtime_error("unsupported_live_float_environment");
            for (;;) {
                Request request;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    condition.wait(lock, [&] { return stopping || !queue.empty(); });
                    if (stopping)
                        break;
                    request = queue.pop();
                }
                begin = Clock::now();
                working = true;
                const bool expired = begin >= request.deadline;
                Decision decision;
                if (expired)
                    policy.observe(request.observation);
                else
                    decision = policy.decide(request.observation);
                const auto finished = Clock::now();
                const auto ns = std::uint64_t(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(finished - begin).count());
                std::lock_guard<std::mutex> lock(mutex);
                stats.worker_ns += elapsed(begin);
                working = false;
                if (expired) {
                    ++stats.observed_only;
                    stats.observe_ns += ns;
                } else {
                    ++stats.decisions;
                    stats.decision_ns += ns;
                    stats.value_evaluations += decision.search.value_evaluations;
                    stats.value_ms += decision.search.value_ms;
                    if (reply)
                        ++stats.overwritten_replies;
                    reply =
                        Reply{request.observation.update, request.epoch, Clock::now(), decision};
                }
            }
        } catch (const std::exception &) {
            std::lock_guard<std::mutex> lock(mutex);
            if (working) {
                const auto ns = elapsed(begin);
                stats.worker_ns += ns;
                stats.failed_ns += ns;
                ++stats.failed_requests;
            }
            error = "live_policy_failed";
            stopping = true;
        }
    }
};
Worker::Worker(Configuration config, RiskTables risk, BranchValueModel model, std::size_t capacity)
    : impl_(new Impl(config, std::move(risk), std::move(model), capacity)) {}
Worker::~Worker() {
    stop();
}
void Worker::submit(Request request) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->stopping || !impl_->error.empty())
        throw std::runtime_error("live_worker_stopped");
    if (request.observation.update != impl_->next_update)
        throw std::runtime_error("nonconsecutive_live_submission");
    impl_->queue.push(std::move(request));
    ++impl_->next_update;
    ++impl_->stats.submitted;
    impl_->stats.queue_peak = std::max(impl_->stats.queue_peak, std::uint64_t(impl_->queue.size()));
    impl_->condition.notify_one();
}
std::optional<Reply> Worker::take(std::uint64_t update, std::uint64_t epoch, Time deadline) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    auto reply = std::move(impl_->reply);
    impl_->reply.reset();
    if (reply && !usable(*reply, update, epoch, deadline)) {
        ++impl_->stats.stale_replies;
        reply.reset();
    }
    return reply;
}
void Worker::stop() {
    if (!impl_)
        return;
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->stopping = true;
        if (!impl_->stop_accounted) {
            impl_->stop_accounted = true;
            impl_->stats.abandoned_requests += impl_->queue.size();
        }
    }
    impl_->condition.notify_one();
    if (impl_->thread.joinable())
        impl_->thread.join();
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        if (impl_->reply) {
            ++impl_->stats.abandoned_replies;
            impl_->reply.reset();
        }
    }
}
Statistics Worker::statistics() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->stats;
}
std::string Worker::failure() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->error;
}
} // namespace th08::online::live
