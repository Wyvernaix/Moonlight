# Repository map

**Status:** Accepted

| Area | Responsibility |
| --- | --- |
| `src/native/` | C++ runtime and public native ABI |
| `src/managed/` | C# SDK, interop, Editor and tooling |
| `modules/` | Registered C++/Rust engine modules |
| `scripts/` | Project and module command-line tooling |
| `python/` | Build, validation and repository automation |
| `data/schemas/` | Canonical versioned data and module contracts in the target architecture |
| `data/config/` | Shared engine, platform, rendering, audio, and CI profiles in the target architecture |
| `cmake/` | Shared native build helpers |
| `tests/` | Cross-language and contract tests |
| `.agents/docs/` | Agent-maintained normalized architecture and module documentation |

Documentation is organized by responsibility rather than source-note
filename. Module-specific contracts belong under `modules/<name>/`, shared
runtime rules under `architecture/`, and reproducibility guidance under
`research/`. The public documentation site can later generate navigation and
API material from this workspace without changing the source documents.

## Change ownership

Build and schema changes must update their directly related docs and fixtures.
Generated files under `build/` are disposable. Architecture source is limited
to `.agents/docs/architecture`; links should be relative and stable. See
[overview](overview.md), [assets and data](assets-and-data.md), and
[versioning](versioning.md).

The current milestone still contains `schemas/module-manifest.schema.json`;
that path is a migration-compatible implementation location. New shared
schemas and configuration must use `data/schemas/` and `data/config/` as
defined in `moonlight-monorepo-architecture.md`. The module validator must
support the transition without accepting two conflicting copies of the same
schema ID.
