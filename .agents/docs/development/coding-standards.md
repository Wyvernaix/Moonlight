# Coding standards

Follow `.editorconfig`, `.clang-format`, and `.clang-tidy`; do not invent formatting per module. C++ is C++20 with extensions disabled and warnings enabled. Prefer RAII, value semantics, narrow interfaces, `const`, and explicit error handling. Public headers must compile independently and must not expose implementation-only dependencies. Rust must pass `cargo fmt --all -- --check` and clippy with warnings denied. C# follows the solution’s `Directory.Build.props` and nullable/API conventions. Python tooling must compile with `python -m py_compile`.

Name APIs for ownership and units, avoid hidden global state, and document thread safety. Keep functions bounded and deterministic at engine boundaries. Tests should assert behavior rather than implementation details and must not depend on wall-clock timing, machine paths, or network access.

Before review:

```sh
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
python scripts/moonlight.py validate
python python/build.py --configuration debug
```

Do not suppress warnings, weaken tests, or reformat unrelated files to make a patch pass. Generated files and vendored code follow their own ownership rules. Security-sensitive changes require the security process and never include secrets in logs or fixtures.
