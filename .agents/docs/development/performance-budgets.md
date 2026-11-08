# Performance budgets

Budgets are acceptance criteria, not targets to average away. Record hardware, OS, compiler, preset, workload, warm-up, sample count, median, p95/p99, and variance. Compare the same commit baseline and candidate; investigate regressions beyond measurement noise.

Initial engineering budgets (adjust only with benchmark evidence and owner approval):

| Area | Budget |
|---|---|
| native unit-test/build correctness | zero test failures; no new warnings |
| frame-critical render path | no blocking calls or unbounded allocation |
| audio callback | no allocation, filesystem, locks with unbounded wait, or logging |
| module validation | deterministic output and zero undeclared dependencies |
| interactive frame work | document target frame rate and p95 frame time per integrator |
| acoustic update | document solver latency and bounded queue/backpressure behavior |

Use Release builds for measurements:

```sh
cmake --preset release && cmake --build --preset release
ctest --preset release --output-on-failure
```

A budget change requires before/after evidence, a reproducible workload, and review from the owning render/audio/performance team. Keep profiler captures in ignored build output, scrub user data, and do not commit machine-specific traces.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python python/build.py --configuration release
ctest --preset release --output-on-failure
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
