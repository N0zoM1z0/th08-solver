#include "test_support.hpp"
#include <cstdint>
#include <th08/direct_laser_policy.hpp>
#include <vector>

// The online proposal adapter converts the current ECL instruction of each
// context into bounded direct-laser warnings. These cases pin the decoded
// timing window and the refusals for variable selectors and moving geometry.
namespace {
struct EclContext {
    int enemy_index, time, next_time, next_opcode;
    int secondary_time, pending_subroutine, active_interpolations, per_frame_ex;
    std::uint16_t operand_flags;
    std::uint32_t enemy_flags;
    bool difficulty_enabled, has_raw_int0, has_parent;
    int raw_int0;
    float position_x, position_y, position_offset_x, position_offset_y;
    float velocity_x, velocity_y, rotation, rotation_velocity;
    float variable0, variable1;
};
} // namespace

int main() {
    using th08::test::check;
    using Warnings = std::vector<th08::policy::DirectLaserWarning>;
    const EclContext direct_context{2,   110, 120, 137,  0,    -1,    0,
                                    -1,  0,   0,   true, true, false, 9,
                                    262, 384, 0,   0,    0,    0,     1.57079632679489661923f,
                                    0,   0,   295};
    Warnings warnings;
    th08::policy::DirectLaserStats direct_stats;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{direct_context}, 120,
                                                warnings, direct_stats);
    check(warnings.size() == 1 && warnings[0].first_update == 11 &&
              warnings[0].last_update == 120 && warnings[0].full_height == 160,
          "constant ECL opcode 137 warning was not decoded at its native boundary");

    auto disabling_context = direct_context;
    disabling_context.per_frame_ex = 9;
    disabling_context.raw_int0 = -1;
    th08::policy::DirectLaserStats disable_stats;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{disabling_context}, 120,
                                                warnings, disable_stats);
    check(warnings.size() == 1 && warnings[0].first_update == 1 && warnings[0].last_update == 10,
          "repeating ECL laser warning crossed its source disable update");

    auto variable_context = direct_context;
    variable_context.operand_flags = 1;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{variable_context}, 120,
                                                warnings, direct_stats);
    check(warnings.empty() && direct_stats.variable_selectors == 1,
          "variable ECL selector was silently treated as a constant future");

    auto moving_context = direct_context;
    moving_context.velocity_x = 1;
    th08::policy::collect_direct_laser_warnings(std::vector<EclContext>{moving_context}, 120,
                                                warnings, direct_stats);
    check(warnings.empty() && direct_stats.dynamic_geometry == 1,
          "moving direct-laser geometry was projected without an owned motion model");
    return 0;
}
