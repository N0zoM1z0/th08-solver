#include "modern/headless/session.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <th08/direct_laser_policy.hpp>
#include <th08/native_policy.hpp>
#include <th08/reactive.hpp>
#include <th08/spell_policy.hpp>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
namespace {
unsigned number(const std::string &text, unsigned maximum) {
    std::size_t end = 0;
    const auto value = std::stoul(text, &end);
    if (end != text.size() || value > maximum || text.empty() || text.front() == '-')
        throw std::runtime_error("invalid numeric option: " + text);
    return unsigned(value);
}
int stage_index(const std::string &name) {
    const std::array<std::string, 9> names{"1", "2", "3", "4a", "4b", "5", "6a", "6b", "extra"};
    const auto found = std::find(names.begin(), names.end(), name);
    if (found == names.end())
        throw std::runtime_error("unknown stage: " + name);
    return int(found - names.begin());
}
std::string dat_hash(const std::string &path) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw std::runtime_error("cannot read DAT");
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx || EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1)
        throw std::runtime_error("SHA256 initialization failed");
    std::array<char, 65536> buffer;
    while (input.read(buffer.data(), buffer.size()) || input.gcount())
        if (EVP_DigestUpdate(ctx.get(), buffer.data(), input.gcount()) != 1)
            throw std::runtime_error("SHA256 update failed");
    if (!input.eof())
        throw std::runtime_error("DAT read failed");
    std::array<unsigned char, 32> digest;
    unsigned size = 0;
    if (EVP_DigestFinal_ex(ctx.get(), digest.data(), &size) != 1 || size != digest.size())
        throw std::runtime_error("SHA256 finalization failed");
    std::ostringstream out;
    for (auto byte : digest)
        out << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return out.str();
}
class RunDirectory {
  public:
    RunDirectory() : previous(fs::current_path()) {
        char scratch[] = "/tmp/th08-headless-XXXXXX";
        if (!mkdtemp(scratch))
            throw std::runtime_error("cannot create isolated runtime directory");
        path = scratch;
        fs::current_path(path);
    }
    ~RunDirectory() {
        std::error_code error;
        fs::current_path(previous, error);
        fs::remove_all(path, error);
    }

