# Release packaging

Releases are tag-driven by `.github/workflows/release.yml`; tags must match `v*.*.*`. The workflow builds Release on Ubuntu, macOS, and Windows, archives the checked-out source with `git archive`, uploads one artifact per OS, and publishes a GitHub release for version tags.

Before tagging, run:

```sh
git status --short
python python/build.py --configuration release
cargo test --workspace --locked
dotnet test Moonlight.sln --configuration Release
ctest --preset release --output-on-failure
```

Verify the changelog, module/API compatibility, supported-platform notes, and that no generated output or secret is tracked. Use an annotated tag only after owner approval; never rewrite a published tag. The release artifact is source packaging, not an installer—do not describe it as a binary distribution unless packaging is added and documented.

For a local archive check:

```sh
mkdir -p build/artifacts
git archive --format=tar.gz --output build/artifacts/moonlight-local.tar.gz HEAD
tar -tzf build/artifacts/moonlight-local.tar.gz | head
```

On Windows use `New-Item -ItemType Directory build/artifacts` and `tar -tf`. Confirm archives contain the intended commit and no credentials. Release failures are owned by release/CI maintainers; do not bypass failed platform jobs.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python python/build.py --configuration release
ctest --preset release --output-on-failure
cargo test --workspace --locked
dotnet test Moonlight.sln --configuration Release
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
