#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace th08::phase {
// Keys are hashes bound by the campaign owner to its exact episode, program,
// input history and requested controls. They are not the41-word projection or
// a quantization of score/PIV. The selector never restores or certifies a world.
struct Candidate {
    std::string id, boundary, family, request_key, checkpoint_key;
    std::uint64_t score = 0;
    // completed refers to this named segment boundary. A completed spell entry
    // remains a partial route and cannot authorize replay export or Wine playback.
    bool completed = false, no_miss = false;
    unsigned bombs = 0;
};
// Select one representative per strategy family before taking a second world
// from a family. Rotate family admission across generations when width is small.
// Scalar dominance across different worlds is deliberately not a pruning rule.
// Failed/incomplete candidates are excluded. Identical request+checkpoint pairs
// are duplicates; different realized checkpoints from the same request survive.
std::vector<Candidate> frontier(std::vector<Candidate>, unsigned width, std::uint64_t generation);
} // namespace th08::phase
