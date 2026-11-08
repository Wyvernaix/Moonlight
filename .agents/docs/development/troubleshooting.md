# Troubleshooting

Start at the repository root and capture the first error:

```sh
python python/build.py --help
python scripts/moonlight.py validate
python python/build.py --skip native --skip rust --skip managed
```

The last command isolates module validation/export. Then run the failing stage directly (see `build-and-test.md`). Verify versions and `PATH` before changing code. CI uses Python 3.11, Rust stable, .NET 10.0.x, CMake/Ninja, and clean runners.

| Symptom | Likely cause | Action |
|---|---|---|
| tool not found | missing SDK or PATH | install the matching tool; restart shell/IDE |
| manifest/schema error | malformed or incomplete module.json | compare `modules/example`, validate JSON, declare dependencies |
| CMake generator/cache error | stale build directory or wrong generator | remove only `build/debug` or `build/release`, re-run preset |
| CTest failure | test assertion/environment | rerun `ctest --preset debug --output-on-failure`; inspect `LastTest.log` |
| Cargo locked/dependency failure | lockfile/toolchain mismatch | use stable and `--locked`; do not hand-edit Cargo.lock casually |
| .NET restore/build failure | wrong SDK or stale obj/bin | verify `dotnet --version`, then clean/rebuild the solution |
| platform-only failure | path, compiler, backend, or line-ending assumption | reproduce on the affected OS and document the difference |

Generated output belongs in ignored `build/`, `target/`, `bin/`, and `obj/`. Do not delete source or alter CI to hide a failure. Preserve logs, command, commit, OS, and tool versions. Escalate ownership/lifetime, security, data-loss, or release-signing failures immediately; do not bypass validation.

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
