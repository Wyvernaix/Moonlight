# Coding standards

Follow `.editorconfig`, `.clang-format`, and `.clang-tidy`; do not invent formatting per module. C++ is C++20 with extensions disabled and warnings enabled. Prefer RAII, value semantics, narrow interfaces, `const`, and explicit error handling. Public headers must compile independently and must not expose implementation-only dependencies. Rust must pass `cargo fmt --all -- --check` and clippy with warnings denied. C# follows the solution’s `Directory.Build.props` and nullable/API conventions. Python tooling must compile with `python -m py_compile`.

Name APIs for ownership and units, avoid hidden global state, and document thread safety. Keep functions bounded and deterministic at engine boundaries. Tests should assert behavior rather than implementation details and must not depend on wall-clock timing, machine paths, or network access.

Before review:

```sh
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
python scripts/moonlight.py validate
python python/build.py --configuration debug
```

Do not suppress warnings, weaken tests, or reformat unrelated files to make a patch pass. Generated files and vendored code follow their own ownership rules. Security-sensitive changes require the security process and never include secrets in logs or fixtures.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
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
