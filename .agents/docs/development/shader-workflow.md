# Shader workflow

Shaders are backend inputs and must be treated as versioned interfaces. Keep source, entry-point names, resource bindings, coordinate conventions, precision assumptions, and supported feature levels documented with the owning render module. Generated binaries/intermediates stay in ignored build directories.

Before submitting a shader change:

```sh
python scripts/moonlight.py validate
python python/build.py --skip managed --skip rust
```

The repository currently has no dedicated shader compiler command in `python/build.py`; use the backend/compiler command required by the target platform and record it in the module README. A missing compiler or unavailable backend is an infrastructure/coverage result, not a passing shader test. Do not check in machine-specific caches.

Validate compilation for every supported backend, reflection/binding compatibility, and a deterministic image or numerical fixture. Test bounds checks, invalid resources, precision-sensitive inputs, and fallback paths. Compare output to the reference renderer with a documented tolerance; explain intentional approximations.

Linux commonly uses Vulkan/GL toolchains; macOS may require Metal tooling from Xcode; Windows may use the DirectX shader compiler/SDK. Keep platform-specific source isolated and make unsupported backends report a clear capability error. Never execute untrusted shader source during tests without sandboxing. Ownership includes resource lifetime, descriptor layouts, synchronization, and cleanup on compilation failure.

## Verification contract

Run the following from a clean checkout at the repository root; record the commit, OS, architecture, tool versions, command, exit code, and relevant artifact paths in the change or incident record:

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
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