  private:
    fs::path previous, path;
};
void hash(std::uint64_t &digest, std::uint64_t value) {
    digest = (digest ^ value) * 1099511628211ULL;
}
std::uint32_t bits(float value) {
    std::uint32_t out;
    std::memcpy(&out, &value, sizeof(out));
    return out;
}
std::vector<std::uint16_t> read_tape(const std::string &path) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open action tape");
    std::vector<std::uint16_t> actions;
    std::string token;
    while (input >> token) {
        if (actions.size() == 1000000)
            throw std::runtime_error("action tape exceeds frame bound");
        actions.push_back(number(token, 0xffff));
    }
    if (!input.eof() || actions.empty())
        throw std::runtime_error("invalid or empty action tape");
    return actions;
}
struct TraceFrame {
    th08::headless::State before, after;
    std::uint16_t action = 0;
    std::vector<th08::headless::BulletView> before_bullets, after_bullets;
    std::vector<th08::headless::LaserView> before_lasers, after_lasers;
    std::vector<th08::headless::LaserHitboxView> before_laser_hitboxes, after_laser_hitboxes;
    std::vector<th08::headless::EclContextView> before_ecl, after_ecl;
    th08::policy::HazardReactiveDecision policy_decision;
    bool has_policy_decision = false;
};
// Fixed tail, reused across updates. Serialization happens after timing stops.
class TraceTail {
  public:
    void before(const th08::headless::State &state, std::uint16_t action,
                const std::vector<th08::headless::BulletView> &bullets,
                const std::vector<th08::headless::LaserView> &lasers,
                const std::vector<th08::headless::LaserHitboxView> &laser_hitboxes,
                const std::vector<th08::headless::EclContextView> &ecl,
                const th08::policy::HazardReactiveDecision *policy_decision) {
        auto &f = frames[next];
        f.before = state;
        f.action = action;
        f.before_bullets.assign(bullets.begin(), bullets.end());
        f.before_lasers.assign(lasers.begin(), lasers.end());
        f.before_laser_hitboxes.assign(laser_hitboxes.begin(), laser_hitboxes.end());
        f.before_ecl.assign(ecl.begin(), ecl.end());
        f.has_policy_decision = policy_decision != nullptr;
        if (policy_decision)
            f.policy_decision = *policy_decision;
    }
    void after(const th08::headless::State &state,
               const std::vector<th08::headless::BulletView> &bullets,
               const std::vector<th08::headless::LaserView> &lasers,
               const std::vector<th08::headless::LaserHitboxView> &laser_hitboxes,
               const std::vector<th08::headless::EclContextView> &ecl) {
        auto &f = frames[next];
        f.after = state;
        f.after_bullets.assign(bullets.begin(), bullets.end());
        f.after_lasers.assign(lasers.begin(), lasers.end());
        f.after_laser_hitboxes.assign(laser_hitboxes.begin(), laser_hitboxes.end());
        f.after_ecl.assign(ecl.begin(), ecl.end());
        next = (next + 1) % frames.size();
        count = std::min(count + 1, frames.size());
    }
    void write(const std::string &path) const {
        std::ofstream out(path);
        if (!out)
            throw std::runtime_error("cannot open diagnostic trace");
        out << std::setprecision(std::numeric_limits<float>::max_digits10)
            << "frame\tphase\tkind\tslot\tstate\tx\ty\tvx\tvy\twidth\theight\ttransforms\t"
               "action\tlatched_input\tsampled_input\torigin_x\torigin_y\tangle\tgraze_enabled\t"
               "ecl_sub_id\tecl_time\tnext_time\tnext_offset\tdifficulty_mask\toperand_flags\t"
               "raw_int0\thas_raw_int0\tvariable0\tvariable1\tsecondary_time\t"
               "pending_subroutine\tactive_interpolations\tper_frame_ex\tdifficulty_enabled\t"
               "enemy_flags\thas_parent\trotation_velocity\tlaser_start_offset\t"
               "laser_end_offset\tlaser_start_length\tlaser_width\tlaser_speed\t"
               "laser_start_time\tlaser_hitbox_start_time\tlaser_duration\t"
               "laser_despawn_duration\tlaser_hitbox_end_delay\tlaser_timer\tlaser_flags\t"
               "laser_motion_observed\tlaser_origin_delta_x\tlaser_origin_delta_y\t"
               "laser_angle_delta\tpolicy_enabled\tpolicy_first_overlap\t"
               "policy_minimum_clearance\tpolicy_danger\tpolicy_center_distance\t"
               "policy_selected\n";
        for (std::size_t i = 0; i < count; ++i) {
            const auto &f = frames[(next + frames.size() - count + i) % frames.size()];
            auto zeros = [&](unsigned n) {
                while (n--)
                    out << "\t0";
            };
            auto phase = [&](const char *name, const auto &s, const auto &bullets,
                             const auto &lasers, const auto &laser_hitboxes, const auto &ecl) {
                out << f.after.frame << '\t' << name << "\tplayer\t-1\t" << s.player_state << '\t'
                    << s.x << '\t' << s.y << "\t0\t0\t" << 2 * s.hurt_half_x << '\t'
                    << 2 * s.hurt_half_y << "\t0\t" << f.action << '\t' << s.latched_input << '\t'
                    << s.sampled_input;
                zeros(44);
                out << '\n';
                if (!std::strcmp(name, "before") && f.has_policy_decision)
                    for (std::size_t candidate_index = 0;
                         candidate_index < f.policy_decision.candidates.size(); ++candidate_index) {
                        const auto &candidate = f.policy_decision.candidates[candidate_index];
                        out << f.after.frame << "\tbefore\tpolicy_candidate\t" << candidate_index
                            << '\t' << candidate.enabled;
                        zeros(7);
                        out << '\t' << candidate.action;
                        zeros(40);
                        out << '\t' << candidate.enabled << '\t' << candidate.first_overlap << '\t'
                            << candidate.minimum_clearance << '\t' << candidate.danger << '\t'
                            << candidate.center_distance << '\t'
                            << (candidate.enabled &&
                                candidate.action == f.policy_decision.selected_action)
                            << '\n';
                    }
                for (const auto &b : bullets) {
                    out << f.after.frame << '\t' << name << "\tbullet\t" << b.slot << '\t'
                        << b.state << '\t' << b.x << '\t' << b.y << '\t' << b.vx << '\t' << b.vy
                        << '\t' << b.full_width << '\t' << b.full_height << '\t'
                        << b.active_transforms << '\t' << f.action << '\t' << s.latched_input
                        << '\t' << s.sampled_input;
                    zeros(44);
                    out << '\n';
                }
                for (const auto &laser : lasers) {
                    out << f.after.frame << '\t' << name << "\tpooled_laser\t" << laser.slot << '\t'
                        << unsigned(laser.state) << '\t' << laser.origin_x << '\t' << laser.origin_y
                        << '\t' << laser.origin_delta_x << '\t' << laser.origin_delta_y << '\t'
                        << laser.start_length << '\t' << laser.width << '\t' << laser.flags << '\t'
                        << f.action << '\t' << s.latched_input << '\t' << s.sampled_input << '\t'
                        << laser.origin_x << '\t' << laser.origin_y << '\t' << laser.angle << "\t0";
                    zeros(18);
                    out << '\t' << laser.start_offset << '\t' << laser.end_offset << '\t'
                        << laser.start_length << '\t' << laser.width << '\t' << laser.speed << '\t'
                        << laser.start_time << '\t' << laser.hitbox_start_time << '\t'
                        << laser.duration << '\t' << laser.despawn_duration << '\t'
                        << laser.hitbox_end_delay << '\t' << laser.timer << '\t' << laser.flags
                        << '\t' << laser.motion_observed << '\t' << laser.origin_delta_x << '\t'
                        << laser.origin_delta_y << '\t' << laser.angle_delta;
                    zeros(6);
                    out << '\n';
                }
                for (const auto &h : laser_hitboxes) {
                    out << f.after.frame << '\t' << name << "\tlaser_hitbox\t" << h.pooled_slot
                        << "\t0\t" << h.center_x << '\t' << h.center_y << "\t0\t0\t" << h.full_width
                        << '\t' << h.full_height << "\t0\t" << f.action << '\t' << s.latched_input
                        << '\t' << s.sampled_input << '\t' << h.origin_x << '\t' << h.origin_y
                        << '\t' << h.angle << '\t' << h.graze_enabled;
                    zeros(40);
                    out << '\n';
                }
                for (const auto &e : ecl) {
                    out << f.after.frame << '\t' << name << "\tecl_context\t" << e.enemy_index
                        << '\t' << e.child_slot << '\t' << e.position_x << '\t' << e.position_y
                        << '\t' << e.velocity_x << '\t' << e.velocity_y << "\t0\t0\t"
                        << e.next_opcode << '\t' << f.action << '\t' << s.latched_input << '\t'
                        << s.sampled_input << '\t' << e.position_offset_x << '\t'
                        << e.position_offset_y << '\t' << e.rotation << "\t0\t" << e.sub_id << '\t'
                        << e.time << '\t' << e.next_time << '\t' << e.next_offset << '\t'
                        << unsigned(e.difficulty_mask) << '\t' << e.operand_flags << '\t'
                        << e.raw_int0 << '\t' << e.has_raw_int0 << '\t' << e.variable0 << '\t'
                        << e.variable1 << '\t' << e.secondary_time << '\t' << e.pending_subroutine
                        << '\t' << e.active_interpolations << '\t' << e.per_frame_ex << '\t'
                        << e.difficulty_enabled << '\t' << e.enemy_flags << '\t' << e.has_parent
                        << '\t' << e.rotation_velocity;
                    zeros(22);
                    out << '\n';
                }
            };
            phase("before", f.before, f.before_bullets, f.before_lasers, f.before_laser_hitboxes,
                  f.before_ecl);
            phase("after", f.after, f.after_bullets, f.after_lasers, f.after_laser_hitboxes,
                  f.after_ecl);
        }
        if (!out)
            throw std::runtime_error("cannot write diagnostic trace");
    }

