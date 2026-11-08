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
