#pragma once

#include "th08/plan_contracts.hpp"
#include <utility>
#include <vector>

namespace th08::plan {

// Streaming validation lets the root hash the exact bytes it parses. Action
// count is bounded by the explicit total-frame budget; token length and input
// whitespace never require an unbounded string allocation.
class PrefixParser {
  public:
    explicit PrefixParser(std::size_t maximum_frames, bool allow_bombs = false)
        : maximum_frames_(maximum_frames), allow_bombs_(allow_bombs) {}

    void append(const char *bytes, std::size_t length) {
        for (std::size_t i = 0; i < length; ++i) {
            const char c = bytes[i];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f') {
                finish_token();
                continue;
            }
            if (c < '0' || c > '9' || value_ > (65535u - unsigned(c - '0')) / 10u)
                throw std::invalid_argument("prefix requires integer actions in 0..65535");
            value_ = value_ * 10u + unsigned(c - '0');
            token_ = true;
        }
    }

    std::vector<std::uint16_t> finish() {
        finish_token();
        return std::move(actions_);
    }

  private:
    void finish_token() {
        if (!token_)
            return;
        if (actions_.size() >= maximum_frames_)
            throw std::invalid_argument("prefix exceeds the total --frames budget");
        actions_.push_back(scoring_input_value(value_, allow_bombs_));
        token_ = false;
        value_ = 0;
    }

    std::size_t maximum_frames_;
    bool allow_bombs_ = false;
    std::vector<std::uint16_t> actions_;
    unsigned value_ = 0;
    bool token_ = false;
};

enum class PrefixBoundary { Resume, Complete, Incomplete, TrailingTerminal };

inline PrefixBoundary prefix_boundary(std::size_t requested, std::size_t executed, bool terminal) {
    if (terminal)
        return executed == requested ? PrefixBoundary::Complete : PrefixBoundary::TrailingTerminal;
    return executed == requested ? PrefixBoundary::Resume : PrefixBoundary::Incomplete;
}

} // namespace th08::plan
