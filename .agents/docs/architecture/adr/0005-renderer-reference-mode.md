# ADR 0005: Renderer reference mode

**Status:** Accepted
**Date:** 2026-01-01

## Context

Fast renderers and spectral/research solvers trade accuracy for throughput.
Without an independent baseline, “close enough” cannot be measured or
reproduced.

## Decision

Keep a deterministic reference renderer independent from approximate paths.
Each comparison records scene snapshot hash, renderer/build IDs, camera,
seed, numerical settings, runtime, memory, validity regime and error metrics
(absolute, relative and perceptual where applicable).

## Consequences

Reference mode is slower and may support fewer platforms, but it is the
compatibility oracle for tests and research. Approximate renderers must declare
when the reference comparison is invalid rather than inventing a score.

## Rejected alternatives

Using production output as its own oracle permits correlated bugs. A single
accuracy threshold was rejected because regimes and metrics differ.

## Validation

Golden scenes, deterministic repeated runs, convergence tests and documented
error budgets are required. See [graph specification](../graph-system-specification.md).

## Contract checks

Reference comparisons require matching snapshot and asset hashes, compatible
settings, and a declared validity regime. A successful comparison emits
reproducible metrics; missing provenance or an invalid regime yields
`NotComparable`, never a fabricated score. Runs are isolated so concurrent
comparisons cannot mutate shared renderer state.
