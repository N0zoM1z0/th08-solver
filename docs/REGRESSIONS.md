# Source-derived regression ledger

## Direction changes retain fractional clock age

### Expectation, ranked probes, and independent oracle

With a finite positive frame-rate multiplier, a direction transform must use the
source `ZunTimer` integer part for firing comparisons and integer-plus-fraction for
the deceleration ramp. Its increment follows `Supervisor::TickTimer`. The control
uses multiplier 1; the highest-value variation changes only that value to 0.5.
Rate changes retaining fractional carry and firing/reset boundaries rank next.
All are reachable through the existing public motion API.

Inspection found that the earlier direction oracle substituted an integer for
`BulletExState::timer`, although the source methods themselves were unchanged.
After replacing that adapter with the actual pinned `ZunTimer`, the unmodified
motion kernel produced 189423 mismatches over its 447684-frame test sequence.
Thus the previous zero-mismatch result did not establish non-unit clock behavior.
The independent oracle now includes the original timer class and tick method.

### Minimal counterexample and durable guard

Initialize a relative turn with interval 2, one repetition, completed count and
timer zero, turn angle 1, turn speed 3. The bullet starts with angle 0 and speed 4.
One update with multiplier 0.5 must leave integer timer 0 and fraction 0.5, with
x velocity 2. The old implementation returned timer 1. A second update must use
age 0.5, producing x velocity 1.5 and integer timer 1 with zero fraction.

`tests/bullet_motion_tests.cpp` first failed on the unfixed implementation with
`half-rate direction clock incorrectly advanced a whole frame`. The fixed guard
also checks a 0.5-to-1 rate change retaining fractional carry, then firing at the
integer threshold and resetting the fraction before that frame's increment.
It repeats with x positions 0, 100, and 0; position does not affect this relative
turn contract. All setup is local and needs no external cleanup.
The guard passes after the fix, and the updated source oracle passes 584936 direction
frames with no mismatches. Both 600-frame unit-rate fixtures solve and replay again.

### Scope and residual limits

The maintained kernel now stores a fractional age, while the source oracle checks
both timer components, velocity, angle, speed, completion count and active flag.
The unit-rate particle fixtures remain regression controls. This does not verify
retail x87 arithmetic, spawn ANM at non-unit rates, concurrent transforms sharing
a state slot, or complete world timing. Those remain separate explicit boundaries.

## Signed terminal laser dimensions

### Expectation and independent oracle

The laser collision/lifetime projection must retain every ordered collision call
from the pinned `BulletManager::OnUpdate` laser loop, including calls on the frame
that retires a laser. The oracle compiles that unchanged loop with the original
`ZunTimer` and `Supervisor::TickTimer`; a recorder captures its collision arguments.
It does not obtain expected sizes from the maintained implementation.

The control is an ordinary positive-width active laser. Three ranked variations
were relevant: terminal ramp roundoff (can delete an actual collision call), phase
fallthrough (can lose additional checks), and fractional timer carry (can shift
activation). The first produced a minimal, repeatable failure.

### Minimal counterexample

Use the modern float32 profile without fast-math or contraction. A despawning laser
has width `0x1.db89b4p+3f`, timer and despawn duration 12, and hitbox end delay 34.
Set position to (-50,0), start offset 0, end offset and length 100, angle and speed 0.
One update computes `(width - float(12) * width / float(12)) / 2 = -0x1p-21f`.
The source emits one collision call and retires the laser without incrementing time.

The first isolated-loop probe failure was scenario 66, frame 124. That initial probe
used an integer timer; the final oracle additionally retains the original fractional
timer. The minimized unit-rate example applies to both. The previous projection returned
invalid with zero calls. A standalone regression then reproduced the failure with
`despawn roundoff lost the signed terminal hitbox` before the implementation changed.

Clamping the size to zero is also wrong: with the resulting center at zero and player
half size (1,1), player position (1,0) misses the signed source rectangle but touches
the clamped rectangle. Position (0,0) still collides with the signed rectangle.

### Guard and forge proof

`tests/laser_motion_tests.cpp` constructs the exact state, advances once, checks the
signed hexadecimal result, checks retirement and call count, and tests both contact
positions through narrow predicates and the owned spatial index. It repeats the setup
three times, perturbing only the unrelated y coordinate. The guard failed before the
fix and passed afterward. No timing, game process, filesystem cleanup, or external
service is involved.

Laser dimensions now retain their sign. The broad phase uses absolute values only
for error padding and clamps empty expanded radii, not the narrow-phase dimensions.
Bullet dimensions and player half sizes still reject negative input. Randomized source
predicate comparisons and index-versus-scan tests also include signed laser dimensions.

### Residual limits

This proves a collision/lifetime projection, not rendering or retail x87 equivalence.
External ECL/EX changes, cancellation, complete spell context, and all malformed engine
states remain separate contracts. A few declared finite-domain bounds still reject
unverified inputs explicitly. Passing this regression does not establish all-case solving.
