# ADR 0004: Scene snapshot contract

**Status:** Accepted
**Date:** 2026-01-01

## Context

Renderer, audio, physics and research systems need consistent data while Atlas
continues editing. Shared mutable objects create races and make results
irreproducible.

## Decision

Atlas is the sole owner of authoring state. It publishes immutable,
versioned snapshots with stable entity IDs, source revision, extraction
options and content hashes. Consumers acquire/release handles and never retain
authoring pointers. See [scene snapshots](../scene-snapshots.md).

## Consequences

Extraction costs memory and can lag edits; consumers gain deterministic,
concurrent reads and clear lifetimes. Unknown required components fail
acquisition rather than silently changing meaning.

## Rejected alternatives

Live shared ECS access was rejected for race and determinism risks. Per-system
copies were rejected because they drift and multiply memory.

## Validation

Test atomic publication, concurrent readers, stable IDs, failed extraction,
schema migration and hash reproducibility.
