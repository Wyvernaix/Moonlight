# ADR 0001: Language boundaries

**Status:** Accepted
**Date:** 2026-01-01

## Context

Runtime code needs predictable performance and a stable plugin surface, while
Editor and automation code benefit from managed tooling and rapid iteration.
Compiler-specific layouts and allocators cannot be a product boundary.

## Decision

C++ and Rust implement engine modules; C# implements Editor, Workbench,
scripting and managed tooling; Python implements automation and project
management. Native/managed communication uses the versioned C ABI described in
[plugin ABI](../plugin-abi.md), with generated bindings where useful.

## Consequences

Modules must keep public APIs C-compatible or language-local. Python is not a
runtime dependency. Managed code may consume runtime services but cannot be a
dependency of shipped runtime targets. This adds binding and fixture work but
keeps deployment and compiler upgrades independent.

## Rejected alternatives

Using C# for the runtime would couple the host to a managed GC and platform
runtime. A single C++ ABI would not safely cover Rust and managed consumers.

## Validation

CI builds all language boundaries, runs ABI fixtures and rejects forbidden
dependency edges. Revisit this decision only with a measured deployment or
performance constraint.
