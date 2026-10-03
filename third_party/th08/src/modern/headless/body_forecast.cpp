#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#include "body_math.hpp"
#include "session.hpp"
#include <algorithm>
#include <cfenv>
#include <climits>
#include <cstring>
#include <set>

namespace th08::headless {
namespace {
using body_detail::Interval;
struct Subprogram {
    const EclRawInstruction *begin = nullptr;
    std::vector<const EclRawInstruction *> instructions;
};
int integer(const EclRawInstruction &instruction, unsigned operand) {
    int value;
    std::memcpy(&value, instruction.operands + operand * 4, 4);
    return value;
}
float scalar(const EclRawInstruction &instruction, unsigned operand) {
    float value;
    std::memcpy(&value, instruction.operands + operand * 4, 4);
    return value;
}
bool int_variable(int selector) {
    return (selector >= ECL_OPERAND_LOCAL_INT_0 && selector <= ECL_OPERAND_ENEMY_INT_7) ||
           (selector >= ECL_OPERAND_EXTRA_INT_0 && selector <= ECL_OPERAND_EXTRA_INT_3);
}
bool float_variable(float selector, bool extra = false) {
    if (!std::isfinite(selector) || std::trunc(selector) != selector)
        return false;
    return (selector >= ECL_OPERAND_LOCAL_FLOAT_0 && selector <= ECL_OPERAND_LOCAL_FLOAT_7) ||
           (extra &&
            (selector == ECL_OPERAND_EXTRA_FLOAT_0 || selector == ECL_OPERAND_EXTRA_FLOAT_1));
}
bool load_subprogram(int id, Subprogram &sub) {
    if (!g_EclManager.eclFile || !g_EclManager.subTable || id < 0 ||
        id >= g_EclManager.eclFile->subCount || g_EclManager.eclFile->subCount > 2048)
        return false;
    const auto begin = g_EclManager.subTable[id];
    auto end = std::numeric_limits<uintptr_t>::max();
    for (int i = 0; i < g_EclManager.eclFile->subCount; ++i)
        if (g_EclManager.subTable[i] > begin)
            end = std::min(end, g_EclManager.subTable[i]);
    // A following loaded subroutine supplies a trusted allocation bound. The
    // terminal subroutine is deliberately outside this small adapter's scope.
    if (!begin || end == std::numeric_limits<uintptr_t>::max())
        return false;
    sub.begin = reinterpret_cast<const EclRawInstruction *>(begin);
    auto cursor = begin;
    for (unsigned count = 0; count < 512 && cursor <= end - 12; ++count) {
        const auto *instruction = reinterpret_cast<const EclRawInstruction *>(cursor);
        if (instruction->nextOffset < 12 || instruction->nextOffset > 1024 ||
            instruction->nextOffset % 4 || cursor + instruction->nextOffset > end)
            return false;
        sub.instructions.push_back(instruction);
        if (instruction->time < 0 && instruction->opcode == -1)
            return true;
        cursor += instruction->nextOffset;
    }
    return false;
}
const EclRawInstruction *jump_target(const Subprogram &sub, const EclRawInstruction &instruction,
                                     int offset) {
    const auto target = std::int64_t(reinterpret_cast<uintptr_t>(&instruction)) + offset;
    for (const auto *candidate : sub.instructions)
        if (std::int64_t(reinterpret_cast<uintptr_t>(candidate)) == target)
            return candidate;
    return nullptr;
}
// Certify the entire local program, not a sampled RNG path. These invisible
// emitters may move/shoot/write their own variables, but cannot mutate another
// owner, enable a body, install an EX callback, or jump outside audited code.
bool passive_subgraph(int id, bool newly_spawned, std::set<int> &visited) {
    if (visited.size() >= 16)
        return false;
    if (!visited.insert(id).second)
        return true;
    Subprogram sub;
    if (!load_subprogram(id, sub))
        return false;
    if (newly_spawned) {
        const auto &first = *sub.begin;
        if (first.time != 0 || first.opcode != ECL_OPCODE_DISABLE_INTERACTION_FLAGS ||
            first.nextOffset != 16 || first.operandFlags || first.difficultyMask != 255 ||
            !(integer(first, 0) & ECL_INTERACTION_NO_SPRITE))
            return false;
    }
    for (const auto *p : sub.instructions) {
        const auto &i = *p;
        if (i.time < 0 && i.opcode == -1)
            break;
        const auto flags = i.operandFlags;
        switch (i.opcode) {
        case ECL_OPCODE_TERMINATE:
        case ECL_OPCODE_RETURN:
            if (i.nextOffset != 12 || flags)
                return false;
            break;
        case ECL_OPCODE_DISABLE_INTERACTION_FLAGS:
            if (i.nextOffset != 16 || flags)
                return false;
            break;
        case ECL_OPCODE_SET_INT:
        case ECL_OPCODE_INT_ADD_ASSIGN:
            if (i.nextOffset != 20 || !(flags & 1) || (flags & ~3u) || !int_variable(integer(i, 0)))
                return false;
            break;
        case ECL_OPCODE_SET_FLOAT:
        case ECL_OPCODE_FLOAT_ADD_ASSIGN:
        case ECL_OPCODE_FLOAT_DIVIDE_ASSIGN:
            if (i.nextOffset != 20 || !(flags & 1) || (flags & ~3u) ||
                !float_variable(scalar(i, 0), true))
                return false;
            break;
        case ECL_OPCODE_FLOAT_ADD:
            if (i.nextOffset != 24 || !(flags & 1) || (flags & ~7u) ||
                !float_variable(scalar(i, 0), true))
                return false;
            break;
        case ECL_OPCODE_NORMALIZE_ANGLE:
            if (i.nextOffset != 16 || flags != 1 || !float_variable(scalar(i, 0), true))
                return false;
            break;
        case ECL_OPCODE_JUMP:
            if (i.nextOffset != 20 || flags || !jump_target(sub, i, integer(i, 1)))
                return false;
            break;
        case ECL_OPCODE_JUMP_DEC:
            if (i.nextOffset != 24 || flags != 4 || !int_variable(integer(i, 2)) ||
                !jump_target(sub, i, integer(i, 1)))
                return false;
            break;
        case ECL_OPCODE_JUMP_IF_INT_LESS_EQUAL:
            if (i.nextOffset != 28 || (flags & ~3u) || !jump_target(sub, i, integer(i, 3)))
                return false;
            break;
        case ECL_OPCODE_ORBIT_AROUND_POINT:
            if (i.nextOffset != 40 || (flags & ~127u))
                return false;
            break;
        case ECL_OPCODE_SET_CHILD_ECL:
            if (i.nextOffset != 20 || flags || integer(i, 0) < 0 || integer(i, 0) >= 4 ||
                !passive_subgraph(integer(i, 1), false, visited))
                return false;
            break;
        case ECL_OPCODE_SET_BULLET_TRANSFORM:
            if (i.nextOffset != 40 || (flags & ~127u) || (flags & 7u) || integer(i, 0) < 0 ||
                integer(i, 0) >= 18)
                return false;
            break;
        case ECL_OPCODE_SHOOT_FAN:
            if (i.nextOffset != 44 || (flags & ~255u))
                return false;
            break;
        case ECL_OPCODE_SPAWN_EFFECT:
            if (i.nextOffset != 24 || flags || integer(i, 0) != 63)
                return false;
            break;
        default:
            return false;
        }
    }
    return true;
}
bool passive_owner(const Enemy &enemy) {
    std::set<int> visited;
    auto context_safe = [&](const EnemyEclContext &context) {
        Subprogram sub;
        if (context.perFrameCallback || !load_subprogram(context.subId, sub) ||
            std::find(sub.instructions.begin(), sub.instructions.end(), context.currentInstr) ==
                sub.instructions.end())
            return false;
        for (const auto &slot : context.interpolationSlots)
            if (slot.callback)
                return false;
        return passive_subgraph(context.subId, false, visited);
    };
    if (enemy.pendingEclSubroutineIndex >= 0 || enemy.mainEclCallStackDepth != 0 ||
        enemy.deathCallbackSubId >= 0 || enemy.timerCallbackThresholdFrames >= 0 ||
        !context_safe(enemy.mainEclContextStorage))
        return false;
    for (int threshold : enemy.lifeCallbackThresholds)
        if (threshold >= 0)
            return false;
    for (const auto *child : enemy.childEclBlocks)
        if (child && (child->callStackDepth != 0 || !context_safe(child->eclContext)))
            return false;
    return true;
}
bool end_spell_prefix(const Enemy &owner, int sub_id, const EclRawInstruction *cursor,
                      unsigned horizon) {
    if (!g_Spellcard.IsActive() || g_Spellcard.activeEnemy != &owner)
        return false;
    Subprogram sub;
    if (!load_subprogram(sub_id, sub))
        return false;
    if (!cursor)
        cursor = sub.begin;
    auto position = std::find(sub.instructions.begin(), sub.instructions.end(), cursor);
    if (position == sub.instructions.end() || sub.instructions.end() - position < 3)
        return false;
    const auto &timer = **position, &reduction = **(position + 1), &end = **(position + 2);
    return timer.time == 0 && timer.opcode == ECL_OPCODE_SET_TIMER_CALLBACK &&
           timer.nextOffset == 20 && timer.operandFlags == 0 && timer.difficultyMask == 255 &&
           integer(timer, 0) >= int(horizon) && reduction.time == 0 &&
           reduction.opcode == ECL_OPCODE_SET_DAMAGE_REDUCTION_TIMER &&
           reduction.nextOffset == 16 && reduction.operandFlags == 0 &&
           reduction.difficultyMask == 255 && end.time == 0 && end.opcode == ECL_OPCODE_END_SPELL &&
           end.nextOffset == 12 && end.operandFlags == 0 && end.difficultyMask == 255;
}

bool timelines_stable_for(const Enemy *owner, unsigned horizon) {
    const int count = g_EclManager.GetTimelineCount();
    if (count < 0 || count > 16)
        return false;
    for (int index = 0; index < count; ++index) {
        const auto &timeline = g_EnemyManager.timelines[index];
        const auto *instruction = timeline.instruction;
        if (!instruction || timeline.timer.subFrame != 0.f || timeline.timer.current < 0 ||
            timeline.timer.current > INT_MAX - int(horizon))
            return false;
        if (instruction->time < 0)
            continue;
        if (instruction->time > timeline.timer.current + int(horizon) - 1)
            continue;
        if (instruction->size != 12 || timeline.timer.current != instruction->time ||
            !(instruction->difficultyMask & g_GameManager.difficultyMask))
            return false;
        if (instruction->opcode == ECL_TIMELINE_OPCODE_WAIT_FOR_EVENT) {
            // Event slots are published only by timelines. With every timeline
            // certified blocked/future/terminal, no publisher can run before
            // the protected boss transition that releases this group.
            for (int event : g_EnemyManager.timelineEventSlots)
                if (event == instruction->args.ints[0])
                    return false;
            continue;
        }
        if (instruction->opcode != ECL_TIMELINE_OPCODE_WAIT_FOR_BOSS_DEFEAT)
            return false;
        const int slot = instruction->args.ints[0];
        if (!owner || slot < 0 || slot >= 8 || g_EnemyManager.bosses[slot] != owner)
            return false;
        // This wait cannot advance while the certified boss is active. A
        // phase-death branch grants immunity before later timeline dispatch.
    }
    return true;
}

bool finite_values(std::initializer_list<float> values) {
    return std::all_of(values.begin(), values.end(), [](float v) { return std::isfinite(v); });
}
bool clock_ready() {
    return g_Supervisor.framerateMultiplier == 1.f && !g_Supervisor.flags.forceExtraTimerStep &&
           !g_GameManager.scriptedUpdateFreeze && !g_GameManager.flags.deathbombFreezeActive &&
           std::fegetround() == FE_TONEAREST;
}
} // namespace

EnemyBodyForecast Session::enemy_body_forecast(unsigned horizon) const {
    EnemyBodyForecast result;
    bool timeline_covered = false;
    auto reject = [&](BodyForecastFailure reason, int owner, unsigned update = 0, int opcode = -1) {
        result.failure = reason;
        result.owner = owner;
        result.update = update;
        result.opcode = opcode;
        return result;
    };
    if (horizon == 0 || horizon > 12 || !clock_ready())
        return reject(BodyForecastFailure::Clock, -1);
    if (g_Player.bombState.isInUse || g_Player.playerState != PLAYER_STATE_ALIVE)
        return reject(BodyForecastFailure::Owner, -1);
    for (const auto &enemy : g_EnemyManager.enemies) {
        const auto flags = enemy.flags1;
        if (!(flags & ENEMY_FLAG_ACTIVE))
            continue;
        if (flags & ENEMY_FLAG_LINKED_CHILD)
            return reject(BodyForecastFailure::Owner, enemy.enemyIndex);
        const bool eligible = (flags & ENEMY_FLAG_COLLISION) &&
                              !(flags & (ENEMY_FLAG_NO_SPRITE | ENEMY_FLAG_HIDE_PRIMARY_ANM |
                                         ENEMY_FLAG_YOUKAI_ALIGNED));
        if (!eligible) {
            // PERSIST_NONINTERACTIVE clears collision at death. Its already
            // installed callback grants EndSpell immunity before re-enabling
            // contact on the next native update; validate that exact prefix.
            const auto &pending = enemy.mainEclContextStorage;
            bool pending_immunity =
                (flags & ENEMY_FLAG_BOSS) && !(flags & ENEMY_FLAG_COLLISION) && enemy.life == 0 &&
                pending.time.current == 0 && pending.time.subFrame == 0.f &&
                pending.secondaryTime.current == 0 && pending.secondaryTime.subFrame == 0.f &&
                !pending.perFrameCallback && enemy.pendingEclSubroutineIndex < 0 &&
                !(enemy.flags2 & ENEMY_FLAG2_FORCE_PAUSE) &&
                end_spell_prefix(enemy, pending.subId, pending.currentInstr, horizon);
            for (const auto &slot : pending.interpolationSlots)
                pending_immunity &= !slot.callback;
            for (const auto *child : enemy.childEclBlocks)
                pending_immunity &= !child;
            if (pending_immunity && timelines_stable_for(&enemy, horizon)) {
                timeline_covered = true;
                continue;
            }
            if (!(flags & ENEMY_FLAG_NO_SPRITE) || !passive_owner(enemy))
                return reject(BodyForecastFailure::Spawn, enemy.enemyIndex);
            continue;
        }
        const auto &context = enemy.mainEclContextStorage;
        const auto unsupported = ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS | ENEMY_FLAG_SKIP_MOVEMENT |
                                 ENEMY_FLAG_MIRROR_MOVEMENT_X | ENEMY_FLAG_INHERIT_PARENT_POSITION;
        if (enemy.parentEnemy || enemy.trailFlags || !(flags & ENEMY_FLAG_BOSS) ||
            (flags & unsupported) || (enemy.flags2 & ENEMY_FLAG2_FORCE_PAUSE) ||
            context.perFrameCallback || enemy.pendingEclSubroutineIndex >= 0 ||
            context.secondaryTime.current || context.secondaryTime.subFrame != 0.f ||
            context.time.subFrame != 0.f || enemy.bossTimer.subFrame != 0.f ||
            enemy.movementTimer.subFrame != 0.f || enemy.life <= 0 ||
            enemy.positionOffset.x != 0.f || enemy.positionOffset.y != 0.f)
            return reject(BodyForecastFailure::Owner, enemy.enemyIndex);
        for (const auto *child : enemy.childEclBlocks)
            if (child)
                return reject(BodyForecastFailure::Owner, enemy.enemyIndex);
        for (const auto &slot : context.interpolationSlots)
            if (slot.callback)
                return reject(BodyForecastFailure::Motion, enemy.enemyIndex);
        const unsigned death_mode =
            (flags & ENEMY_FLAG_DEATH_MODE_MASK) >> ENEMY_FLAG_DEATH_MODE_SHIFT;
        // Ordinary damage is capped at 70 per native enemy update. Outside
        // the known phase-death immunity path, require death to be impossible
        // throughout this forecast; never assume an unknown callback is safe.
        const bool delayed_immunity =
            death_mode == ENEMY_DEATH_MODE_PERSIST_NONINTERACTIVE &&
            end_spell_prefix(enemy, enemy.deathCallbackSubId, nullptr, horizon);
        if (death_mode != ENEMY_DEATH_MODE_END_BOSS_PHASE && !delayed_immunity &&
            enemy.life <= 70 * int(horizon))
            return reject(BodyForecastFailure::Lifecycle, enemy.enemyIndex);
        if (!timelines_stable_for(&enemy, horizon))
            return reject(BodyForecastFailure::Program, enemy.enemyIndex);
        timeline_covered = true;
        // A nonzero main return stack is harmless here: RETURN/CALL are not
        // supported future instructions, so this forecast never consumes it.
        float x = enemy.position.x, y = enemy.position.y;
        float vx = enemy.velocity.x, vy = enemy.velocity.y;
        float ox = enemy.movementInterpolationOrigin.x, oy = enemy.movementInterpolationOrigin.y;
        const float dx = enemy.movementInterpolationDelta.x,
                    dy = enemy.movementInterpolationDelta.y;
        const auto &bounds = enemy.movementBounds;
        if (!finite_values({x, y, vx, vy, ox, oy, dx, dy, enemy.hitboxDimensions.x,
                            enemy.hitboxDimensions.y, bounds.lower.x, bounds.lower.y,
                            bounds.upper.x, bounds.upper.y}) ||
            enemy.hitboxDimensions.x < 0 || enemy.hitboxDimensions.y < 0 ||
            bounds.lower.x > bounds.upper.x || bounds.lower.y > bounds.upper.y)
            return reject(BodyForecastFailure::Numeric, enemy.enemyIndex);
        const bool clamped = flags & ENEMY_FLAG_CLAMP_POSITION;
        if (clamped &&
            (x < bounds.lower.x || x > bounds.upper.x || y < bounds.lower.y || y > bounds.upper.y))
            return reject(BodyForecastFailure::Motion, enemy.enemyIndex);
        unsigned mode = (flags & ENEMY_FLAG_MOVEMENT_MODE_MASK) >> ENEMY_FLAG_MOVEMENT_MODE_SHIFT;
        unsigned easing = (flags >> ENEMY_FLAG_MOVEMENT_EASING_SHIFT) & 7;
        int timer = enemy.movementTimer.current, duration = enemy.movementDuration;
        int time = context.time.current;
        if ((mode != ENEMY_MOVEMENT_MODE_NONE && mode != ENEMY_MOVEMENT_MODE_INTERPOLATED) ||
            (mode == ENEMY_MOVEMENT_MODE_INTERPOLATED &&
             (duration <= 0 || timer < 0 || timer > duration || easing > 6)) ||
            time < 0 || time > INT_MAX - int(horizon) || enemy.bossTimer.current < 0 ||
            enemy.bossTimer.current > INT_MAX - int(horizon))
            return reject(BodyForecastFailure::Motion, enemy.enemyIndex);
        Subprogram sub;
        if (!load_subprogram(context.subId, sub) ||
            std::find(sub.instructions.begin(), sub.instructions.end(), context.currentInstr) ==
                sub.instructions.end())
            return reject(BodyForecastFailure::Program, enemy.enemyIndex);
        const auto *instruction = context.currentInstr;
        bool uncertain = false;
        Interval position_x{x, x}, position_y{y, y}, delta{0, 0};
        try {
            for (unsigned update = 1; update <= horizon; ++update, ++time) {
                // Boss callbacks grant >=70 invulnerability updates before
                // body contact, so this known timer boundary has no lethal tail.
                if (enemy.timerCallbackThresholdFrames >= 0 &&
                    enemy.bossTimer.current + int(update) - 1 >= enemy.timerCallbackThresholdFrames)
                    break;
                unsigned dispatched = 0;
                while (instruction->time == time) {
                    if (++dispatched > 100)
                        return reject(BodyForecastFailure::Program, enemy.enemyIndex, update);
                    const auto &i = *instruction;
                    const auto mask =
                        g_GameManager.difficultyMask | enemy.eclDifficultyMaskOverride;
                    if ((i.difficultyMask & mask) == mask) {
                        const auto op = i.opcode;
                        bool neutral =
                            (op == 0 && i.nextOffset == 12 && !i.operandFlags) ||
                            (op == ECL_OPCODE_ENABLE_INTERACTION_FLAGS && i.nextOffset == 16 &&
                             !i.operandFlags && integer(i, 0) == ECL_INTERACTION_DAMAGEABLE) ||
                            (op == ECL_OPCODE_SET_DAMAGE_REDUCTION_TIMER && i.nextOffset == 16) ||
                            (op == ECL_OPCODE_SET_INT && i.nextOffset == 20 &&
                             (i.operandFlags & 1) && !(i.operandFlags & ~3u) &&
                             int_variable(integer(i, 0))) ||
                            ((op == ECL_OPCODE_SET_FLOAT || op == ECL_OPCODE_FLOAT_ADD_ASSIGN) &&
                             i.nextOffset == 20 && (i.operandFlags & 1) &&
                             !(i.operandFlags & ~3u) && float_variable(scalar(i, 0))) ||
                            (op == ECL_OPCODE_NORMALIZE_ANGLE && i.nextOffset == 16 &&
                             i.operandFlags == 1 && float_variable(scalar(i, 0))) ||
                            (op == ECL_OPCODE_SHOOT_FAN_AIMED && i.nextOffset == 44);
                        if (op == ECL_OPCODE_SPAWN_ENEMY_RELATIVE && i.nextOffset == 40 &&
                            !i.operandFlags) {
                            std::set<int> visited;
                            neutral = passive_subgraph(integer(i, 0), true, visited);
                            if (!neutral)
                                return reject(BodyForecastFailure::Spawn, enemy.enemyIndex, update,
                                              op);
                        }
                        if (op == ECL_OPCODE_JUMP && i.nextOffset == 20 && !i.operandFlags) {
                            const int target_time = integer(i, 0);
                            const auto *target = jump_target(sub, i, integer(i, 1));
                            if (!target || target_time < 0 || target_time > INT_MAX - int(horizon))
                                return reject(BodyForecastFailure::Program, enemy.enemyIndex,
                                              update, op);
                            instruction = target;
                            time = target_time;
                            continue;
                        }
                        if (op == ECL_OPCODE_MOVE_RANDOM_IN_BOUNDS && i.nextOffset == 24 &&
                            !i.operandFlags) {
                            duration = integer(i, 0);
                            easing = unsigned(integer(i, 1));
                            const float speed = scalar(i, 2);
                            if (uncertain || duration <= 0 || easing > 6 || !std::isfinite(speed))
                                return reject(BodyForecastFailure::Motion, enemy.enemyIndex, update,
                                              op);
                            timer = duration;
                            ox = x;
                            oy = y;
                            mode = ENEMY_MOVEMENT_MODE_INTERPOLATED;
                            delta = body_detail::random_delta(speed, duration);
                            if (delta.lo > 0 || delta.hi < 0 ||
                                (clamped && (ox < bounds.lower.x || ox > bounds.upper.x ||
                                             oy < bounds.lower.y || oy > bounds.upper.y)))
                                return reject(BodyForecastFailure::Numeric, enemy.enemyIndex,
                                              update);
                            uncertain = true;
                            position_x = {x, x};
                            position_y = {y, y};
                        } else if (!neutral)
                            return reject(BodyForecastFailure::Program, enemy.enemyIndex, update,
                                          op);
                    }
                    const auto address =
                        reinterpret_cast<uintptr_t>(instruction) + instruction->nextOffset;
                    instruction = reinterpret_cast<const EclRawInstruction *>(address);
                    if (std::find(sub.instructions.begin(), sub.instructions.end(), instruction) ==
                        sub.instructions.end())
                        return reject(BodyForecastFailure::Program, enemy.enemyIndex, update);
                }
                if (mode == ENEMY_MOVEMENT_MODE_INTERPOLATED) {
                    --timer;
                    const float progress = body_detail::eased_progress(timer, duration, easing);
                    if (uncertain) {
                        position_x = body_detail::interpolated_position(position_x, ox, delta,
                                                                        progress, timer <= 0);
                        position_y = body_detail::interpolated_position(position_y, oy, delta,
                                                                        progress, timer <= 0);
                        if (timer <= 0) {
                            mode = ENEMY_MOVEMENT_MODE_NONE;
                            vx = vy = 0;
                        }
                    } else {
                        vx = ox + dx * progress - x;
                        vy = oy + dy * progress - y;
                        if (timer <= 0) {
                            mode = ENEMY_MOVEMENT_MODE_NONE;
                            x = ox + dx;
                            y = oy + dy;
                            vx = vy = 0;
                        }
                    }
                }
                if (uncertain) {
                    // This pre-clamp all-angle interval contains its in-bounds
                    // origin. Native clamping maps it into itself; retain the
                    // canonical maximum-excursion warning rather than tighten
                    // the policy's proposal region or add arbitrary padding.
                    if (position_x.lo > ox || position_x.hi < ox || position_y.lo > oy ||
                        position_y.hi < oy)
                        return reject(BodyForecastFailure::Numeric, enemy.enemyIndex, update);
                    const auto bx = body_detail::body_bounds(position_x, enemy.hitboxDimensions.x);
                    const auto by = body_detail::body_bounds(position_y, enemy.hitboxDimensions.y);
                    result.warnings.push_back({bx.lo, by.lo, bx.hi, by.hi, update, enemy.enemyIndex,
                                               BodyForecastKind::RandomMoveEnvelope});
                } else {
                    if (clamped) {
                        x = std::clamp(x, bounds.lower.x, bounds.upper.x);
                        y = std::clamp(y, bounds.lower.y, bounds.upper.y);
                    }
                    x += vx;
                    y += vy;
                    if (clamped) {
                        x = std::clamp(x, bounds.lower.x, bounds.upper.x);
                        y = std::clamp(y, bounds.lower.y, bounds.upper.y);
                    }
                    if (!finite_values({x, y}))
                        return reject(BodyForecastFailure::Numeric, enemy.enemyIndex, update);
                    const float hx = (enemy.hitboxDimensions.x * (1.f / 1.5f)) / 2.f;
                    const float hy = (enemy.hitboxDimensions.y * (1.f / 1.5f)) / 2.f;
                    const float min_x = x - hx, min_y = y - hy;
                    const float max_x = x + hx, max_y = y + hy;
                    if (!finite_values({min_x, min_y, max_x, max_y}))
                        return reject(BodyForecastFailure::Numeric, enemy.enemyIndex, update);
                    // Potential life/death callbacks are covered conditionally:
                    // no transition follows this motion, or native immunity
                    // prevents lethal contact for the remainder of H<=12.
                    result.warnings.push_back({min_x, min_y, max_x, max_y, update, enemy.enemyIndex,
                                               BodyForecastKind::ConditionalLifecycle});
                }
            }
        } catch (const std::exception &) {
            return reject(BodyForecastFailure::Numeric, enemy.enemyIndex);
        }
    }
    // An empty/currently passive actor set is not evidence of an empty future.
    // Check timeline publication even when no body entered the owner-specific
    // certificate. A boss wait requires an actually protected owner, not nullptr.
    if (!timeline_covered && !timelines_stable_for(nullptr, horizon))
        return reject(BodyForecastFailure::Program, -1);
    return result;
}
} // namespace th08::headless
