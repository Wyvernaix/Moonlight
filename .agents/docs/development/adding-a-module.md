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

The target validator loads `data/schemas/module/module-manifest.schema.json`,
checks manifests, then checks the dependency graph. The current milestone may
use the compatibility path `schemas/module-manifest.schema.json` during the
data-tree migration. A failure identifies malformed JSON/schema fields, an
unknown dependency, or a cycle; fix that before investigating compiler output.
If a module has no tests, document why and add contract tests at the nearest
boundary.

## API and ownership

Headers are the compatibility boundary: use stable types, explicit ownership, and no global mutable state. Prefer values or spans with documented lifetime; make allocation and threading behavior explicit. The module owns resources it creates and releases them on all failure paths. Do not retain pointers into caller-owned snapshots. Cross-module dependencies must be declared in the manifest and linked through the project’s CMake targets.

CI parity is `python python/build.py` (debug) on Linux/macOS/Windows. Do not commit generated catalog/build output. Changes to a module manifest, public API, serialization, or capability require the owning team review and migration notes.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python scripts/moonlight.py validate
python scripts/moonlight.py export --out build/graph/module-catalog.json
python python/build.py --configuration debug
```

**Expected exit condition:** every command exits 0; module validation reports `validated 1 module(s)` and a complete build ends with `Moonlight build completed successfully.`. A deliberate unsupported-platform/backend result is recorded as such, never silently treated as a pass.

| Environment | Required evidence | CI comparison |
|---|---|---|
| Ubuntu Linux | compiler, CMake/Ninja, Python, Rust, .NET versions; `build/` logs | Ubuntu job in `build.yml`/`test.yml` |
| macOS | same versions plus architecture and SDK | macOS matrix job |
| Windows PowerShell | same versions plus MSVC generator/toolset | Windows matrix job |

Reproducibility requires pinned lockfiles and `rust-toolchain.toml`, no network-dependent tests, deterministic seeds, and a clean `build/debug` or `build/release` directory. Preserve `build/**/Testing/Temporary/LastTest.log`, compiler output, test reports, catalogs, profiles, or captures as applicable. Roll back by reverting the change and deleting only generated `build/`, `target/`, `bin/`, or `obj/` outputs before rerunning; never delete source or change CI to hide a failure.

For failures, keep the first error, run the smallest command above that reproduces it, compare tool versions with CI, and inspect `LastTest.log`; classify the result as environment, deterministic test, data/ABI, or concurrency failure before editing. Static checks include `git diff --check`, Python compilation, Rust fmt/clippy, and the repository CMake/.NET checks. Add property-based, fuzz, sanitiser, or numerical-regression coverage when parsing manifests, serialized data, shaders, memory ownership, or solver math changes; document unavailable tooling and the seed/input instead of claiming coverage.

Security boundary: examples must not access production credentials, network services, customer content, or unrestricted filesystem paths. Scrub paths, tokens, dumps, and personal data from artifacts; review generated archives and logs before upload. CI parity is the corresponding workflow under `.github/workflows/`; local success does not replace the required Linux/macOS/Windows matrix.
