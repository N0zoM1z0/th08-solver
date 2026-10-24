#include "modern/headless/session.hpp"
#include "th08/plan_prefix.hpp"
#include "th08/shadow_native.hpp"
#include "th08/shadow_plan.hpp"
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
int main(int argc, char **argv) try {
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
    check(contents("score.dat") == score_before && contents("th08.cfg") == config_before,
          "native branch changed the owner's score/config files");
    s::ActionBuffer queue(origin);
    check(queue.replace(good), "verified future did not enter action buffer");
    for (const auto &frame : good.frames) {
        check(queue.next() && queue.next()->input == frame.input, "buffer changed native inputs");
        session.step(std::uint16_t(frame.input));
        s::advance_receipt(actual, frame.input, s::capture(session));
        check(queue.confirm(actual), "committed execution diverged from verified branch");
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
