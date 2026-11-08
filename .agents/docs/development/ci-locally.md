# CI locally

CI is split into build, test, validation, PR, security, and release workflows under `.github/workflows/`. The closest local PR equivalent is:

```sh
python python/build.py
python -m py_compile python/build.py scripts/moonlight.py scripts/moonlight/*.py
python scripts/moonlight.py export --out build/graph/module-catalog.json
```

To match the matrix, repeat the relevant command on clean Linux, macOS, and Windows environments. Build workflow uses Release; PR workflow uses Debug. Native test workflow runs CMake/Ninja and CTest, Rust uses `cargo test --workspace --locked`, and managed tests use `dotnet test Moonlight.sln --configuration Release`.

Validation also checks the exact workflow inventory. Do not add a workflow casually: update the inventory check and obtain ownership approval. GitHub Actions versions and tool versions are authoritative; local tool versions should be recorded when results differ. A green local run does not replace a required matrix job. Report unavailable OS/backend coverage explicitly rather than marking it passed.
