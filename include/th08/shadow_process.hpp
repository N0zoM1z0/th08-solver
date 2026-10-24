#pragma once
#include "th08/shadow_protocol.hpp"
#include <memory>

namespace th08::headless {
struct Config;
}
namespace th08::shadow {
// Construct on a single-threaded caller before creating its native Session.
// The child initializes independently in its own scratch directory. Its only
// mutations are acknowledged real inputs; this follower performs no lookahead.
class Follower {
  public:
    explicit Follower(const headless::Config &config, unsigned timeout_ms = 30000);
    ~Follower();
    Follower(const Follower &) = delete;
    Follower &operator=(const Follower &) = delete;
    Packet initial();
    Packet advance(std::uint64_t sequence, std::uint64_t epoch, std::uint32_t input);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace th08::shadow
