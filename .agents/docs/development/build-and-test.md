# Build and test

The supported entry point is the complete, CI-parity build:

```sh
python python/build.py                  # debug (PR default)
python python/build.py --configuration release
python python/build.py --help
```

The script validates modules, configures/builds/tests native code, checks Rust formatting/clippy/check/tests, then restores/builds/tests `Moonlight.sln`. Use `--skip native`, `--skip rust`, `--skip managed`, or `--skip modules` only when isolating a known failure.

## Individual stages

Native (CMake 3.28+, Ninja):

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
# substitute release for a production-style build
```

CMake writes to `build/debug` or `build/release`; never configure with a different generator in those directories. Rust:

```sh
cargo fmt --all -- --check
cargo clippy --workspace --all-targets -- -D warnings
cargo check --workspace --locked
cargo test --workspace --locked
```

Managed:

```sh
dotnet restore Moonlight.sln
dotnet build Moonlight.sln --no-restore --configuration Debug
dotnet test Moonlight.sln --no-build --configuration Debug
```

Use `Release` instead of `Debug` when matching `--configuration release`. Windows commands are identical in PowerShell; quote paths containing spaces. Linux/macOS may need `./` for local scripts.

## Expected output and diagnosis

A healthy complete run ends with `Moonlight build completed successfully.`; native CTest reports all tests passed, Cargo reports tests passed, and .NET exits 0. A missing executable is an environment failure; a non-zero stage exit is a code/configuration failure. Re-run only the failing stage with its command, preserve the first compiler/test error, and inspect `build/<preset>/Testing/Temporary/LastTest.log` for CTest. Delete a stale build directory only after recording the failure (`rm -rf build/debug` or `Remove-Item -Recurse build/debug`) and reconfigure.

## CI parity

PR CI runs the complete debug build on Ubuntu, macOS, and Windows. Scheduled tests run native, Rust, and managed tests independently in Debug/Release-equivalent modes; validation runs Python/module checks on Ubuntu. Reproduce the same command before claiming a fix and report OS, compiler/SDK versions, preset, and exact command.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
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
