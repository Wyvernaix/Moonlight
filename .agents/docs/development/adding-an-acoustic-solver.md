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
