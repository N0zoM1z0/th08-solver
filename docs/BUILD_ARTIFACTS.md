# Reusable source probes and build artifacts

Keep maintained C++ in `include/`, `src/`, `tools/`, `tests/`, or `benchmarks/`.
Root `build*/` directories are disposable, ignored outputs. Do not force-add them:
generated translation units contain extracted reference-source bodies, and compiler
probes and fault-injected headers are not maintained solver implementations.

## Local scratch audit (2026-09-12)

| Local file | Tracked reusable source or replacement |
|---|---|
| `build-enemy-agent/generate.cpp` | `tools/source_probe.cpp`, `source_world_motion` CMake target |
| `build-enemy-agent/source_enemy_motion.cpp` | `tools/enemy_motion_source_probe.cpp`, `tests/source_enemy_motion_cases.hpp` |
| `build-enemy-agent/source_world_motion.cpp` | `tools/world_motion_source_probe.cpp`, `tests/source_world_motion_cases.hpp` |
| `build-anm-agent/camera_source_driver.cpp` | `tools/source_probe.cpp`, `source_camera_particle` CMake target |
| `build-anm-agent/camera_oracle.cpp` | `tools/camera_particle_source_probe.cpp`, `tests/source_camera_particle_cases.hpp` |
| `build/source_oracle.cpp`, `build-anm-agent/source_oracle.cpp` | `tools/*_source_probe.cpp`, `tools/source_probe.cpp`, `tests/source_*_cases.hpp` |
| `build-anm-agent/camera_fault_include/th08/camera_particle.hpp` | Intentional mutation of `include/th08/camera_particle.hpp`; not a reusable implementation |

The camera fault changes normalization's `length > 1.0e-8f` test to `length > 0`.
Its tiny-vector counterexample is already in the tracked camera tests and source
comparison. Keep the correct header, regression, and explanation, not a duplicate
incorrect implementation. Existing local scratch files are left untouched.

The old enemy driver spliced a previous generated translation unit and depended on
its exact textual layout. The maintained replacement generates directly from the
hash-pinned reference checkout. No previous build directory is needed.

## Independent component comparisons

With the pinned reference checkout prepared as described in [Validation](VALIDATION.md):

```sh
cmake -S . -B build-probes -DCMAKE_BUILD_TYPE=Release \
  -DTH08_REFERENCE_SOURCE="$PWD/.cache/th08"
cmake --build build-probes --parallel 2 \
  --target source_enemy_motion source_world_motion source_camera_particle source_spawn
./build-probes/source_enemy_motion
./build-probes/source_world_motion
./build-probes/source_camera_particle
./build-probes/source_spawn
```

The first three opt-in targets reuse the same source bodies and comparisons as the
integrated `source_oracle` CTest. `source_spawn` separately checks6000 transactions
against unchanged SpawnEnemy1/2 bodies, with a controlled immediate-ECL boundary.
It does not compare full RunEcl or a full world. These targets add no work to the default CTest
suite. Add `-DTH08_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug` in a separate build directory
to instrument them with ASan/UBSan. The source generator also accepts a final
`enemy_motion`, `world_motion`, `camera_particle`, or `spawn` argument; omitting it generates
the unchanged integrated oracle. Unknown component names fail before opening output.

Source hashes are checked on generation. These comparisons establish the documented
native component profile, not retail executable equivalence or complete spell solving.

For future experiments, promote reusable drivers and deterministic cases into the
maintained directories before calling the work complete. Track their CMake wiring
and reproduction instructions; leave binaries, generated source, raw game data,
and intentionally broken local copies ignored.
