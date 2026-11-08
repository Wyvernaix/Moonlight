# Troubleshooting

Start at the repository root and capture the first error:

```sh
python python/build.py --help
python scripts/moonlight.py validate
python python/build.py --skip native --skip rust --skip managed
```

The last command isolates module validation/export. Then run the failing stage directly (see `build-and-test.md`). Verify versions and `PATH` before changing code. CI uses Python 3.11, Rust stable, .NET 10.0.x, CMake/Ninja, and clean runners.

| Symptom | Likely cause | Action |
|---|---|---|
| tool not found | missing SDK or PATH | install the matching tool; restart shell/IDE |
| manifest/schema error | malformed or incomplete module.json | compare `modules/example`, validate JSON, declare dependencies |
| CMake generator/cache error | stale build directory or wrong generator | remove only `build/debug` or `build/release`, re-run preset |
| CTest failure | test assertion/environment | rerun `ctest --preset debug --output-on-failure`; inspect `LastTest.log` |
| Cargo locked/dependency failure | lockfile/toolchain mismatch | use stable and `--locked`; do not hand-edit Cargo.lock casually |
| .NET restore/build failure | wrong SDK or stale obj/bin | verify `dotnet --version`, then clean/rebuild the solution |
| platform-only failure | path, compiler, backend, or line-ending assumption | reproduce on the affected OS and document the difference |

Generated output belongs in ignored `build/`, `target/`, `bin/`, and `obj/`. Do not delete source or alter CI to hide a failure. Preserve logs, command, commit, OS, and tool versions. Escalate ownership/lifetime, security, data-loss, or release-signing failures immediately; do not bypass validation.
