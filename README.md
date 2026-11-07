# Moonlight Engine

Moonlight is an engine workspace for deterministic, cross-platform rendering and
simulation systems. The repository is currently in foundation work: the build
and quality gates are defined before production modules are introduced.

## Status

This project is not production-ready yet. The root configuration is kept
buildable and reproducible so new modules can be added without changing the
development contract.

## Prerequisites

- CMake 3.28 or newer
- A C++23 compiler
- Rust stable (the repository toolchain file is authoritative)
- Git

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

## Repository layout

| Path | Purpose |
| --- | --- |
| `modules/` | Engine modules and their public documentation |
| `schemas/` | Versioned JSON schemas for project data |
| `docs/` | Generated and authored project documentation |
| `CMakeLists.txt` | Native build entry point |
| `Cargo.toml` | Rust workspace entry point |

## Engineering standards

Changes should be small, reviewable, tested, and documented when they alter a
public contract. Run the relevant CMake, Rust, formatting, and static-analysis
checks before opening a pull request. Never commit generated build output,
credentials, or local IDE state.

See the contribution, security, and code-of-conduct documents in `.github/` for
the project governance requirements.