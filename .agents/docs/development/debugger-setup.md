# Debugger setup

Checked-in IDE run configurations provide the complete Python build, native Debug/Release builds, and core tests. Select the `Debug` CMake preset for symbols and diagnostics; use Release only when reproducing an optimization-sensitive issue.

## Command-line debugging

Configure and build first:

```sh
cmake --preset debug
cmake --build --preset debug
```

Linux: launch `gdb --args build/debug/moonlight_core_tests` and use `run`, `bt`, `thread apply all bt`, and `watch`. macOS: use `lldb -- build/debug/moonlight_core_tests`, then `run` and `thread backtrace all`. Windows: open the CMake target in Visual Studio/CLion or use `devenv`/VS Code with the MSVC debugger; do not mix a MinGW binary with an MSVC debugger configuration.

Set breakpoints at the first failing assertion and at module boundaries. Capture the full backtrace, thread name, preset, compiler, and input. For managed code use the IDE .NET debugger or `dotnet test --blame` when a test hangs; for Rust use `rust-gdb`/`rust-lldb` after `cargo test` identifies the test.

## Debugging rules

Reproduce with one deterministic input, disable unrelated optimizations only when necessary, and never “fix” a race by adding sleeps. Do not inspect or print secrets, raw credentials, or user content in logs. Verify ownership and thread affinity before changing lifetime code. After a fix, rerun the focused test and then `python python/build.py --configuration debug`; remove temporary breakpoints and generated traces.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python python/build.py --configuration debug
ctest --preset debug --output-on-failure
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
