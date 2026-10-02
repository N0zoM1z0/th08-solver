# Imported TH08 game runtime

Source: https://github.com/N0zoM1z0/th08, branch `port/portable-64bit`,
commit `861bec908b84fa4658382d7526e5a0075f520846`.

This is an ordinary tracked source tree in th08-solver, not a submodule or an
independent Git repository. It retains the reconstructed game translation units,
native Linux compatibility headers/adapters, translation table and MIT license.
Upstream agent skills, workflows, reconstruction tooling, documentation and images
are omitted. Original game data are supplied separately and never distributed.

The solver's [headless build](../../cmake/Headless.cmake) replaces window/render
submission with CPU resource storage, exposes deterministic input/time, and makes
resource initialization synchronous. Gameplay and ANM update code remain in the
imported production translation units. Changes are guarded by `TH08_HEADLESS`.
The numerical profile uses native float32 with contraction disabled and no
fast-math. See the solver's [validation](../../docs/VALIDATION.md).
