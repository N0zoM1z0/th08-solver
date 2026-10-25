#include "modern/headless/session.hpp"
#include "th08/online_native.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/shadow_bridge.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_plan.hpp"
#include "th08/shadow_proposal.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
namespace s = th08::shadow;
void check(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
std::string contents(const char *path) {
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
int story_entry(const char *dat_path, const char *tape_path, bool sampled) {
    const auto dat = std::filesystem::absolute(dat_path).string();
    std::ifstream file(tape_path);
    th08::plan::PrefixParser parser(1000000);
    char bytes[8192];
    while (file) {
        file.read(bytes, sizeof(bytes));
        parser.append(bytes, std::size_t(file.gcount()));
    }
    check(file.eof(), "cannot read story entry tape");
    const auto actions = parser.finish();
    const auto previous = std::filesystem::current_path();
    char scratch[] = "/tmp/th08-shadow-entry-test-XXXXXX";
    check(mkdtemp(scratch) != nullptr, "entry scratch failed");
    std::filesystem::current_path(scratch);
    th08::headless::Config config;
    config.dat_path = dat.c_str();
    config.story = true;
    config.shot = 1;
    config.difficulty = 3;
    config.input_profile = sampled ? th08::headless::InputProfile::SampledGameplay
                                   : th08::headless::InputProfile::Controller;
    th08::headless::Session session(config);
    bool reached = false;
    unsigned calls = 0;
    for (auto input : actions) {
        if (calls >= 32000)
            break;
        const auto before = session.state();
        const auto after = session.step(input);
        ++calls;
        if (after.stage != before.stage) {
            check(after.stage == 1 && after.calculation_updates == before.calculation_updates,
                  "story control update was collapsed into a gameplay calculation");
            const auto first = session.step(0x45);
            ++calls;
            check(first.calculation_updates == after.calculation_updates + 1 &&
                      first.sampled_input == (sampled ? 0x45 : 0) &&
                      first.latched_input == (sampled ? 0x45 : 0),
                  "new-stage sampled input profile changed or lost the original latch");
            reached = true;
            break;
        }
    }
    check(reached && session.state().deaths == 0, "complete first-stage entry gate failed");
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(scratch);
    std::cout << "story entry profile=" << (sampled ? "sampled" : "controller")
              << " updates=" << calls << '\n';
    return 0;
}
int retained_4b(const char *dat_path, const char *trace_path) {
    const auto dat = std::filesystem::absolute(dat_path).string();
    std::ifstream file(trace_path, std::ios::binary);
    std::vector<s::Packet> tape;
    s::bridge::Trace bytes;
    while (file.read(reinterpret_cast<char *>(bytes.data()), bytes.size()))
        tape.push_back(s::decode(bytes.data(), s::Bytes{}.size()));
    check(file.eof() && !file.gcount() && tape.size() == 34318,
          "expected the retained seed42386 Stage4B death journal");
    const auto previous = std::filesystem::current_path();
    char scratch[] = "/tmp/th08-held-4b-test-XXXXXX";
    check(mkdtemp(scratch) != nullptr, "held regression scratch failed");
    std::filesystem::current_path(scratch);
    th08::headless::Config config;
    config.dat_path = dat.c_str();
    config.stage = 4;
    config.difficulty = 3;
    config.shot = 0;
    config.seed = 42386;
    config.input_profile = th08::headless::InputProfile::SampledGameplay;
    th08::headless::Session session(config);
    th08::online::Configuration settings;
    settings.horizon = 24;
    settings.beam = 192;
    settings.diversity_cell = 16;
    settings.spawn_guard = 16;
    settings.reach_filter = settings.hard_phantoms = settings.motion_envelope = true;
    th08::online::Policy policy(settings);
    th08::online::Observation observation;
    th08::online::capture_memory(session, 0, observation, true);
    policy.observe(observation);
    s::Packet current;
    current.state = s::capture(session);
    for (unsigned i = 0; i < 34256; ++i) {
        session.step(std::uint16_t(tape[i].input));
        s::advance_receipt(current, tape[i].input, s::capture(session));
        check(!s::difference(current, tape[i]), "retained prefix changed execution or RNG");
        th08::online::capture_memory(session, current.sequence, observation, true);
        policy.observe(observation);
    }
    std::vector<std::uint16_t> prefix;
    for (unsigned i = 34256; i < 34264; ++i)
        prefix.push_back(std::uint16_t(tape[i].input));
    while (prefix.size() < 32)
        prefix.push_back(s::bridge::fallback(0x45, current.sequence + prefix.size() + 1, false));
    const auto guided =
        s::predict_with_policy(session, policy, current, prefix, 64, {false, -1}, 1000);
    const auto held = s::predict_with_hold(session, current, prefix, 64, {false, -1}, 1000);
    check(guided.status == s::BranchStatus::Death && !guided.certifies(),
          "retained policy-tail counterexample no longer reproduces");
    check(held.status == s::BranchStatus::Horizon && held.completed == 64 && held.certifies(),
          "held tail did not recover a complete native horizon");
    for (unsigned i = 0; i < prefix.size(); ++i)
        check(held.frames[i].input == prefix[i], "held recovery rewrote the immutable prefix");
    s::Packet after = current;
    after.state = s::capture(session);
    check(!s::difference(after, current), "held recovery mutated the actual world");
    std::cout << "retained4b origin=34256 guided_death=" << guided.completed
              << " held_complete=" << held.completed << " guided_ms=" << guided.wall_ms
              << " held_ms=" << held.wall_ms
              << " charged=" << guided.charged_updates + held.charged_updates << '\n';
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(scratch);
    return 0;
}
int main(int argc, char **argv) try {
    if (argc == 4) {
        if (std::string(argv[3]) == "retained4b")
            return retained_4b(argv[1], argv[2]);
        check(std::string(argv[3]) == "sampled" || std::string(argv[3]) == "controller",
              "unknown input profile");
        return story_entry(argv[1], argv[2], std::string(argv[3]) == "sampled");
    }
    check(argc == 3, "expected DAT and complete control tape");
    const auto dat = std::filesystem::absolute(argv[1]).string();
    std::ifstream file(argv[2]);
    th08::plan::PrefixParser parser(2048);
    char buffer[8192];
    while (file) {
        file.read(buffer, sizeof(buffer));
        parser.append(buffer, std::size_t(file.gcount()));
    }
    check(file.eof(), "cannot read control tape");
    const auto actions = parser.finish();
    const auto previous = std::filesystem::current_path();
    char scratch[] = "/tmp/th08-shadow-rollout-test-XXXXXX";
    check(mkdtemp(scratch) != nullptr, "scratch failed");
    std::filesystem::current_path(scratch);
    th08::headless::Config config;
    config.dat_path = dat.c_str();
    config.stage = 7;
    config.spell = 179;
    th08::headless::Session session(config);
    const auto score_before = contents("score.dat");
    const auto config_before = contents("th08.cfg");
    s::Packet origin;
    origin.state = s::capture(session);
    const auto good = s::predict(session, origin, actions, {false, 179}, 10000);
    check(good.certifies() && good.status == s::BranchStatus::Terminal &&
              good.frames.size() == 1292,
          "complete native future was not verified");
    const std::vector<std::uint16_t> hold_prefix(actions.begin(), actions.begin() + 8);
    auto literal_hold = hold_prefix;
    literal_hold.resize(64, std::uint16_t(hold_prefix.back() & 0xf5));
    const auto held = s::predict_with_hold(session, origin, hold_prefix, 64, {false, 179}, 10000);
    const auto literal = s::predict(session, origin, literal_hold, {false, 179}, 10000);
    check(held.status == literal.status && held.frames.size() == literal.frames.size() &&
              held.frames.size() == 64 && held.certifies(),
          "held continuation did not require the complete native horizon");
    for (unsigned i = 0; i < held.frames.size(); ++i) {
        if (const auto field = s::difference(held.frames[i], literal.frames[i]))
            std::cerr << "held mismatch at=" << i << " field=" << field
                      << " held_input=" << held.frames[i].input
                      << " literal_input=" << literal.frames[i].input << '\n';
        check(!s::difference(held.frames[i], literal.frames[i]),
              "held continuation changed the immutable prefix or its literal native execution");
    }
    s::Packet actual = origin;
    actual.state = s::capture(session);
    check(!s::difference(origin, actual), "successful child mutated the parent world");
    const auto bad =
        s::predict(session, origin, std::vector<std::uint16_t>(1292, 5), {false, 179}, 10000);
    check(!bad.certifies() && bad.status == s::BranchStatus::Death && bad.completed < 1292,
          "fatal native future certified a prefix");
    actual.state = s::capture(session);
    check(!s::difference(origin, actual), "failed child mutated parent world or RNG");
    check(good.charged_updates == 1292 && bad.charged_updates == bad.completed,
          "native branch cost omitted successful or failed updates");
    class TapeGuidance final : public s::RolloutGuidance {
        const std::vector<std::uint16_t> &actions_;

      public:
        std::size_t position = 8, observations = 0;
        explicit TapeGuidance(const std::vector<std::uint16_t> &actions) : actions_(actions) {}
        void observe(const s::Packet &packet) override {
            check(packet.sequence == observations++, "guided rollout observed a stale world");
        }
        std::uint16_t next_input() override {
            return actions_.at(position++);
        }
    } guidance(actions);
    const auto guided = s::predict_guided(
        session, origin, std::vector<std::uint16_t>(actions.begin(), actions.begin() + 8),
        actions.size(), guidance, {false, 179}, 10000);
    check(guided.certifies() && guided.guidance_requests == actions.size() - 8 &&
              guided.frames.size() == good.frames.size() && guidance.position == 8 &&
              guidance.observations == 0,
          "guided continuation leaked state or failed the complete native horizon");
    for (unsigned i = 0; i < good.frames.size(); ++i)
        check(!s::difference(guided.frames[i], good.frames[i]),
              "guided continuation changed input/latch/world execution");
    check(contents("score.dat") == score_before && contents("th08.cfg") == config_before,
          "native branch changed the owner's score/config files");
    s::ActionBuffer queue(origin);
    check(queue.replace(good), "verified future did not enter action buffer");
    th08::online::Configuration profile;
    profile.horizon = 24;
    th08::online::Policy policy(profile);
    th08::online::Observation observation;
    th08::online::capture_memory(session, 0, observation, true);
    policy.observe(observation);
    std::vector<std::uint16_t> lead(actions.begin(), actions.begin() + 8);
    const auto future = s::propose_after_lead(session, policy, origin, lead, {false, 179}, 1000);
    check(future.status == s::BranchStatus::Horizon && !future.inputs.empty() &&
              future.attempts == 8 && future.completed == 8 && future.charged_updates == 8,
          "proposal work did not own and charge its forced lead");
    actual.state = s::capture(session);
    check(!s::difference(actual, origin) && contents("score.dat") == score_before &&
              contents("th08.cfg") == config_before,
          "future proposal mutated the actual owner");
    for (const auto &frame : good.frames) {
        check(queue.next() && queue.next()->input == frame.input, "buffer changed native inputs");
        session.step(std::uint16_t(frame.input));
        s::advance_receipt(actual, frame.input, s::capture(session));
        check(queue.confirm(actual), "committed execution diverged from verified branch");
        if (frame.sequence <= 8) {
            th08::online::capture_memory(session, frame.sequence, observation, true);
            policy.observe(observation);
            if (frame.sequence == 8) {
                std::vector<th08::online::ValueProposal> serial;
                policy.decide(observation, nullptr, &serial);
                check(serial.size() == future.inputs.size(), "proposal history leaked from child");
                for (unsigned i = 0; i < serial.size(); ++i)
                    check(serial[i].inputs == future.inputs[i],
                          "proposal did not use the true end-of-lead observation and latch");
            }
        }
    }
    check(!queue.next() && session.state().deaths == 0, "complete buffered execution failed");
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(scratch);
    std::cout << "complete true-RNG native branch=" << good.completed
              << " failed branch=" << bad.completed
              << " native_ms=" << double(good.native_ns + bad.native_ns) / 1e6 << '\n';
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
