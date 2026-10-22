#pragma once

#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <sys/wait.h>

namespace th08::plan {

// Shared tapes have a fixed, explicit upper bound; increasing a CLI horizon
// cannot silently truncate the world or allocate an unbounded stack object.
inline constexpr int max_rollout = 240;
inline constexpr int max_beam = 8192;
inline constexpr int holder_slots = 4096;

enum class RolloutStatus {
    Running,
    Horizon,
    Terminal,
    Death,
    Capacity,
    StepError,
    Incomplete,
    ChildError,
};

struct RolloutSummary {
    int frames = 0;
    int death = 0; // One-based native update; zero means no observed death.
    RolloutStatus status = RolloutStatus::Running;
};

inline bool verified_window(const RolloutSummary &r, int horizon) {
    if (r.death || r.frames <= 0 || r.frames > horizon)
        return false;
    return (r.status == RolloutStatus::Horizon && r.frames == horizon) ||
           r.status == RolloutStatus::Terminal;
}

inline bool failed_rollout(const RolloutSummary &r, int horizon) {
    if (verified_window(r, horizon))
        return false;
    return r.status != RolloutStatus::Death || r.death != r.frames || r.frames <= 0 ||
           r.frames > horizon;
}

inline bool fits_tape(std::size_t bullets, std::size_t lasers, std::size_t regions,
                      std::size_t bullet_capacity, std::size_t laser_capacity,
                      std::size_t region_capacity) {
    return bullets <= bullet_capacity && lasers <= laser_capacity && regions <= region_capacity;
}

// Recording mode executes one GameManager calculation, or a controller-only
// stage handoff. Playback may batch calculations and is outside this contract.
inline bool supported_recording_step(std::uint64_t calculations, bool stage_changed) {
    return calculations == 1 || (calculations == 0 && stage_changed);
}

struct ChildResult {
    bool waited = false;
    int status = 0;
    int error = 0;

    bool exited(int expected = 0) const {
        return waited && WIFEXITED(status) && WEXITSTATUS(status) == expected;
    }
};

// Never interpret EINTR, an unrelated child, a signal, or a failed fork as a
// successful rollout. Only this exact child can publish its tape.
inline ChildResult wait_for_child(pid_t pid) {
    if (pid <= 0)
        return {false, 0, EINVAL};
    int status = 0;
    pid_t result;
    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);
    return result == pid ? ChildResult{true, status, 0} : ChildResult{false, 0, errno};
}

template <class Integer> Integer integer(std::string_view value, std::string_view option) {
    Integer result{};
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
        throw std::invalid_argument(std::string(option) + " requires an integer");
    return result;
}

inline std::uint16_t nmnb_input_value(unsigned input) {
    if (input > 65535 || (input & ~0x10f5u))
        throw std::invalid_argument("verification requires supported NMNB input bits");
    return std::uint16_t(input);
}

inline std::uint16_t nmnb_input(std::string_view value) {
    return nmnb_input_value(integer<unsigned>(value, "action"));
}

inline double real(const std::string &value, std::string_view option) {
    char *end = nullptr;
    errno = 0;
    const double result = std::strtod(value.c_str(), &end);
    if (value.empty() || end != value.c_str() + value.size() || errno == ERANGE ||
        !std::isfinite(result))
        throw std::invalid_argument(std::string(option) + " requires a finite number");
    return result;
}

enum class FinalRoute { Natural = 0, A = 1, B = 2 };

struct Options {
    std::string dat, out_dir, verify, prefix;
    int stage = 0, spell = -1, difficulty = 0, seed = 0;
    int rollout = 64, beam = 384, commit = 8, checkpoint = 240, keep = 32;
    int attempts = 4, cegar = 4;
    long frame_limit = 120000;
    double seconds = 3600;
    bool shoot = true, story = false;
    // Disable every planner-generated quiet proposal to match an always-shoot
    // student's action set. Recorded prefixes and route control remain literal.
    bool quiet_retries = true;
    FinalRoute final_route = FinalRoute::Natural;
    int until_stage = 8, shot = 0, measure_every = 0;
    int diversity_cell = 0; // Pixels; zero preserves the existing global score beam.
    float boss_follow = 0.25f;
};

inline void range(bool ok, const char *message) {
    if (!ok)
        throw std::invalid_argument(message);
}

