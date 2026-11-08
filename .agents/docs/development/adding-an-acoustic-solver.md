# Adding an acoustic solver

An acoustic solver must state its units (SI metres, seconds, pascals), sample rate, frequency range, boundary/material model, valid geometry scale, numerical method, stability constraints, and tolerances. Define behavior for degenerate geometry, disconnected regions, invalid materials, and unsupported features before coding.

## Validation-first sequence

1. Add a module with explicit dependencies and a solver capability manifest.
2. Define immutable input snapshots and output units/ownership.
3. Implement analytical cases (free field, simple reflection, or known modal response).
4. Add conservation/stability, determinism, invalid-input, and regression tests.
5. Integrate with the audio graph and verify device-loss/shutdown behavior.

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
cargo test --workspace --locked  # when the solver has Rust targets
```

Real-time audio callbacks must be non-blocking, allocation-free, lock-free where required by the audio contract, and free of filesystem, logging, and GPU calls. Precompute or swap immutable snapshots off the callback; use bounded queues and explicit backpressure. Solver workers own their scratch memory and publish results atomically; callers must not retain mutable buffers.

Test multiple platforms and sample rates, with deterministic seeds and tolerances appropriate to floating point. A failed convergence or NaN is an explicit error/capability result, never silent audio corruption. Document CPU cost and latency budgets in `performance-budgets.md`; owner review is mandatory for units, ABI, audio-thread, or serialized-input changes.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
cargo test --workspace --locked
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
