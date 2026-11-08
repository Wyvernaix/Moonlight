# ADR 0003: ABI and plugin loading

**Status:** Accepted
**Date:** 2026-01-01

## Context

Plugins must survive compiler, platform and language changes without exposing
C++ class layouts or allocator assumptions. Loading untrusted or stale binaries
must fail before execution.

## Decision

Use the versioned C table and single discovery symbol in [plugin ABI](../plugin-abi.md).
Admission validates major/minor version, structure size, signatures, platform,
hash/signature, ownership, capabilities and lifecycle hooks. ABI major changes
are breaking; minor versions are append-only with size guards.

## Consequences

The host owns lifecycle, deadlines and callback unregistering. Plugins require
fixture builds and explicit signing/allow-list policy. This is more verbose
than direct dynamic linking but makes failure diagnosable.

## Rejected alternatives

Passing C++ interfaces is rejected because compiler/STL/RTTI/allocator choices
are not stable. Blindly accepting a symbol is rejected as unsafe.

## Validation

Test old/new fixtures, wrong sizes, unsupported platforms, tampered binaries,
allocator errors, callback races and bounded unload.
