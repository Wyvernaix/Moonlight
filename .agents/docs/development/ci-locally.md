# CI locally

CI is split into build, test, validation, PR, security, and release workflows under `.github/workflows/`. The closest local PR equivalent is:

```sh
python python/build.py
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
python scripts/moonlight.py export --out build/graph/module-catalog.json
```

To match the matrix, repeat the relevant command on clean Linux, macOS, and Windows environments. Build workflow uses Release; PR workflow uses Debug. Native test workflow runs CMake/Ninja and CTest, Rust uses `cargo test --workspace --locked`, and managed tests use `dotnet test Moonlight.sln --configuration Release`.

Validation also checks the exact workflow inventory. Do not add a workflow casually: update the inventory check and obtain ownership approval. GitHub Actions versions and tool versions are authoritative; local tool versions should be recorded when results differ. A green local run does not replace a required matrix job. Report unavailable OS/backend coverage explicitly rather than marking it passed.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python python/build.py --configuration debug
python scripts/moonlight.py export --out build/graph/module-catalog.json
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
