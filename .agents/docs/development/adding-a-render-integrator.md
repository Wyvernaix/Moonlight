# Adding a render integrator

A render integrator consumes immutable extracted scene data and produces a frame through a declared backend. Begin with a design note covering supported features, coordinate/colour spaces, units, sample strategy, numerical assumptions, resource ownership, synchronization, and fallback behavior.

## Sequence

1. Add a module and manifest dependency on core scene/extraction contracts.
2. Define the integrator interface and capability query; reject unsupported features explicitly.
3. Implement resource creation, update, render, and destruction with clear thread ownership.
4. Add a deterministic reference-scene test and an image/numerical tolerance.
5. Exercise resize, empty scene, missing resource, device loss, and shutdown paths.
6. Validate/build/test on all supported platforms.

```sh
python scripts/moonlight.py validate
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

The render loop must not mutate Atlas or retain pointers into transient extraction buffers. GPU resources are owned by the integrator/device layer, released in reverse dependency order, and synchronized before destruction. Avoid blocking or unbounded allocation on frame-critical threads. Log capability failures without leaking scene/user data.

Approximation is acceptable only when documented and compared with the reference renderer under a stated tolerance. Record GPU/driver, resolution, seed, and capture settings. Add a performance budget and regression fixture; owner approval is required for public API, shader binding, frame-graph, or serialization changes.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
ctest --preset debug --output-on-failure
```

**Expected exit condition:** every command exits 0; module validation reports `validated 1 module(s)` and a complete build ends with `Moonlight build completed successfully.`. A deliberate unsupported-platform/backend result is recorded as such, never silently treated as a pass.

| Environment | Required evidence | CI comparison |
|---|---|---|
| Ubuntu Linux | compiler, CMake/Ninja, Python, Rust, .NET versions; `build/` logs | Ubuntu job in `build.yml`/`test.yml` |
| macOS | same versions plus architecture and SDK | macOS matrix job |
| Windows PowerShell | same versions plus MSVC generator/toolset | Windows matrix job |

Reproducibility requires pinned lockfiles and `rust-toolchain.toml`, no network-dependent tests, deterministic seeds, and a clean `build/debug` or `build/release` directory. Preserve `build/**/Testing/Temporary/LastTest.log`, compiler output, test reports, catalogs, profiles, or captures as applicable. Roll back by reverting the change and deleting only generated `build/`, `target/`, `bin/`, or `obj/` outputs before rerunning; never delete source or change CI to hide a failure.

For failures, keep the first error, run the smallest command above that reproduces it, compare tool versions with CI, and inspect `LastTest.log`; classify the result as environment, deterministic test, data/ABI, or concurrency failure before editing. Static checks include `git diff --check`, Python compilation, Rust fmt/clippy, and the repository CMake/.NET checks. Add property-based, fuzz, sanitiser, or numerical-regression coverage when parsing manifests, serialized data, shaders, memory ownership, or solver math changes; document unavailable tooling and the seed/input instead of claiming coverage.

Security boundary: examples must not access production credentials, network services, customer content, or unrestricted filesystem paths. Scrub paths, tokens, dumps, and personal data from artifacts; review generated archives and logs before upload. CI parity is the corresponding workflow under `.github/workflows/`; local success does not replace the required Linux/macOS/Windows matrix.
