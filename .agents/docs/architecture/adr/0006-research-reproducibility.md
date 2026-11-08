# ADR 0006: Research reproducibility

**Status:** Accepted
**Date:** 2026-01-01

## Context

Spectral, acoustic and rendering results are meaningful only when inputs,
software and numerical settings can be reconstructed. Large fields and captures
cannot all live in Git.

## Decision

Every run emits a manifest containing source commit, module/catalog hash,
platform, compiler, build and renderer IDs, input asset hashes, seed, solver
parameters, numerical tolerances, timestamps and output hashes. Large outputs
use the content-addressed asset store; manifests and checksums are committed
or attached to the run record.

## Consequences

Runs become auditable and comparable, at the cost of manifest discipline and
storage retention. A run with missing provenance is marked exploratory and
cannot be used as a reference result.

## Rejected alternatives

Recording only a Git commit omits toolchains, assets and runtime parameters.
Storing all binaries in Git is impractical and obscures integrity.

## Validation

CI validates required manifest fields, hash resolution, deterministic seeds,
schema compatibility and reference-render comparisons.