  private:
    std::array<TraceFrame, 32> frames;
    std::size_t next = 0, count = 0;
};
void write_collision(std::ostream &out, const th08::headless::CollisionEvent &event) {
    using Kind = th08::headless::CollisionKind;
    const char *kind = event.kind == Kind::Bullet         ? "bullet"
                       : event.kind == Kind::LethalRegion ? "lethal_region"
                       : event.kind == Kind::Laser        ? "laser"
                                                          : "none";
    auto bounds = [&](const auto &b) {
        out << '[' << b.left << ',' << b.top << ',' << b.right << ',' << b.bottom << ']';
    };
    out << ",\"collision\":{\"kind\":\"" << kind << "\",\"frame\":" << event.frame
        << ",\"bullet_slot\":" << event.bullet_slot << ",\"player_bounds\":";
    bounds(event.player);
    out << ",\"hazard_bounds\":";
    bounds(event.hazard);
    out << ",\"vx\":" << event.vx << ",\"vy\":" << event.vy
        << ",\"active_transforms\":" << event.active_transforms
        << ",\"laser_slot\":" << event.laser_slot
        << ",\"laser_hitbox_call\":" << event.laser_hitbox_call << ",\"laser_center\":["
        << event.laser_center_x << ',' << event.laser_center_y << ']' << ",\"laser_size\":["
        << event.laser_full_width << ',' << event.laser_full_height << ']' << ",\"laser_origin\":["
        << event.laser_origin_x << ',' << event.laser_origin_y << ']'
        << ",\"laser_angle\":" << event.laser_angle
        << ",\"movement_input\":" << event.movement_input
        << ",\"sampled_input\":" << event.sampled_input << '}';
}
} // namespace

