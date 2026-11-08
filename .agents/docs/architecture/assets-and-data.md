# Assets and data

**Status:** Draft

Assets are immutable, content-addressed resources. A manifest records
`hash`, `size`, `media_type`, `schema`, `source`, dependency hashes and import
settings. Paths are discovery hints only; the hash is identity.

```json
{"hash":"sha256:...","size":1048576,"media_type":"image/ktx2",
 "schema":"moonlight.asset","version":1,"dependencies":[]}
```

## Pipeline

`Source → Imported → Hashed → Verified → Decoded → Resident → Evicted`.
Import is deterministic for pinned settings and tool version. Verification
checks digest, size, schema and declared dependencies before decode. Missing
optional assets select a documented fallback; missing required assets fail the
load.

The store API is `put(stream) -> Hash`, `get(hash, range?) -> Bytes`,
`pin(hash)`, `unpin(hash)`, and `gc()`. GC may remove only unpinned entries
not referenced by a live run or snapshot. Partial downloads are resumable but
never visible as resident.

Limits prevent decompression bombs and unbounded metadata. Failed hashes are
quarantined. See [data formats](data-formats.md), [scene snapshots](scene-snapshots.md)
and [security](security.md). Test import determinism, corruption, interruption,
GC reachability, range reads and cache eviction.
