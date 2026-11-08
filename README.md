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

The portable third-party dependency set is described by `vcpkg.json`. It is
installed automatically into `third_party/vcpkg` when the Workbench workflow is
used. The bootstrap is pinned to a vcpkg release and selects the host triplet
for Windows, macOS, or Linux. It does not modify system packages or require
administrator access.

To install the repository-local dependencies explicitly:

```sh
python python/bootstrap.py
```

Then set `VCPKG_ROOT` to `third_party/vcpkg` and select `debug-vcpkg`, or
select `workbench-vcpkg`, which uses the repository-local toolchain path. The
same manifest is used on Linux, macOS, and Windows.
Headless Debug and Release remain dependency-light and do not require a Vulkan
SDK or display server.

## Configure and build

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

The default CMake build includes Core, every discovered native module, and
their test executables. The explicit `moonlight_all` target is also available
when selecting the complete project from CLion or another CMake IDE. It builds
the native targets, Cargo workspace, managed solution, and runs the Python
module validation/catalog export:

```sh
cmake --build --preset debug --target moonlight_all
```

The individual cross-language targets are also available in CLion:

```text
moonlight_module_graph        Runnable Cargo module-graph executable
moonlight_rust_build          Build the Cargo workspace
moonlight_managed_build       Moonlight.sln
moonlight_validate_modules    Python module validation and catalog export
moonlight_workspace_check     Rust checks/tests, managed tests, and Python validation
```

Open the repository root in CLion, select the checked-in `Debug` or `Release`
CMake profile, and choose `moonlight_all` as the build target. The target is
enabled by default through `MOONLIGHT_BUILD_RUST`,
`MOONLIGHT_BUILD_MANAGED`, and `MOONLIGHT_VALIDATE_MODULES`; these options can
be disabled for a language-specific or dependency-light configure.

Only executable targets can be launched with CLion's Run/Debug action.
`moonlight_rust_build`, `moonlight_managed_build`,
`moonlight_validate_modules`, and `moonlight_workspace_check` are build/check
targets and should be selected with **Build Target**. The runnable targets are
`moonlight_workbench` (Workbench profile) and `moonlight_module_graph`
(Debug/Release profiles).

The interactive Workbench is an opt-in Vulkan build. It creates an SDL3/Vulkan
window and renders a basic indexed sphere from an Atlas scene through the
Spectral backend:

```sh
cmake --preset workbench
cmake --build --preset workbench --target moonlight_workbench
build/workbench/bin/moonlight_workbench
```

For a fresh checkout, use the automatic dependency path:

```sh
python python/build.py --workbench
build/workbench-vcpkg/bin/moonlight_workbench
```

The Workbench requires Vulkan headers, a Vulkan loader/driver, SDL3, and
`glslc`. These are installed by the repository-local vcpkg bootstrap. A Vulkan
runtime/driver is still required to run the Workbench and cannot be downloaded
portably by a project build; install the vendor driver through the operating
system or GPU vendor. On Linux and macOS, the bootstrap may also require
native autotools (`autoconf`, `automake`, `libtool`, and `pkg-config`) for
transitive packages and prints a package-manager-specific command if they are
missing. Arch (`pacman`), Debian/Ubuntu (`apt-get`), Fedora/RHEL (`dnf`),
openSUSE (`zypper`), Alpine (`apk`), and macOS Homebrew are recognized.
Headless Debug and Release builds do not compile or require the Vulkan backend.

On Linux, missing host utilities can optionally be installed through the
detected package manager:

```sh
python python/bootstrap.py --install-host-tools
```

This is opt-in because it may invoke `sudo`; the normal bootstrap never changes
the operating system.

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

The orchestrator also supports developer-focused workflows:

```sh
# Preview every command without executing it
python python/build.py --dry-run

# Limit native CMake parallelism
python python/build.py --jobs 4

# Configure and build the Vulkan Workbench only
python python/build.py --workbench --skip native
```

The equivalent `just` recipes are `just plan` and `just workbench`. The
Workbench recipe intentionally skips the headless native test stage; use
`just build` for the complete validation pipeline.

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
| `data/schemas/` | Canonical versioned schemas |
| `data/config/` | Cross-platform engine and feature profiles |
| `data/registry/` | Declarative registry inputs |
| `.agents/docs/` | Authored architecture and engineering documentation |
| `CMakeLists.txt` | Native build entry point |
| `Cargo.toml` | Rust workspace entry point |
| `src/native/` | C++ runtime and stable C ABI |
| `src/managed/` | .NET SDK and interop projects |
| `tools/` | Build, validation, graph, shader, and release tools |
| `python/build.py` | Python orchestration for the complete multi-language build |
| `scripts/moonlight.py` | Module discovery, manifest validation, and catalog CLI |
| `data/schemas/module/module-manifest.schema.json` | Canonical module schema |
| `apps/` | User-facing applications and command-line tools |
| `research/` | Reproducible experiment protocols and manifests |

Graphics-facing modules and applications are design targets. The current
foundation deliberately builds headlessly; enabling the optional vcpkg
dependencies is the supported path for SDL3/Vulkan and future graphics
vertical slices.

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