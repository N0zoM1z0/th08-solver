#include <cstdlib>
#include <iostream>
#include <random>
#include <th08/geometry.hpp>

using namespace th08::geometry;
void require(bool yes, const char *message) {
    if (!yes) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
template <class F> void rejects(F f) {
    try {
        f();
    } catch (const std::invalid_argument &) {
        return;
    }
    require(false, "invalid geometry was accepted");
}
int main() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    rejects([&] { Snapshot s({}, {nan, 1}); });
    rejects([] { Snapshot s({}, {-1, 1}); });
    Snapshot empty({}, {1, 1});
    rejects([&] { empty.query({nan, 0}); });
    rejects([&] { empty.scan({0, nan}); });
    rejects([] { Snapshot s({Hazard::bullet({{}, {-1, 2}})}, {1, 1}); });
    rejects([&] { Snapshot s({Hazard::laser({{{}, {1, 1}}, {}, nan})}, {1, 1}); });
    require(!empty.query({192, 224}), "empty world collision");
    require(box_hit({2, 0}, {1, 1}, {{0, 0}, {2, 2}}), "touch must collide");
    require(!box_hit({std::nextafter(2.f, 3.f), 0}, {1, 1}, {{0, 0}, {2, 2}}),
            "outside touch must miss");
    std::vector<Hazard> original{Hazard::bullet({{32, 32}, {2, 2}})};
    Snapshot owned(original, {1, 1});
    original[0].box.center = {300, 300};
    require(owned.query({32, 32}) && !owned.query({300, 300}), "caller mutation changed snapshot");
    Snapshot copy = owned;
    Snapshot moved = std::move(copy);
    require(moved.query({32, 32}), "snapshot move invalidated index");
    require(laser_hit({0, 4}, {3, .25f}, {{{4, 0}, {2, 2}}, {}, 1.5707963705062866f}),
            "laser rotation contract");
    std::mt19937 rng(20260912);
    auto u = [&](float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); };
    std::uint64_t queries = 0;
    for (int world = 0; world < 120; ++world) {
        std::vector<Hazard> hazards;
        Vec2 half{u(.1f, 3), u(.1f, 3)};
        for (int i = 0; i < 256; ++i) {
            Box b{{u(-100, 484), u(-100, 548)}, {u(0, 20), u(0, 20)}};
            if (i % 13 == 0)
                hazards.push_back(
                    Hazard::laser({b, {u(-500, 500), u(-500, 500)}, u(-3.15f, 3.15f)}));
            else
                hazards.push_back(Hazard::bullet(b));
        }
        Snapshot s(hazards, half);
        for (int i = 0; i < 2000; ++i) {
            Vec2 p{u(-50, 434), u(-50, 498)};
            require(s.query(p) == s.scan(p), "grid disagrees with brute force");
            ++queries;
        }
        // Cell edges and exact contact are underrepresented by random points.
        for (int x = 0; x <= 384; x += 16)
            for (int y = 0; y <= 448; y += 16) {
                require(s.query({float(x), float(y)}) == s.scan({float(x), float(y)}),
                        "cell boundary mismatch");
                ++queries;
            }
        for (const auto &h : hazards)
            if (h.kind == Hazard::Kind::bullet) {
                Vec2 p{h.box.center.x + h.box.size.x / 2 + half.x, h.box.center.y};
                require(s.query(p) == s.scan(p), "contact boundary mismatch");
                ++queries;
            }
    }
    std::cout << "{\"differential_queries\":" << queries << ",\"mismatches\":0}\n";
}
