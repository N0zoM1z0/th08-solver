#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "ReplayManager.hpp"
#include "modern/headless/session.hpp"
#include "th08/plan_contracts.hpp"
#include "th08/plan_route.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <unistd.h>

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void choice(const char *dat, int shot, th08::plan::FinalRoute route) {
    using namespace th08;
    headless::Config config;
    config.dat_path = dat;
    config.stage = STAGE5;
    config.story = true;
    config.shot = shot;
    config.unlock_final_routes = true;
    headless::Session session(config);
    require(g_ReplayManager->replayData->clearState == 2 &&
                !g_ReplayManager->replayData->isPractice,
            "explicit route history was not recorded");
    // Isolate the real DAT's post-boss choice interpreter. This is a controller
    // regression, not a gameplay clear or a replacement for full-run validation.
    g_Gui.MsgRead(10);
    bool saw_choice = false;
    std::uint16_t preceding = 0;
    for (int frame = 1; frame <= 1200; ++frame) {
        const auto state = session.state();
        saw_choice = saw_choice || state.final_route_selection;
        const auto input =
            plan::final_route_input(route, state, frame).value_or(std::uint16_t(frame % 2 ? 0 : 1));
        g_GuiMessageInputPrevious = preceding;
        g_GuiMessageInputCurrent = input;
        preceding = input;
        g_Gui.impl->RunMsg();
        if (saw_choice && g_Gui.impl->message.currentMsgIdx != 3) {
            const int expected = route == plan::FinalRoute::A ? 0 : 1;
            require(g_GameManager.flags.finalStageRoute == unsigned(expected),
                    "original dialogue selected the wrong route");
            g_GameManager.AdvanceToNextStage();
            require(g_GameManager.currentStage == (expected ? STAGE6B : STAGE6A),
                    "original transition selected the wrong final stage");
            return;
        }
    }
    throw std::runtime_error("original route selection did not finish");
}

void extra(const char *dat) {
    using namespace th08;
    headless::Config config;
    config.dat_path = dat;
    config.stage = EXTRASTAGE;
    config.difficulty = EXTRA;
    config.story = true;
    headless::Session session(config);
    require(!g_GameManager.IsPracticeMode() && !g_ReplayManager->replayData->isPractice &&
                session.state().lives == 2 && g_GameManager.GetClockTime() == 6,
            "full Extra silently used practice initialization");
    const auto state = session.step(5);
    require(state.calculation_updates == 1 && state.stage == EXTRASTAGE,
            "full Extra did not execute its original calculation chain");
}
} // namespace

int main(int argc, char **argv) {
    if (argc != 2)
        return 2;
    const auto dat = std::filesystem::absolute(argv[1]).string();
    char scratch[] = "/tmp/th08-routes-XXXXXX";
    if (!mkdtemp(scratch))
        return 2;
    bool ok = true;
    for (int index = 0; index < 5; ++index) {
        const auto directory = std::filesystem::path(scratch) / std::to_string(index);
        std::filesystem::create_directory(directory);
        const pid_t pid = fork();
        if (pid == 0) {
            try {
                std::filesystem::current_path(directory);
                if (index == 4)
                    extra(dat.c_str());
                else
                    choice(dat.c_str(), index < 2 ? 0 : 11,
                           index % 2 ? th08::plan::FinalRoute::B : th08::plan::FinalRoute::A);
                _exit(0);
            } catch (const std::exception &error) {
                std::fprintf(stderr, "native route contract %d: %s\n", index, error.what());
                _exit(1);
            }
        }
        if (!th08::plan::wait_for_child(pid).exited())
            ok = false;
    }
    std::filesystem::remove_all(scratch);
    return ok ? 0 : 1;
}
