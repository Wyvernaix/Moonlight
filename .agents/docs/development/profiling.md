# Profiling

Profile a Release build for performance conclusions and a Debug build only for locating behavior. Record commit, OS/CPU/GPU, compiler and SDK versions, CMake preset, workload/input seed, warm-up policy, profiler/version, sample count, and artifact path. Never treat one noisy run as a regression.

```sh
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
```

Linux tools include `perf` (where permitted) and heap profilers; macOS uses Instruments; Windows uses Visual Studio Performance Profiler or WPA. GPU captures must use the backend’s supported capture tool and a reproducible scene. Keep captures outside the source tree or under ignored `build/profiles/`; scrub paths, usernames, and customer data before sharing.

Measure a baseline and candidate on the same machine, with identical power mode and workload. Report median and percentile latency, throughput, allocations, frame time, and confidence/variance. Separate CPU, GPU, synchronization, I/O, and startup costs. Instrumentation must be bounded and disabled in production builds unless explicitly approved.

## Budgets and ownership

Use budgets documented in `performance-budgets.md`; a budget violation blocks merge until the owning team accepts a new budget with evidence. Render, audio, and streaming loops must not allocate or block on real-time threads. Profiled changes require owner review, a regression test or benchmark where practical, and a before/after artifact—not just a screenshot.

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
