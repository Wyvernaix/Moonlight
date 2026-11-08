# Adding a module

A module is a separately discoverable unit under `modules/<id>`. Start by checking existing conventions:

```sh
find modules -maxdepth 2 -type f | sort
python scripts/moonlight.py list
```

Use a lowercase dotted identifier (for example `render.pathtracer`) and create `module.json`, `CMakeLists.txt`, public headers under `include/moonlight/<name>/`, implementation under `src/`, tests under `tests/`, and a README. Declare every dependency and list only implemented languages (`cpp`, `rust`, or both). Keep the manifest version semver-compatible; expose capabilities explicitly and avoid undocumented implicit coupling.

A minimal workflow is:

```sh
mkdir -p modules/<id>/{include/moonlight/<name>,src,tests}
# add module.json and CMakeLists.txt following modules/example
python scripts/moonlight.py validate
python scripts/moonlight.py export --out build/graph/module-catalog.json
cmake --preset debug && cmake --build --preset debug
ctest --preset debug --output-on-failure
```

The validator loads `schemas/module-manifest.schema.json`, checks manifests, then checks the dependency graph. A failure identifies either malformed JSON/schema fields, an unknown dependency, or a cycle; fix that before investigating compiler output. If a module has no tests, document why and add contract tests at the nearest boundary.

## API and ownership

Headers are the compatibility boundary: use stable types, explicit ownership, and no global mutable state. Prefer values or spans with documented lifetime; make allocation and threading behavior explicit. The module owns resources it creates and releases them on all failure paths. Do not retain pointers into caller-owned snapshots. Cross-module dependencies must be declared in the manifest and linked through the project’s CMake targets.

CI parity is `python python/build.py` (debug) on Linux/macOS/Windows. Do not commit generated catalog/build output. Changes to a module manifest, public API, serialization, or capability require the owning team review and migration notes.
