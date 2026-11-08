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
