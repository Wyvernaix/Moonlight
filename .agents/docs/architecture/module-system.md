# Module system

**Status:** Accepted

A module is a directory containing `module.json`, a build definition, public
headers or Rust crate API, implementation, tests and capability documentation.
The Python CLI discovers manifests below `modules/`, validates the schema,
rejects duplicate IDs and required cycles, and exports
`build/graph/module-catalog.json`.

## Manifest contract

```json
{
  "$schema": "../../data/schemas/module/module-manifest.schema.json",
  "id": "moonlight.example",
  "version": "0.1.0",
  "language": "cpp",
  "dependencies": {"required": ["moonlight.core"], "optional": []},
  "provides": ["example.v1"],
  "platforms": ["linux", "macos", "windows"]
}
```

IDs, versions and dependency arrays are deterministic and sorted in generated
catalogs. The catalog records source path, manifest hash, target names,
capabilities and validation diagnostics; consumers must not infer fields from
directory names. The current milestone may retain a compatibility copy at
`schemas/module-manifest.schema.json` during migration, but the validator must
resolve one canonical schema ID and reject conflicting copies.

## Lifecycle

`discovered → validated → catalogued → built → loaded → started → stopped`.
The registry owns discovery; the host owns runtime transitions. A module may
expose services only through the registry and must unregister all of them at
stop. See [module lifecycle](module-lifecycle.md) and [module graph](module-graph.md).

## API and failures

Supported commands are `scripts/moonlight.py validate` and `export`; failures
return non-zero with actionable file and JSON-pointer diagnostics. Missing
manifests, schema errors, duplicate IDs, missing required dependencies and
cycles fail validation. Optional dependencies are reported as absent.

The `modules/example/` vertical slice is the reference fixture. Tests must
assert manifest/catalog parity, deterministic export, cycle detection and
startup rollback.
