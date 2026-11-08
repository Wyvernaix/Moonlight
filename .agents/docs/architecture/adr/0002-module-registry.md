# ADR 0002: Module registry

**Status:** Accepted
**Date:** 2026-01-01

## Context

Build metadata and diagrams drift when each tool keeps its own module list.
Runtime startup also needs a validated dependency and capability graph.

## Decision

`module.json` manifests are canonical. The Python registry discovers, schema
validates, checks unique IDs and required cycles, and generates the catalog
used by build, runtime diagnostics and graph tooling. Hand-maintained duplicate
graphs are prohibited. See [module system](../module-system.md).

## Consequences

Adding a module requires a manifest, tests and documentation. Generated
catalogs are reproducible and can be checked in CI, but a graph compiler must
eventually extract build metadata without redefining runtime semantics.

## Rejected alternatives

Directory scanning without manifests loses version, capability and platform
contracts. Maintaining a second YAML graph guarantees drift.

## Validation

Validation tests cover duplicate IDs, malformed manifests, missing required
dependencies, cycles, deterministic export and optional providers.

## Contract checks

Discovery starts from a bounded repository root and produces either a complete
catalog or a typed failure; partial catalogs are not valid inputs to build or
startup. The catalog postcondition is stable ordering and a content hash over
canonical manifests. Concurrent readers may consume a published catalog, but
writers publish by atomic replacement. See [architecture conformance](../architecture-conformance.md).
