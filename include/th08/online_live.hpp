#pragma once
#include "th08/online.hpp"
#include <chrono>
#include <deque>
#include <memory>
#include <optional>
#include <string>

namespace th08::online::live {
using Clock = std::chrono::steady_clock;
using Time = Clock::time_point;

// API-update ticks include control-only stage handoffs. Compute from the epoch,
// never by adding a rounded period or resetting the clock after delayed work.
std::chrono::nanoseconds tick_offset(std::uint64_t completed_updates);
bool deadline_overrun(Time deadline, Time now, std::chrono::nanoseconds tolerance);

struct Request {
    Observation observation;
    std::uint64_t epoch = 0;
    Time deadline;
};
struct Reply {
    std::uint64_t update = 0, epoch = 0;
    Time finished; // Publication under the mailbox lock, not just search return.
    Decision decision;
};
bool usable(const Reply &, std::uint64_t update, std::uint64_t epoch, Time deadline);

// The worker mutex owns access to this bounded FIFO. Expired observations still
// reach Policy::observe; dropping them would corrupt finite-difference history.
class ObservationQueue {
  public:
    explicit ObservationQueue(std::size_t capacity);
    void push(Request);
    Request pop();
    bool empty() const;
    std::size_t size() const;

  private:
    std::size_t capacity_;
    std::deque<Request> queue_;
};
struct Statistics {
    std::uint64_t submitted = 0, observed_only = 0, decisions = 0;
    std::uint64_t stale_replies = 0, overwritten_replies = 0, abandoned_requests = 0;
    std::uint64_t abandoned_replies = 0, failed_requests = 0;
    std::uint64_t queue_peak = 0, worker_ns = 0, decision_ns = 0, observe_ns = 0, failed_ns = 0;
    std::uint64_t value_evaluations = 0;
    double value_ms = 0;
};

// Only the worker touches the owned Policy. This class never holds a Session,
// native manager pointer or callback. Cancellation invalidates replies and
// stops future jobs; the bounded current search is joined, not force-killed.
class Worker {
  public:
    Worker(Configuration, RiskTables = {}, BranchValueModel = {}, std::size_t capacity = 4);
    ~Worker();
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;
    void submit(Request);
    std::optional<Reply> take(std::uint64_t update, std::uint64_t epoch, Time deadline);
    void stop();
    Statistics statistics() const;
    std::string failure() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace th08::online::live
