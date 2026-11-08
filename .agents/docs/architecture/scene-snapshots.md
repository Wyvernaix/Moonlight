# Scene snapshots

**Status:** Accepted

Atlas owns mutable authoring state. Extraction produces an immutable,
reference-counted snapshot consumed by renderer, audio, physics and research
systems; consumers never retain pointers into authoring storage.

## Schema and API

```json
{
  "schema": "moonlight.scene.snapshot",
  "version": 1,
  "snapshot_id": "uuid",
  "source_revision": 42,
  "entities": [{"id": "uuid", "components": {"transform": {}}}],
  "assets": [{"hash": "sha256:...", "kind": "mesh"}]
}
```

```text
SnapshotHandle extract(AuthoringRevision, ExtractOptions)
Result<SnapshotView, Error> acquire(SnapshotHandle)
void release(SnapshotHandle)
```

Stable entity IDs survive extraction; `source_revision` and content hashes make
the result reproducible. Extraction is deterministic for the same authoring
revision and options. A snapshot is valid until release and may be read
concurrently.

## Lifecycle and invariants

`Authoring → Extracting → Sealed → Published → Retired → Released`. Publishing
is atomic: a consumer sees either the previous or complete snapshot. A failed
extract does not replace the current one. Components are self-describing and
unknown required components fail acquisition; optional components are marked
unavailable.

Tests cover concurrent readers, revision races, stable IDs, failed extraction,
hash parity, lifetime under pressure and schema migration. See [data formats](data-formats.md)
and [memory ownership](memory-ownership.md).
