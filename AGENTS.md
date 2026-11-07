# Moonlight contributor guidance

## Scope

Treat this repository as a cross-platform C++ and Rust workspace. Prefer
existing CMake presets and the repository-pinned Rust toolchain over ad-hoc
commands.

## Required checks

Before submitting a change, run the smallest applicable checks:

```sh
cmake --preset debug
cmake --build --preset debug
cargo check --workspace
```

Keep public interfaces documented, preserve deterministic behavior, and avoid
adding dependencies without recording why they are needed. Do not commit build
directories, IDE metadata, secrets, or generated files.