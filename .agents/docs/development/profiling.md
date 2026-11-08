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
