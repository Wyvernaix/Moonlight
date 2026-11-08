# Bootstrap

This page is the reproducible first checkout procedure for Moonlight. Run commands from the repository root; do not put generated files in source directories.

## Required host toolchain

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

The host compiler, Git, Python, CMake, Ninja, and the platform's native build
utilities must already be installed;
the project cannot safely install a compiler or OS-level SDK without
administrator privileges. On Windows use Developer PowerShell, on macOS run
`xcode-select --install`, and on Linux install the distribution C++ toolchain.
The repository bootstrap installs the project-level CMake dependencies after
these host tools exist.

## Project dependency bootstrap

From the repository root, run:

```sh
python python/bootstrap.py
```

This downloads the pinned vcpkg release into `third_party/vcpkg`, bootstraps
vcpkg locally, and installs the manifest dependencies for the detected
Windows, macOS, or Linux architecture. The Workbench command performs this
automatically:

```sh
python python/build.py --workbench
```

The Vulkan runtime/driver remains an OS or GPU-vendor responsibility and is
not installed by vcpkg. On Linux and macOS, vcpkg may also require the native
autotools utilities (`autoconf`, `automake`, `libtool`, and `pkg-config`) for
transitive packages. Bootstrap recognizes `pacman`, `apt-get`, `dnf`,
`zypper`, `apk`, and Homebrew, and reports the appropriate install command.
Linux users may explicitly allow package installation with:

```sh
python python/bootstrap.py --install-host-tools
```

The default bootstrap never invokes a system package manager.

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
