# Dependency and toolchain pinning

Pinning makes clean-room builds and CI comparable. The authoritative files are `rust-toolchain.toml`, `Cargo.lock`, `global.json`, `Directory.Packages.props`, CMake presets, and workflow setup actions. Do not invent a second version source in documentation.

## Inventory and clean-room check

```sh
git status --short
git diff -- Cargo.lock rust-toolchain.toml global.json Directory.Packages.props CMakePresets.json
python --version; cmake --version; ninja --version
cargo --version; rustc --version; dotnet --version
mkdir -p build
cargo metadata --locked --format-version 1 > build/dependency-metadata.json
```

The command must exit 0 and the metadata artifact must be kept under ignored `build/`. Record OS, architecture, compiler, SDK, and versions. CI's setup steps and workflow matrices are the comparison point; a locally newer tool is not proof of compatibility.

## Change procedure

1. Identify the owner and reason for every version change.
2. Change the single authoritative pin and regenerate lockfiles with the repository's package manager.
3. Review the complete dependency diff, licenses, transitive sources, and security advisories.
4. Run formatting, static analysis, tests, and a clean Debug and Release build.
5. Verify Ubuntu, macOS, and Windows or record the unavailable matrix job as a blocker.

```sh
cargo fmt --all -- --check
cargo clippy --workspace --all-targets -- -D warnings
cargo test --workspace --locked
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
python scripts/moonlight.py validate
python python/build.py --configuration debug
python python/build.py --configuration release
```

Expected exit condition is zero for every command, no unexplained lockfile change, and matching CI results. Network access is allowed only for dependency resolution in a controlled clean-room step; tests must not require network services.

## Rollback, security, and diagnosis

If a pin breaks a platform, revert the pin and lockfile together, remove only generated outputs, and rerun the previous known-good commit. Diagnose using the first resolver/compiler error and recorded versions; do not hand-edit `Cargo.lock` or bypass `--locked`. Review checksums, provenance, licenses, and advisories. Never commit credentials, private registries' tokens, or downloaded binaries. Fuzz/property tests are required when a dependency parses untrusted data; static analysis and CI parity are required for every pin change.
