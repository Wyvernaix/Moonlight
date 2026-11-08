# Documentation workflow

Development documentation lives under `.agents/docs/development/` and must describe the repository as it exists, not an aspirational toolchain. When commands, presets, manifests, CI, ownership, or compatibility rules change, update the relevant page in the same change. Prefer executable commands, expected output, platform notes, and failure diagnosis over vague guidance.

For a documentation change:

```sh
find .agents/docs/development -name '*.md' -print | sort
git diff --check
python scripts/moonlight.py validate
```

Run build/test commands when documentation claims they work; documentation-only edits need no full rebuild if the commands are unchanged, but state what was not run. Verify links and paths against the repository. Do not edit generated docs, files outside the requested documentation directory, or CI files merely to make an example pass.

Every page should identify ownership/lifetime and safety implications where relevant. Avoid secrets, customer data, unsupported promises, and commands that delete broad paths. Keep examples copy/pasteable on Linux/macOS/Windows or label shell-specific syntax. Reviewers should be able to reproduce the documented result from a clean checkout.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
git diff --check
python scripts/moonlight.py validate
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
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