inline Options parse_options(int argc, const char *const *argv) {
    range(argc % 2 == 1, "each option requires a value");
    Options o;
    bool stage_given = false, spell_given = false, frames_given = false, seconds_given = false,
         prefix_given = false;
    for (int i = 1; i < argc; i += 2) {
        const std::string key = argv[i], value = argv[i + 1];
        if (key == "--dat")
            o.dat = value;
        else if (key == "--out")
            o.out_dir = value;
        else if (key == "--verify")
            o.verify = value;
        else if (key == "--prefix") {
            prefix_given = true;
            o.prefix = value;
        } else if (key == "--final-route") {
            range(value == "natural" || value == "a" || value == "b",
                  "--final-route must be natural, a or b");
            o.final_route = value == "a"   ? FinalRoute::A
                            : value == "b" ? FinalRoute::B
                                           : FinalRoute::Natural;
        } else if (key == "--stage") {
            stage_given = true;
            constexpr std::string_view names[] = {"1", "2",  "3",  "4a",   "4b",
                                                  "5", "6a", "6b", "extra"};
            o.stage = -1;
            for (int s = 0; s < 9; ++s)
                if (value == names[s])
                    o.stage = s;
            range(o.stage >= 0, "unknown stage name");
        } else if (key == "--seconds") {
            seconds_given = true;
            o.seconds = real(value, key);
        } else if (key == "--boss-follow")
            o.boss_follow = float(real(value, key));
        else if (key == "--frames") {
            frames_given = true;
            o.frame_limit = integer<long>(value, key);
        } else {
            // Recognize before parsing so malformed unknown options remain errors.
            int *field = nullptr;
            if (key == "--spell") {
                spell_given = true;
                field = &o.spell;
            } else if (key == "--difficulty")
                field = &o.difficulty;
            else if (key == "--seed")
                field = &o.seed;
            else if (key == "--rollout")
                field = &o.rollout;
            else if (key == "--beam")
                field = &o.beam;
            else if (key == "--commit")
                field = &o.commit;
            else if (key == "--checkpoint")
                field = &o.checkpoint;
            else if (key == "--keep")
                field = &o.keep;
            else if (key == "--attempts")
                field = &o.attempts;
            else if (key == "--cegar")
                field = &o.cegar;
            else if (key == "--until-stage")
                field = &o.until_stage;
            else if (key == "--shot")
                field = &o.shot;
            else if (key == "--measure")
                field = &o.measure_every;
            else if (key == "--diversity-cell")
                field = &o.diversity_cell;
            else if (key == "--shoot" || key == "--story" || key == "--quiet-retries") {
                const int flag = integer<int>(value, key);
                range(flag == 0 || flag == 1, "boolean options require 0 or 1");
                if (key == "--shoot")
                    o.shoot = flag != 0;
                else if (key == "--story")
                    o.story = flag != 0;
                else
                    o.quiet_retries = flag != 0;
                continue;
            } else
                throw std::invalid_argument("unknown option: " + key);
            *field = integer<int>(value, key);
        }
    }
    range(!o.dat.empty() && !o.out_dir.empty(), "--dat and --out are required");
    range(!spell_given || stage_given, "--spell requires an explicit --stage");
    range(!prefix_given || !o.prefix.empty(), "--prefix requires a nonempty actions path");
    range(o.prefix.empty() || o.verify.empty(), "--prefix conflicts with --verify");
    range(o.prefix.empty() || (frames_given && seconds_given),
          "--prefix requires explicit --frames and --seconds budgets");
    range(o.spell >= -1 && o.spell < 222, "--spell must be -1..221");
    range(o.difficulty >= 0 && o.difficulty <= 4, "--difficulty must be 0..4");
    range(o.seed >= 0 && o.seed <= 65535, "--seed must be 0..65535");
    range(o.rollout >= 2 && o.rollout <= max_rollout, "--rollout must be 2..240");
    range(o.beam > 0 && o.beam <= max_beam, "--beam must be 1..8192");
    range(o.commit > 0 && o.commit <= o.rollout, "--commit must be 1..rollout");
    range(o.checkpoint > 0, "--checkpoint must be positive");
    range(o.keep > 0 && o.keep < holder_slots - 1, "--keep must be 1..4094");
    range(o.attempts >= 0 && o.attempts <= 1024, "--attempts must be 0..1024");
    range(o.cegar >= 0 && o.cegar <= 64, "--cegar must be 0..64");
    range(o.frame_limit > 0 && o.frame_limit < std::numeric_limits<int>::max(),
          "--frames must be 1..2147483646");
    range(o.seconds > 0, "--seconds must be positive");
    range(o.until_stage >= o.stage && o.until_stage < 9,
          "--until-stage must include the starting stage and be 0..8");
    range(o.shot >= 0 && o.shot < 12, "--shot must be 0..11");
    range(o.measure_every >= 0, "--measure must be nonnegative");
    range(o.diversity_cell >= 0 && o.diversity_cell <= 1024,
          "--diversity-cell must be 0..1024 pixels");
    range(o.boss_follow >= 0 && o.boss_follow <= 1, "--boss-follow must be 0..1");
    range(!o.story || o.spell < 0, "--story cannot use a spell checkpoint");
    range(!o.story || ((o.stage == 8) == (o.difficulty == 4)),
          "full Extra requires --stage extra --difficulty 4; standard stories require 0..3");
    range(o.final_route == FinalRoute::Natural || (o.story && o.stage < 6),
          "--final-route requires a standard story starting before its final stage");
    return o;
}

} // namespace th08::plan
