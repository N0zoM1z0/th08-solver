#pragma once
#include "th08/shadow_rollout.hpp"
#include <deque>
#include <optional>
#include <utility>

namespace th08::shadow {
// The execution frontier never moves backwards. A late prediction may replace
// only unexecuted actions, after every already executed receipt matches its
// candidate prefix. A failed native branch cannot supply even a short plan.
class ActionBuffer {
    Packet frontier_;
    std::deque<Packet> pending_;

  public:
    explicit ActionBuffer(Packet initial) : frontier_(std::move(initial)) {}
    const Packet &frontier() const {
        return frontier_;
    }
    std::size_t size() const {
        return pending_.size();
    }
    const Packet *next() const {
        return pending_.empty() ? nullptr : &pending_.front();
    }
    bool replace(const Prediction &prediction,
                 const std::vector<Packet> &executed_since_origin = {}) {
        if (!prediction.certifies() || prediction.origin.sequence > frontier_.sequence)
            return false;
        const auto consumed = frontier_.sequence - prediction.origin.sequence;
        if (consumed >= prediction.frames.size() || executed_since_origin.size() != consumed)
            return false;
        if (!consumed && difference(frontier_, prediction.origin))
            return false;
        Packet previous = prediction.origin;
        for (std::size_t i = 0; i < prediction.frames.size(); ++i) {
            const auto &frame = prediction.frames[i];
            if (frame.kind != Kind::State || frame.status ||
                frame.sequence != previous.sequence + 1 ||
                frame.epoch !=
                    previous.epoch + (frame.state[Field::Stage] != previous.state[Field::Stage]))
                return false;
            input_value(frame.input);
            if (i < consumed && difference(frame, executed_since_origin[i]))
                return false;
            previous = frame;
        }
        if (consumed && difference(frontier_, executed_since_origin.back()))
            return false;
        pending_.assign(prediction.frames.begin() + consumed, prediction.frames.end());
        return true;
    }
    bool confirm(const Packet &actual) {
        input_value(actual.input);
        if (actual.kind != Kind::State || actual.status ||
            actual.sequence != frontier_.sequence + 1 ||
            actual.epoch !=
                frontier_.epoch + (actual.state[Field::Stage] != frontier_.state[Field::Stage]))
            throw std::runtime_error("nonconsecutive_shadow_execution");
        const bool matched = !pending_.empty() && !difference(pending_.front(), actual);
        if (matched)
            pending_.pop_front();
        else
            pending_.clear();
        frontier_ = actual;
        return matched;
    }
};
// A failed tail extension belongs to the nominal plan, even as the actual
// frontier advances. Retrying that same prefix can spend the budget needed for
// fresh alternatives. Only a certificate with a new origin enables reuse again.
class NominalExtension {
    std::optional<std::uint64_t> failed_origin_;

  public:
    bool eligible(const Prediction &plan, const ActionBuffer &queue, unsigned lead) const {
        const auto now = queue.frontier().sequence;
        if (!plan.certifies() || plan.origin.sequence > now ||
            failed_origin_ == plan.origin.sequence)
            return false;
        const auto consumed = now - plan.origin.sequence;
        return consumed < plan.frames.size() && queue.size() == plan.frames.size() - consumed &&
               queue.size() > lead;
    }
    void observe_attempt(const Prediction &plan, const Prediction &extension) {
        if (!extension.certifies())
            failed_origin_ = plan.origin.sequence;
    }
};
} // namespace th08::shadow