// The engine adapter owns the game. This CLI owns options, tapes and reports.
int main(int argc, char **argv) {
    try {
        th08::headless::Config config;
        std::string dat, output, strategy = "stationary", tape_path, replay_path, stage_name = "1";
        std::string trace_path;
        unsigned limit = 20000;
        unsigned prefix_frame = 0;
        bool allow_unused = false;
        bool shoot = false, shoot_set = false;
        for (int i = 1; i < argc; i += 2) {
            if (i + 1 >= argc)
                throw std::runtime_error("option requires a value");
            const std::string key = argv[i], value = argv[i + 1];
            if (key == "--dat")
                dat = fs::absolute(value).string();
            else if (key == "--spell-id")
                config.spell = number(value, 221);
            else if (key == "--stage") {
                config.stage = stage_index(value);
                stage_name = value;
            } else if (key == "--difficulty")
                config.difficulty = number(value, 4);
            else if (key == "--seed")
                config.seed = number(value, 65535);
            else if (key == "--frames")
                limit = number(value, 1000000);
            else if (key == "--prefix-frame")
                prefix_frame = number(value, 1000000);
            else if (key == "--allow-unused-actions")
                allow_unused = number(value, 1);
            else if (key == "--output")
                output = fs::absolute(value).string();
            else if (key == "--actions")
                tape_path = fs::absolute(value).string();
            else if (key == "--replay")
                replay_path = fs::absolute(value).string();
            else if (key == "--trace")
                trace_path = fs::absolute(value).string();
            else if (key == "--strategy")
                strategy = value;
            else if (key == "--shoot") {
                shoot = number(value, 1);
                shoot_set = true;
            } else
                throw std::runtime_error("unknown option: " + key);
        }
        if (dat.empty() || !limit)
            throw std::runtime_error("--dat and a positive frame budget are required");
        if (prefix_frame > limit || (allow_unused && replay_path.empty()))
            throw std::runtime_error("invalid replay/diagnostic boundary");
        if (strategy != "stationary" && strategy != "reactive" && strategy != "hazard-reactive" &&
            strategy != "hazard-reactive-linear" && strategy != "spell-portfolio")
            throw std::runtime_error("unknown strategy");
        if (!shoot_set)
            shoot = config.spell < 0;
        const auto identity = dat_hash(dat);
        if (identity != "9d7edf43b8ddd347cbb641836f6b5050745dd936f688daebbf9382ca557043bb")
            throw std::runtime_error("DAT identity mismatch");
        auto replay = replay_path.empty() ? std::vector<std::uint16_t>{} : read_tape(replay_path);
        config.dat_path = dat.c_str();
        RunDirectory scratch;
        th08::headless::Session session(config);
        std::unique_ptr<TraceTail> tail;
        if (!trace_path.empty())
            tail = std::make_unique<TraceTail>();
        std::vector<std::uint16_t> actions;
        actions.reserve(limit);
        const auto initial_io_ns = session.file_io_time_ns();
        const auto start = std::chrono::steady_clock::now();
        double update_ms = 0, decision_ms = 0, diagnostics_ms = 0;
        th08::policy::HazardReactiveStats policy_stats;
        th08::policy::DirectLaserStats direct_laser_stats;
        std::vector<th08::policy::DirectLaserWarning> direct_laser_warnings;
        direct_laser_warnings.reserve(16);
        std::uint64_t linear_profile_decisions = 0;
        const char *last_policy_profile = "none";
        auto state = session.state();
        bool started = false, complete = false;
        unsigned peak = 0, first_hit = 0, spell_start = 0;
        std::string outcome = "frame_limit";
        std::uint64_t digest = 1469598103934665603ULL;
        std::uint64_t previous_digest = digest;
        std::uint64_t prefix_digest = digest;
        for (unsigned i = 0; i < limit; ++i) {
            const auto decision_start = std::chrono::steady_clock::now();
            const auto &observed = session.bullets();
            const auto &observed_lasers = session.lasers();
            const auto &observed_laser_hitboxes = session.laser_hitboxes();
            std::uint16_t action = 4;
            th08::policy::HazardReactiveDecision policy_decision;
            bool has_policy_decision = false;
            if (!replay_path.empty()) {
                if (i == replay.size()) {
                    outcome = "tape_end";
                    break;
                }
                action = replay[i];
            } else {
                if (strategy == "reactive")
                    action = th08::policy::reactive(state.x, state.y, session.focused_axis_speed(),
                                                    session.focused_diagonal_speed(), observed);
                else if (strategy == "hazard-reactive" || strategy == "hazard-reactive-linear" ||
                         strategy == "spell-portfolio") {
                    auto profile = th08::policy::native_spell_policy(-1);
                    if (strategy == "hazard-reactive-linear")
                        profile = {"linear-ranking", {12, 120, false}};
                    else if (strategy == "spell-portfolio") {
                        const int policy_spell = config.spell >= 0
                                                     ? config.spell
                                                     : (state.spell_active ? state.spell : -1);
                        profile = th08::policy::native_spell_policy(policy_spell);
                    }
                    last_policy_profile = profile.name;
                    if (!profile.hazards.vector_acceleration)
                        ++linear_profile_decisions;
                    if (profile.direct_ecl_lasers) {
                        th08::policy::collect_direct_laser_warnings(
                            session.ecl_contexts(), profile.hazards.laser_horizon,
                            direct_laser_warnings, direct_laser_stats);
                        profile.hazards.candidate_mask = th08::policy::direct_laser_candidate_mask(
                            state.x, state.y, state.hurt_half_x, state.hurt_half_y,
                            session.focused_axis_speed(), session.focused_diagonal_speed(),
                            state.latched_input, direct_laser_warnings, direct_laser_stats);
                    }
                    action = th08::policy::hazard_reactive(
                        state.x, state.y, state.hurt_half_x, state.hurt_half_y,
                        session.focused_axis_speed(), session.focused_diagonal_speed(),
                        state.latched_input, observed, observed_lasers, policy_stats,
                        profile.hazards, tail ? &policy_decision : nullptr);
                    has_policy_decision = tail != nullptr;
                }
                if (shoot)
                    action |= 1;
                // Alternating confirm advances real message scripts; timeline
                // and dialogue updates remain in the original game calc chain.
                if (config.spell < 0 && i % 2)
                    action |= 4096;
            }
            const auto decision_end = std::chrono::steady_clock::now();
            decision_ms +=
                std::chrono::duration<double, std::milli>(decision_end - decision_start).count();
            if (tail)
                tail->before(state, action, observed, observed_lasers, observed_laser_hitboxes,
                             session.ecl_contexts(),
                             has_policy_decision ? &policy_decision : nullptr);
            const auto update_start = std::chrono::steady_clock::now();
            if (tail)
                diagnostics_ms +=
                    std::chrono::duration<double, std::milli>(update_start - decision_end).count();
            state = session.step(action);
            update_ms += std::chrono::duration<double, std::milli>(
                             std::chrono::steady_clock::now() - update_start)
                             .count();
            actions.push_back(action);
            peak = std::max(peak, unsigned(state.bullets));
            previous_digest = digest;
            hash(digest, state.frame);
            hash(digest, action);
            hash(digest, state.rng_seed);
            hash(digest, bits(state.x));
            hash(digest, bits(state.y));
            hash(digest, state.player_state);
            hash(digest, state.spell_active);
            hash(digest, state.rng_draws);
            hash(digest, state.score);
            hash(digest, state.graze);
            hash(digest, state.gauge);
            hash(digest, bits(state.lives));
            hash(digest, session.actor_digest());
            const auto &updated_bullets = session.bullets();
            const auto &updated_laser_hitboxes = session.laser_hitboxes();
            if (tail) {
                const auto trace_start = std::chrono::steady_clock::now();
                const auto &updated_lasers = session.lasers();
                tail->after(state, updated_bullets, updated_lasers, updated_laser_hitboxes,
                            session.ecl_contexts());
                diagnostics_ms += std::chrono::duration<double, std::milli>(
                                      std::chrono::steady_clock::now() - trace_start)
                                      .count();
            }
            for (const auto &b : updated_bullets) {
                hash(digest, b.state);
                hash(digest, bits(b.x));
                hash(digest, bits(b.y));
                hash(digest, bits(b.vx));
                hash(digest, bits(b.vy));
            }
            if (state.frame == prefix_frame)
                prefix_digest = digest;
            if (state.spell_active && !started) {
                started = true;
                spell_start = state.frame;
                if (config.spell >= 0 && state.spell != config.spell)
                    throw std::runtime_error(
                        "practice checkpoint selected a different spell; check stage/difficulty");
            }
            if (state.player_state == 2) {
                first_hit = state.frame;
                outcome = "collision";
                break;
            }
            complete = config.spell >= 0 ? started && !state.spell_active : state.stage_complete;
            if (complete) {
                outcome = "complete";
                break;
            }
            if (state.retry_menu) {
                outcome = "retry_menu";
                break;
            }
        }
        const auto ms =
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
        const double io_ms = double(session.file_io_time_ns() - initial_io_ns) / 1000000;
        if (state.frame < prefix_frame)
            throw std::runtime_error("execution ended before the requested prefix boundary");
        if (!replay_path.empty() && actions.size() != replay.size() && !allow_unused)
            throw std::runtime_error("action tape extends beyond the execution boundary");
        if (tail)
            tail->write(trace_path);
        if (!tape_path.empty()) {
            std::ofstream tape(tape_path);
            if (!tape)
                throw std::runtime_error("cannot open action tape output");
            for (auto action : actions)
                tape << action << '\n';
            if (!tape)
                throw std::runtime_error("cannot write action tape");
        }
        std::ofstream file;
        if (!output.empty())
            file.open(output);
        auto &out = output.empty() ? std::cout : file;
        if (!out)
            throw std::runtime_error("cannot open report");
        out << std::setprecision(std::numeric_limits<float>::max_digits10);
        out << "{\"source_revision\":\"861bec908b84fa4658382d7526e5a0075f520846\","
            << "\"profile\":\"native-headless-float32\",\"dat_sha256\":\"" << identity
            << "\",\"stage\":\"" << stage_name << "\",\"requested_spell_id\":" << config.spell
            << ",\"difficulty\":" << config.difficulty << ",\"seed\":" << config.seed
            << ",\"optimization\":" << TH08_HEADLESS_OPTIMIZATION << ",\"compiler\":\""
            << __VERSION__ << "\""
            << ",\"strategy\":\"" << (replay_path.empty() ? strategy : "replay")
            << "\",\"frame_budget\":" << limit << ",\"outcome\":\"" << outcome
            << "\",\"frames\":" << state.frame << ",\"spell_id\":" << state.spell
            << ",\"spell_start\":" << spell_start << ",\"first_hit\":" << first_hit
            << ",\"deaths\":" << state.deaths << ",\"player_state\":" << state.player_state
            << ",\"rng_draws\":" << state.rng_draws << ",\"graze\":" << state.graze
            << ",\"rng_seed\":" << state.rng_seed << ",\"score\":" << state.score
            << ",\"gauge\":" << state.gauge << ",\"peak_bullets\":" << peak
            << ",\"trace_digest\":\"" << digest
            << "\",\"simulation_ms\":" << std::max(0., update_ms - io_ms)
            << ",\"file_io_ms\":" << io_ms << ",\"decision_ms\":" << decision_ms
            << ",\"execution_ms\":" << ms << ",\"diagnostics_ms\":" << diagnostics_ms
            << ",\"policy_decisions\":" << policy_stats.decisions
            << ",\"policy_candidates\":" << policy_stats.candidates
            << ",\"policy_bullet_checks\":" << policy_stats.bullet_checks
            << ",\"policy_vector_acceleration_checks\":" << policy_stats.vector_acceleration_checks
            << ",\"policy_unsupported_transform_checks\":"
            << policy_stats.unsupported_transform_checks
            << ",\"policy_laser_paths\":" << policy_stats.laser_paths
            << ",\"policy_laser_paths_pruned\":" << policy_stats.laser_paths_pruned
            << ",\"policy_rigid_laser_paths\":" << policy_stats.rigid_laser_paths
            << ",\"policy_laser_checks\":" << policy_stats.laser_checks
            << ",\"policy_predicted_overlaps\":" << policy_stats.predicted_overlaps
            << ",\"policy_bullet_horizon\":12,\"policy_laser_horizon\":120"
            << ",\"policy_vector_acceleration_enabled\":"
            << (policy_stats.decisions != 0 && linear_profile_decisions == 0 ? "true" : "false")
            << ",\"policy_linear_profile_decisions\":" << linear_profile_decisions
            << ",\"policy_profile_last\":\"" << last_policy_profile << "\""
            << ",\"policy_direct_laser_decisions\":" << direct_laser_stats.decisions
            << ",\"policy_ecl_contexts\":" << direct_laser_stats.contexts
            << ",\"policy_direct_laser_calls\":" << direct_laser_stats.upcoming_calls
            << ",\"policy_direct_laser_warnings\":" << direct_laser_stats.usable_warnings
            << ",\"policy_direct_laser_variable_selectors\":"
            << direct_laser_stats.variable_selectors
            << ",\"policy_direct_laser_dynamic_geometry\":" << direct_laser_stats.dynamic_geometry
            << ",\"policy_direct_laser_candidate_overlaps\":"
            << direct_laser_stats.candidate_overlaps
            << ",\"policy_direct_laser_constrained_decisions\":"
            << direct_laser_stats.constrained_decisions
            << ",\"policy_direct_laser_allowed_candidates\":"
            << direct_laser_stats.allowed_candidates << ",\"previous_trace_digest\":\""
            << previous_digest << "\",\"prefix_frame\":" << prefix_frame
            << ",\"prefix_trace_digest\":\"" << prefix_digest << "\",\"unused_actions\":"
            << (replay_path.empty() ? 0 : replay.size() - actions.size())
            << ",\"diagnostics_enabled\":" << (tail ? "true" : "false");
        write_collision(out, session.collision());
        out << "}\n";
        if (!out)
            throw std::runtime_error("cannot write report");
        return complete ? 0 : 2;
    } catch (const std::exception &e) {
        std::cerr << "th08_headless: " << e.what() << '\n';
        return 1;
    }
}
