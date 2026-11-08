# Bootstrap

This page is the reproducible first checkout procedure for Moonlight. Run commands from the repository root; do not put generated files in source directories.

## Required toolchain

| Tool | Required version/role |
|---|---|
| Git | checkout and release archives |
| Python | 3.11+ (CI uses 3.11) |
| CMake | 3.28+ |
| Ninja | native generator |
| C++ compiler | C++20; GCC/Clang on Linux/macOS, MSVC on Windows |
| Rust | stable plus `rustfmt` and `clippy` |
| .NET SDK | 10.0.x |

Confirm the tools before changing code:

```sh
python --version; cmake --version; ninja --version
cargo --version; rustc --version; dotnet --version
```

On Windows use `py --version` if `python` is not registered, and run commands in Developer PowerShell (or Git Bash for the shell examples). On macOS install Xcode Command Line Tools (`xcode-select --install`) and use Homebrew CMake/Ninja if the system versions are too old. On Linux install the distribution C++ build-essential package, CMake, Ninja, Python 3.11, Rustup, and .NET 10; do not mix an old system `dotnet` with the CI SDK.

## Python environment

The repository supplies platform bootstrap scripts. On Linux/macOS:

```sh
./python/get_python.sh
```

On Windows:

```bat
python\get_python.bat
```

These scripts are optional for the build if a compatible Python is already on `PATH`; inspect `python/build.py` before changing their behavior. The build uses the standard library, while `python/requirements.txt` is the place for any future tooling dependencies.

## First checkout

```sh
git submodule update --init --recursive
python scripts/moonlight.py validate
python python/build.py --configuration debug
```

Expected validation includes `validated 1 module(s)` in the current tree and the final build line `Moonlight build completed successfully.`. If a command is absent, fix `PATH` or install the matching prerequisite rather than committing generated binaries.

## Ownership and safety

Read `CODEOWNERS` and the relevant module README before editing. Never commit credentials, local SDK paths, `build/`, `target/`, `bin/`, or `obj/` outputs. Keep changes scoped to an owned module and preserve public API/manifest compatibility. Stop and ask the owning team when a change crosses a module boundary or changes serialized data.
