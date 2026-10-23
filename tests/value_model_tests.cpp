#include "test_support.hpp"
#include "th08/online_value.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>

namespace fs = std::filesystem;
namespace on = th08::online;
using th08::test::check;
namespace {
template <class F> void rejects(F action, const char *message) {
    bool failed = false;
    try {
        action();
    } catch (const std::exception &) {
        failed = true;
    }
    check(failed, message);
}
void write(const fs::path &path, const std::string &bytes) {
    std::ofstream output(path);
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output << bytes;
}
std::string row(const std::string &status, unsigned updates, unsigned index = 0) {
    std::ostringstream output;
    output << std::string(64, 'a') << "\t42\t5\t3\t0\t107\t128\t" << index << '\t' << status << '\t'
           << updates << '\t' << std::string(64, 'b');
    for (unsigned i = 0; i < on::kValueFeatures; ++i)
        output << '\t' << (i == 0 ? 1 : 0);
    return output.str() + '\n';
}
} // namespace
int main(int argc, char **argv) {
    try {
        check(argc == 2, "value test needs scratch");
        const fs::path root(argv[1]);
        fs::remove_all(root);
        fs::create_directories(root);
        on::BranchValueModel model;
        model.loaded = true;
        model.scale.fill(1);
        model.weight[0] = 1;
        on::ValueFeatures features{};
        check(model.probability(features) == .5, "zero-logit probability changed");
        features[0] = 1;
        check(std::fabs(model.probability(features) - .7310585786300049) < 1e-14,
              "logistic value normalization or feature order changed");
        model.bias = 1000;
        check(model.probability(features) == 1, "positive logit overflowed");
        model.bias = -1000;
        check(model.probability(features) == 0, "negative logit overflowed");
        model.bias = -.125;
        model.mean[0] = .125;
        model.scale[0] = .25;
        on::save_branch_value(model, (root / "weights.tsv").string());
        const auto loaded = on::load_branch_value((root / "weights.tsv").string());
        check(loaded.probability(features) == model.probability(features),
              "saved model changed prediction");
        rejects([&] { on::save_branch_value(model, (root / "weights.tsv").string()); },
                "saved model snapshot was overwritten");
        features[0] = std::numeric_limits<float>::quiet_NaN();
        rejects([&] { loaded.probability(features); }, "non-finite features reached a score");
        model.scale[0] = 0;
        rejects([&] { model.probability({}); }, "zero feature scale accepted");
        write(root / "truncated.tsv",
              "th08_branch_value_v1\t24\t64\t24\nfeature\tmean\tscale\tweight\n");
        rejects([&] { on::load_branch_value((root / "truncated.tsv").string()); },
                "truncated model accepted");
        const std::string header =
            "th08_branch_dataset_v1\t24\t64\t24\n" + on::branch_dataset_header() + '\n';
        write(root / "data.tsv",
              header + row("survived", 64) + row("death", 12, 1) + row("complete", 23, 2));
        const auto data = on::read_branch_examples((root / "data.tsv").string());
        check(data.size() == 3 && data[0].survived && !data[1].survived && data[2].survived,
              "native label/terminal meaning changed");
        for (const auto *status : {"fault", "budget", "unsupported"}) {
            write(root / "bad.tsv", header + row(status, 12));
            rejects([&] { on::read_branch_examples((root / "bad.tsv").string()); },
                    "failed or incomplete native branch became a label");
        }
        write(root / "bad.tsv", header + row("survived", 63));
        rejects([&] { on::read_branch_examples((root / "bad.tsv").string()); },
                "short surviving branch became a safe label");
        write(root / "bad.tsv", header + row("survived", 64) + row("death", 12));
        rejects([&] { on::read_branch_examples((root / "bad.tsv").string()); },
                "duplicate branch label accepted");
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
