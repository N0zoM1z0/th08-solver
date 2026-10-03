#include "../tools/headless_report.hpp"
// Deterministic process-protocol fixture. This is not native game evidence.
#include <csignal>
#include <fstream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>
int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--parser-test") {
        using th08::headless_report::Document;
        for (const std::string text : {"{\"x\":[1 ]}", "{\"x\":-1.25e+2,\"y\":{\"z\":true}}"})
            Document(text).parse();
        for (const std::string text :
             {"{\"x\":- 1}", "{\"x\":1}trailing", "{\"x\":1,\"x\":2}", "{\"x\":01}", "{\"x\":1.}",
              "{\"x\":[1,]}", "{\"x\":1e}", "{\v\"x\":1}"}) {
            bool rejected = false;
            try {
                Document(text).parse();
            } catch (const std::exception &) {
                rejected = true;
            }
            if (!rejected)
                return 1;
        }
        return 0;
    }
    std::map<std::string, std::string> args;
    for (int i = 1; i + 1 < argc; i += 2)
        args[argv[i]] = argv[i + 1];
    const unsigned seed = std::stoul(args["--seed"]), cap = std::stoul(args["--frames"]);
    const bool resume = args.count("--resume-prefix"), replay = args.count("--replay");
    if ((seed == 3 && resume) || (seed == 8 && !resume && !replay) || (seed == 9 && replay)) {
        signal(SIGTERM, SIG_IGN);
        for (;;)
            pause();
    }
    std::vector<unsigned> actions;
    const auto path = resume ? args["--resume-prefix"] : replay ? args["--replay"] : "";
    if (!path.empty()) {
        std::ifstream f(path);
        unsigned a;
        while (f >> a)
            actions.push_back(a);
    }
    const unsigned frames = (resume || replay || seed == 5) ? 80 : 64;
    const bool complete = frames == 80;
    actions.resize(frames, 4);
    if (seed == 6 && resume)
        actions.at(15) = 4;
    std::ofstream tape(args["--actions"]);
    for (auto a : actions)
        tape << a << '\n';
    if (args.count("--policy-log")) {
        std::ofstream log(args["--policy-log"]);
        log << "frame\toverlap\thorizon\ttrace_digest\n";
        for (unsigned f = 1; f <= frames; ++f) {
            const unsigned horizon = seed == 10 ? 12 : seed == 11 ? 20 :
                                     seed == 12 && f >= 32 ? 12 : 32;
            log << f << '\t' << (f < 32 ? horizon + 1 : 1) << '\t' << horizon << '\t' << f << '\n';
        }
    }
    const unsigned prefix = args.count("--prefix-frame") ? std::stoul(args["--prefix-frame"]) : 0;
    std::ofstream out(args["--output"]);
    out << "{\"source_revision\":\"fixture\",\"profile\":\"mock-process-only\",\"dat_sha256\":"
           "\"fixture\",\"stage\":\"extra\",\"requested_spell_id\":"
        << (seed == 7 && resume ? 202 : 203) << ",\"difficulty\":4,\"seed\":" << seed
        << ",\"frame_budget\":" << cap << ",\"outcome\":\"" << (complete ? "complete" : "collision")
        << "\",\"frames\":" << frames
        << ",\"spell_id\":203,\"spell_start\":1,\"first_hit\":" << (complete ? 0 : frames)
        << ",\"deaths\":0,\"player_state\":" << (complete ? 3 : 2)
        << ",\"rng_draws\":0,\"rng_seed\":0,\"graze\":0,\"score\":0,\"gauge\":0,\"peak_bullets\":0,"
           "\"trace_digest\":\""
        << frames << "\",\"previous_trace_digest\":\"" << frames - 1
        << "\",\"prefix_frame\":" << prefix << ",\"prefix_trace_digest\":\""
        << (seed == 1 && resume ? 999 : prefix)
        << "\",\"unused_actions\":0,\"collision\":{\"kind\":\"" << (complete ? "none" : "bullet")
        << "\"}}\n";
    if (seed == 4)
        out << "trailing malformed content";
    return seed == 2 ? 0 : complete ? 0 : 2;
}
