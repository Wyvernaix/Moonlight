# Dependency rules

**Status:** Accepted

## Graph contract

`module.json` declares `id`, `version`, `dependencies.required`,
`dependencies.optional`, provided capabilities, platform constraints and ABI
requirements. IDs are lowercase dotted names. A required edge means “must be
started and compatible”; an optional edge means “may be used if present”.
Capability requirements resolve to exactly one compatible provider unless the
consumer declares a deterministic preference.

Build dependencies (CMake targets, Cargo crates and .NET projects) are a
separate graph from runtime dependencies. Generated catalogs must preserve both
layers; a link edge is not proof of a runtime edge.

## Invariants and policy

- Required edges form a DAG; self-edges and duplicate edges are invalid.
- A module cannot depend on Editor, Workbench or test-only targets.
- Capability providers must declare API version and platform support.
- Optional absence must select a documented fallback before `Started`.
- Dependency changes require graph diff review and a cycle check.
- Hidden global lookups and direct cross-module private includes are forbidden.

## Resolution state machine

`ManifestFound → SchemaValid → IDsUnique → RequiredResolved →
CapabilitiesResolved → PolicyApproved`. Failure is terminal for the build.
Runtime resolution additionally checks ABI, platform and version ranges; it
never substitutes an incompatible provider.

## Example

```json
{
  "id": "moonlight.renderer.vulkan",
  "version": "1.2.0",
  "dependencies": {"required": ["moonlight.core"], "optional": ["moonlight.audio"]},
  "provides": ["renderer.v1"],
  "platforms": ["linux", "windows"]
}
```

Use the [module graph](module-graph.md) and [module system](module-system.md)
for generated outputs and the [lifecycle](module-lifecycle.md) for ordering.
Tests must cover cycles, missing required IDs, optional fallback, capability
ambiguity, version incompatibility and platform filtering.
