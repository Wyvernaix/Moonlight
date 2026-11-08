# Moonlight Engine

Moonlight is an engine workspace for deterministic, cross-platform rendering and
simulation systems. The repository is currently in foundation work: the build
and quality gates are defined before production modules are introduced.

## Status

This project is not production-ready yet. The repository now includes the
buildable foundation: native Core lifecycle contracts, managed SDK projects,
schema-backed module manifests, and a Rust dependency-graph validator/exporter.
Production modules are still introduced incrementally behind those contracts.

## Prerequisites

- CMake 3.28 or newer
- A C++20 compiler
- Rust stable (the repository toolchain file is authoritative)
- .NET SDK 10.0 or newer (the `global.json` file selects the SDK)
- Python 3.11 or newer
- Ninja
- Git

The checked-in build workflows exercise Linux, macOS, and Windows. The native
targets use CMake/Ninja, the managed targets use the .NET CLI, and Rust targets
use Cargo; none of these build systems depends on a specific operating system.

Optional tooling such as `clang-format`, `clang-tidy`, and `pre-commit` is
recommended for local quality checks.

## Configure and build

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The `release` preset is available for optimized local and CI builds:

```sh
cmake --preset release
cmake --build --preset release
```

Rust workspace checks can be run with:

```sh
cargo check --workspace
cargo test --workspace
```

The managed SDK can be built with:

```sh
dotnet build Moonlight.sln --configuration Release
```

The complete current project can be built and tested through the Python
orchestrator:

```sh
python python/build.py
python python/build.py --configuration release
```

It validates and exports the module graph, builds/tests C++ with CMake, checks
and tests Rust with Cargo, and restores/builds/tests C# with the .NET CLI.
Individual stages can be skipped with `--skip native`, `--skip rust`,
`--skip managed`, or `--skip modules`. `just build` and `just validate` call the
same orchestrator when `just` is installed.

Registered modules live under `modules/<id>/`. Validate and inspect them with:

```sh
python scripts/moonlight.py list
python scripts/moonlight.py validate
python scripts/moonlight.py export --out build/graph/module-catalog.json
```

To install the repository's Python tooling in a local virtual environment:

```sh
python/get_python.sh
```

On Windows, run `python\get_python.bat`. The matching `python/pip.sh` and
`python/pip.cmd` wrappers use that environment without requiring activation.

## Repository layout

| Path | Purpose |
| --- | --- |
| `modules/` | Engine modules and their public documentation |
| `schemas/` | Versioned JSON schemas for project data |
| `docs/` | Generated and authored project documentation |
| `CMakeLists.txt` | Native build entry point |
| `Cargo.toml` | Rust workspace entry point |
| `src/native/` | C++ runtime and stable C ABI |
| `src/managed/` | .NET SDK and interop projects |
| `tools/` | Build, validation, graph, and release tools |
| `python/build.py` | Python orchestration for the complete multi-language build |
| `scripts/moonlight.py` | Module discovery, manifest validation, and catalog CLI |
| `schemas/module-manifest.schema.json` | Schema contract for registered modules |
| `apps/` | User-facing applications and command-line tools |
| `research/` | Reproducible experiment protocols and manifests |

The native Vulkan GUI vertical slice is under `apps/native-gui/`. Use the
`gui` CMake preset with `VCPKG_ROOT` configured to build the ImGui editor and
project manager. Core and Project System remain independently buildable in
headless CI.

CLion users can open the repository root and select the checked-in **Debug** or
**Release** CMake profile. The shared run configurations include
**Moonlight Core Tests**, **Moonlight Build Complete**, and
**Moonlight Build Release**. The latter two invoke `python/build.py`, so they
build and validate the native, Rust, managed, and module-graph stages together.

## Engineering standards

Changes should be small, reviewable, tested, and documented when they alter a
public contract. Run the relevant CMake, Rust, formatting, and static-analysis
checks before opening a pull request. Never commit generated build output,
credentials, or local IDE state.

See the architecture specification and module-system guide, plus the
contribution, security, and code-of-conduct documents in `.github/` for
the project governance requirements.