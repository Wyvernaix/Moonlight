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
