# Data formats

**Status:** Draft

## Contract

Every persisted document is UTF-8 JSON (or a documented binary container)
with a top-level `schema`, `version`, `id`, and `metadata` object. Writers emit
the newest supported version; readers accept the current and explicitly
migratable older versions. Unknown fields are preserved where possible.
Unknown required components are a hard load error, never silently discarded.

```json
{
  "schema": "moonlight.scene",
  "version": 1,
  "id": "scene://demo/forest",
  "metadata": {"source_commit": "abc123", "content_sha256": "..."},
  "entities": []
}
```

`version` changes for incompatible semantics; additive fields remain
backwards-compatible. A migration is pure, deterministic, idempotent, and
tested with fixtures. Invalid input reports JSON pointer, expected type and
source location.

## Storage and lifecycle

Small manifests live in the repository. Large meshes, textures, captures,
traces, solver fields and render outputs live in the asset store and are
referenced by `sha256:<hex>` plus byte length and media type. Load states are
`Absent → Fetching → Verified → Decoded → Resident → Evicted`; checksum or
decode failure enters `Rejected` and cannot become resident without retry.

## Invariants

- IDs are stable and unique within a schema namespace.
- Hashes cover canonical bytes, not filesystem paths.
- A writer must fsync temporary output before atomic rename.
- Readers impose size, nesting and string limits before allocation.
- A migration never executes arbitrary code from a document.

## APIs and failures

```text
Result<Document, FormatError> read(path, expected_schema)
Result<Hash, StoreError> put(stream, MediaType)
Result<Bytes, StoreError> get(Hash, ByteRange?)
```

Errors distinguish `NotFound`, `HashMismatch`, `UnsupportedVersion`,
`SchemaViolation`, `PermissionDenied`, `TooLarge`, and `Corrupt`. Retries are
allowed for transport failures only; corruption is quarantined and surfaced.
See [assets and data](assets-and-data.md), [scene snapshots](scene-snapshots.md),
and [versioning](versioning.md).

## Testing

Maintain golden fixtures for every supported version, round-trip and migration
tests, malformed-input/property tests, hash verification tests, interrupted
write tests, and compatibility tests against the previous release.
