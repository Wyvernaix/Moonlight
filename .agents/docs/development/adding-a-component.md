# Adding a component

Components are data contracts, not arbitrary bags of engine state. Before implementation, write down the stable identifier, semantic version, units, defaults, required/optional fields, serialization representation, and compatibility policy. Search existing component definitions and tests so identifiers are not duplicated.

## Implementation sequence

1. Define the immutable public schema and validation rules.
2. Add registration in the owning module, with a deterministic identifier.
3. Implement serialization/deserialization with explicit version handling.
4. Add round-trip, missing-field, invalid-value, and old-version migration tests.
5. Export a read-only snapshot for systems that consume the component.
6. Run module validation and the complete build.

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
```

## Ownership and lifetime

Atlas owns component storage. Systems may request extraction, but must consume immutable snapshots and must not mutate Atlas memory or retain references past the documented snapshot lifetime. The creator owns handles/resources; destruction is idempotent and occurs on the owning thread unless the component explicitly documents otherwise. Serialization must not invoke user callbacks, access render/audio devices, or perform unbounded allocation.

Validate at the boundary: reject wrong units, NaN/infinite values, invalid enum values, and incompatible versions with an actionable error. Preserve unknown fields when forward compatibility is promised; otherwise fail closed and document the policy. Never silently reinterpret a field or change defaults without a version/migration note.

## Contract tests

A component is complete only when tests prove deterministic defaults, canonical serialization, round-trip equality, invalid input rejection, migration behavior, and snapshot lifetime. Test on all CI platforms where binary layout, path handling, or endianness could matter. Include the exact command and result in the change description; generated fixtures belong in ignored build output unless they are intentional small test assets.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python scripts/moonlight.py validate
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
