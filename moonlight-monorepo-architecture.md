# Moonlight Monorepo — Modular Engine Architecture

**Version:** 2.0
**Date:** 2026-11-08
**Status:** Proposed target architecture; implementation status is tracked per
directory and module, not inferred from the tree.

This document defines the repository shape and the boundaries that make
Moonlight buildable on Linux, macOS, and Windows. It is inspired by the Open 3D
Engine (O3DE) separation between engine code, extensible packages, project
content, registries, tools, and build orchestration. Moonlight deliberately
uses **modules** instead of O3DE Gems and uses a single `data/` hierarchy for
schemas, configuration, registries, templates, and other versioned engine data.

The tree is a contract for ownership and dependency direction. A directory is
not an implemented feature merely because it appears below. A module becomes
available only after it has a valid manifest, build target, API, tests,
documentation, and declared capabilities.

## 1. Architectural principles

1. One repository contains the engine runtime, first-party modules, managed
   applications, tools, project data, tests, and reproducible research.
2. C++ owns performance-critical runtime and platform-facing systems. Rust is
   used for isolated tools, graph/data processing, and other components where
   memory safety or parallel data processing justifies a stable FFI boundary.
   C# owns the Workbench, Editor, scripting runtime, managed SDK, and managed
   tools. Python owns orchestration and repository automation.
3. CMake/Ninja builds native code, Cargo builds Rust, and the .NET SDK builds
   managed code. `python/build.py` and the root task runner coordinate them but
   do not replace them.
4. `src/` contains foundational runtime code only. Independent engine features
   belong in `modules/`; applications belong in `apps/`; developer tools belong
   in `tools/`; versioned non-code inputs belong in `data/`.
5. Atlas owns authoritative scene identity, ECS storage, hierarchy, transforms,
   serialization, prefab composition, and immutable snapshots. Other systems
   consume snapshots or explicit extraction products.
6. Spectral separates reference transport from Voxel GI, caches, coherent
   wave transport, and Maxwell solvers. Every approximation declares its valid
   regime and measured error.
7. Orcha keeps offline/async acoustic simulation away from the real-time audio
   callback. Audio output consumes immutable or lock-free-ready results.
8. Required runtime dependencies form an acyclic graph. Build dependencies,
   runtime dependencies, capability providers, and optional integrations are
   represented as separate graph layers.
9. Stable cross-language/plugin boundaries use a versioned C ABI with explicit
   ownership, structure sizes, error codes, thread affinity, and feature
   negotiation. C++ and Rust object layouts never cross that boundary.
10. Generated output, caches, binaries, downloaded datasets, and local
    credentials never belong in source-owned directories.

## 2. Canonical repository architecture

The following is the canonical target tree. Braced entries are documented
families, not literal directory names. A directory marked `generated/`,
`build/`, `out/`, or `cache/` is disposable unless its containing document
explicitly says otherwise.

```text
moonlight/
├── README.md
├── CONTRIBUTING.md
├── SECURITY.md
├── CODE_OF_CONDUCT.md
├── LICENSE
├── THIRD_PARTY_NOTICES.md
├── CHANGELOG.md
├── CMakeLists.txt
├── CMakePresets.json
├── CMakeUserPresets.json.example
├── Cargo.toml
├── Cargo.lock
├── rust-toolchain.toml
├── Moonlight.sln
├── global.json
├── Directory.Build.props
├── Directory.Build.targets
├── Directory.Packages.props
├── justfile
├── .editorconfig
├── .gitattributes
├── .gitignore
├── .clang-format
├── .clang-tidy
├── .pre-commit-config.yaml
│
├── cmake/                              # Build logic, never runtime data
│   ├── MoonlightOptions.cmake
│   ├── MoonlightWarnings.cmake
│   ├── MoonlightSanitizers.cmake
│   ├── MoonlightDependencies.cmake
│   ├── MoonlightModule.cmake
│   ├── MoonlightModules.cmake
│   ├── MoonlightCodegen.cmake
│   ├── MoonlightInstall.cmake
│   ├── MoonlightTesting.cmake
│   └── toolchains/{linux-clang,linux-gcc,macos-clang,windows-msvc}.cmake
│
├── data/                               # Versioned engine/project data
│   ├── schemas/                         # JSON/YAML schemas and schema index
│   │   ├── module/module-manifest.schema.json
│   │   ├── graph/{module-catalog,module-graph}.schema.json
│   │   ├── scene/{scene,prefab,component}.schema.json
│   │   ├── assets/{asset-manifest,import-settings}.schema.json
│   │   ├── rendering/{render-profile,material,light,camera}.schema.json
│   │   ├── audio/{acoustic-material,audio-profile}.schema.json
│   │   ├── research/{experiment,run,result}.schema.json
│   │   ├── schema-index.json
│   │   └── migrations/{v1-to-v2}/
│   ├── config/                          # Human-authored defaults and profiles
│   │   ├── engine/default.toml
│   │   ├── logging/default.toml
│   │   ├── rendering/{interactive,production,reference,research}.toml
│   │   ├── audio/{realtime,acoustic-lab}.toml
│   │   ├── platform/{linux,macos,windows,headless}.toml
│   │   └── profiles/{development,production,headless-ci}.toml
│   ├── registry/                        # Declarative registration/override data
│   │   ├── engine-modules.json
│   │   ├── capabilities.json
│   │   ├── component-types.json
│   │   ├── platform-features/{linux,macos,windows}.json
│   │   └── migrations/
│   ├── defaults/                        # Small safe fallback values only
│   ├── templates/                       # Project/module/asset templates
│   │   ├── project/
│   │   ├── module/{cpp,rust,mixed}/
│   │   ├── component/
│   │   └── experiment/
│   ├── presets/                         # Named rendering/audio/tool presets
│   ├── shaders/                         # Shared shader source and includes
│   │   ├── common/
│   │   ├── vulkan/
│   │   ├── dx12/
│   │   └── generated/                   # Never hand-edit
│   ├── builtin-assets/                  # Small licensed engine assets only
│   └── licenses/                        # Data/license manifests
│
├── src/                                 # Foundational runtime, not feature packs
│   ├── native/
│   │   ├── core/
│   │   │   ├── include/moonlight/core/
│   │   │   │   ├── runtime/ modules/ services/ capabilities/
│   │   │   │   ├── jobs/ resources/ events/ logging/ memory/
│   │   │   │   ├── platform/ diagnostics/ public_api/
│   │   │   └── src/{runtime,modules,services,capabilities,jobs,resources,
│   │   │              events,logging,memory,platform,diagnostics}/
│   │   ├── public-api/include/moonlight/
│   │   │   ├── module_api.h
│   │   │   ├── service_api.h
│   │   │   └── abi_version.h
│   │   └── generated/                   # ABI/codegen output
│   ├── managed/
│   │   ├── Moonlight.Interop/
│   │   ├── Moonlight.ManagedSDK/
│   │   ├── Moonlight.Diagnostics/
│   │   └── Moonlight.Scripting/
│   └── shaders/                         # Runtime-owned shader interfaces
│       ├── include/
│       └── generated/
│
├── modules/                             # O3DE-Gem-like extensibility boundary
│   ├── example/                         # First validated C++ module
│   ├── atlas/                           # Scene graph and ECS
│   ├── flywheel/                        # Animation and pose evaluation
│   ├── physics/                         # Physics services
│   ├── graphics/                        # RHI and graphics services
│   ├── assets/                          # Asset database/import/streaming
│   ├── input/
│   ├── scripting/
│   ├── networking/
│   ├── ui/
│   ├── telemetry/
│   ├── orcha/                           # Acoustic/audio systems
│   └── spectral/                        # Optical/rendering systems
│       ├── foundation/
│       ├── data/
│       ├── materials/
│       ├── lighting/
│       ├── transport/path-tracing/
│       ├── gi/voxel-gi/
│       ├── optics/{interfaces,polarization,wave-optics,thin-films,diffraction}
│       ├── volumes/
│       ├── solvers/maxwell/
│       ├── imaging/{camera,sensors,colour-science}
│       ├── backends/{cpu,vulkan,dx12,metal}
│       └── diagnostics/
│
├── apps/                                # User-facing executables and hosts
│   ├── workbench/                       # C# Editor/Workbench
│   │   ├── src/{Workbench.App,Workbench.Shell,Workbench.Interop,
│   │   │        Workbench.ProjectSystem,Workbench.SceneEditor,
│   │   │        Workbench.Inspector,Workbench.AssetBrowser,
│   │   │        Workbench.RenderView,Workbench.AudioEditor,
│   │   │        Workbench.AnimationEditor,Workbench.Profilers}
│   │   ├── tests/
│   │   └── docs/
│   ├── player/
│   ├── headless-runner/
│   ├── asset-compiler/
│   ├── render-cli/
│   ├── audio-bake-cli/
│   └── module-inspector/
│
├── tools/                               # Build/developer tools, not runtime
│   ├── module-generator/
│   ├── module-validator/
│   ├── dependency-graph/
│   ├── schema-generator/
│   ├── abi-checker/
│   ├── shader-toolchain/
│   ├── reference-renderer/
│   ├── benchmark-runner/
│   ├── dataset-validator/
│   ├── scene-diff/
│   └── crash-symbolizer/
│
├── scripts/                             # Small cross-platform CLI entrypoints
│   ├── moonlight.py
│   ├── moonlight.sh
│   ├── moonlight.bat
│   └── moonlight/                        # Python CLI implementation
│
├── python/                              # Repository automation only
│   ├── build.py
│   ├── requirements.txt
│   ├── bootstrap/{linux,macos,windows}/
│   ├── ci/{build,test,validation,security,release}/
│   ├── tooling/{format,lint,docs,licenses}/
│   ├── data/{import,validate,convert}/
│   └── Platform/{Linux,Mac,Windows}/
│
├── tests/                               # Cross-module and contract tests
│   ├── native/ managed/ rust/
│   ├── module-contracts/
│   ├── api-compatibility/
│   ├── integration/ end-to-end/
│   ├── asset-pipeline/ scene-regression/
│   ├── render-regression/ audio-regression/
│   ├── failure-injection/ fuzz/
│   └── fixtures/{schemas,manifests,scenes,materials,corrupt-inputs}/
│
├── research/                             # Reproducible experiments, not runtime
│   ├── spectral/{hypotheses,protocols,experiments,datasets,scenes,analysis,
│   │             figures,papers,results}/
│   ├── orcha/{protocols,experiments,datasets,scenes,analysis,figures,results}/
│   └── atlas/{workloads,experiments,analysis,results}/
│
├── benchmarks/                           # Performance data and harnesses
│   ├── runtime/ ecs/ rendering/ voxel-gi/
│   ├── acoustic-simulation/ electromagnetic/
│   └── datasets/                         # Manifests, not large binaries
│
├── packaging/                            # Installers, archives, signing
│   ├── manifests/{linux,macos,windows}/
│   ├── cmake/ containers/ signing/ release-notes/
│   └── licenses/
│
├── templates/                             # Repository-level project templates
│   ├── minimal-project/
│   ├── rendering-project/
│   └── research-project/
```

### Why `data/` is a first-class root

O3DE-style engine repositories distinguish code from the registries and content
that configure or extend it. Moonlight uses `data/` for all versioned,
non-source inputs consumed by runtime, tools, and applications:

- `data/schemas/` is the canonical validation contract. Module-local schemas
  are allowed only for data private to that module and must be referenced from
  its manifest.
- `data/config/` contains checked-in defaults and named profiles. User
  overrides belong in `CMakeUserPresets.json`, OS user configuration, or an
  explicitly documented project directory, never in the repository defaults.
- `data/registry/` contains declarative discovery and capability metadata. It
  is not a second implementation of module discovery; generated catalogs must
  identify their source manifests and hashes.
- `data/templates/` contains scaffolding inputs. Generated projects remain
  outside the engine source tree and must pass validation before use.
- `data/builtin-assets/` is intentionally small. Large, proprietary, or
  generated assets use manifests in `research/`, `tests/fixtures/`, or
  controlled artifact storage.

There is no top-level `schemas/` or `config/` directory in the canonical
layout. This prevents schema/config ownership from being split across
competing roots.

## 3. Module contract and standard layout

Every registered module follows this layout:

```text
modules/<id>/
├── module.json                  # Required, schema-versioned identity
├── CMakeLists.txt               # Required for C++ or mixed modules
├── Cargo.toml                  # Required for Rust modules when standalone
├── README.md
├── CHANGELOG.md
├── include/moonlight/<name>/    # Stable public C++ headers only
├── src/                         # Private implementation
│   ├── internal/ algorithms/ services/ entry/
│   └── generated/               # Disposable codegen output
├── schemas/                     # Only module-private schemas
├── data/                        # Small module-owned defaults/assets
├── shaders/                     # GPU source and reflection metadata
├── tests/{unit,integration,contracts,failure}/
├── benchmarks/
├── docs/
└── examples/
```

`module.json` declares the ID, semantic version, language(s), required and
optional dependencies, capabilities, ABI compatibility, platform support, and
the CMake/Cargo targets. Discovery scans only `modules/*/module.json`; a
directory without a manifest is not a runtime module.

Modules must not depend on `apps/`, `python/`, `.agents/`, test-only code, or
Workbench implementation details. They may consume Core APIs, declared module
dependencies, and data contracts. A module-owned `data/` directory is for
private defaults and small assets; shared schemas and profiles remain under
root `data/`.

## 4. Ownership and dependency direction

```text
data schemas/config/registry
          ↓
Core runtime and platform services
          ↓
registered modules
          ↓
applications and managed tooling

tools/python/scripts ──→ build, validate, generate, package
tests/research/benchmarks ──→ observe and verify; never become runtime deps
```

The module graph has four explicitly different edge types:

1. **Runtime:** load/start dependency declared in `module.json`.
2. **Build:** CMake, Cargo, or .NET compile dependency.
3. **Capability:** provider/consumer relationship resolved by Core.
4. **Tooling:** a tool reads or generates an artifact but is not shipped with
   the runtime.

The graph compiler must preserve edge type, source file, version constraint,
platform condition, and optionality. Required runtime cycles, duplicate IDs,
ambiguous capability providers, and incompatible ABI versions are hard errors.

## 5. Data, assets, and generated output

All persistent data uses a versioned schema and records:

- `schema_id` and `schema_version`;
- stable identity or content hash;
- source/revision and tool version;
- units, coordinate system, and encoding where numerical;
- dependency hashes;
- migration or compatibility policy.

The asset pipeline is:

```text
source → imported → hashed → verified → decoded → resident → evicted
```

Paths are discovery hints, not identity. Corrupt or incomplete data never
becomes resident. Unknown fields are preserved where forward compatibility is
promised; unknown required components are retained as opaque data rather than
silently dropped. Generated catalogs, shader binaries, API bindings, graph
documents, and build files must record input hashes and are regenerated into
`build/` or `out/` unless explicitly checked in.

## 6. Build and developer workflow

`python/build.py` is the canonical complete-project orchestrator. It validates
modules and exports the graph/catalog before building native, Rust, and managed
stages. CMake presets are the portable source of native configuration; Cargo
owns the Rust workspace; `dotnet` owns managed restore/build/test.

```sh
python scripts/moonlight.py validate
python scripts/moonlight.py export --out build/graph/module-catalog.json
python python/build.py --configuration debug
python python/build.py --configuration release
```

No command may require Workbench, a display server, or a GPU for Core,
manifest, schema, graph, or CPU reference tests. GPU jobs are explicit matrix
jobs and unavailable hardware must be reported as unavailable, never as pass.

## 7. O3DE-inspired decisions and deliberate differences

| O3DE concept | Moonlight equivalent | Difference |
|---|---|---|
| Gem | Registered module under `modules/` | Module IDs and C++/Rust ABI contracts are Moonlight-specific |
| `gem.json` | `module.json` | Schema includes language, capabilities, ABI, platform, and graph layers |
| Engine registry | `data/registry/` plus generated catalog | Runtime registry is generated from validated manifests |
| Project assets/config | `data/` and project-owned external data | Shared schemas/config are centralized under one root |
| O3DE tools/scripts | `tools/`, `scripts/`, `python/` | Rust/C# tools are allowed; Python remains orchestration-only |
| Engine code | `src/` and registered modules | Foundational Core is separated from feature modules |
| Templates | `data/templates/` and root `templates/` | Templates are versioned and schema-validated |

This is structural inspiration, not a dependency on O3DE code or assets.
Moonlight must not copy O3DE implementation or licensed content.

## 8. Required validation and CI

Every change to this architecture must validate:

1. Directory ownership and forbidden dependency direction.
2. Module manifests, schemas, required dependency acyclicity, and capability
   resolution.
3. CMake configure/build/test, Cargo format/check/test, and .NET restore/build
   according to the changed surface.
4. Generated catalog freshness and graph-layer separation.
5. Serialization migrations and unknown-data preservation.
6. Sanitizer, fuzz, failure-injection, and ABI compatibility tests where the
   changed contract requires them.
7. Documentation links, Markdown fences, and architecture conformance.

The six workflow files are the only CI workflow entrypoints:
`build.yml`, `test.yml`, `validation.yml`, `security.yml`, `release.yml`, and
`pr.yml`. Shared checks are implemented as scripts or reusable workflow calls,
not duplicated ad-hoc commands.

## 9. Milestones

1. Core, module manifest schema, Python registry CLI, CMake discovery, one C++
   module, and cross-platform build.
2. Atlas ECS, transforms, hierarchy, serialization, and headless snapshot tests.
3. Managed C ABI interop and minimal Workbench scene inspection.
4. Spectral foundation, measured-data schemas, Fresnel/absorption tests, and
   independent reference path tracer.
5. Orcha direct-path acoustics, DSP, spatialization, and real-time-safe output.
6. Graph compiler, generated catalogs, browser explorer, and module generator.
7. Voxel GI compared against reference transport; then advanced optical and
   acoustic solvers.
8. Reproducible research, compatibility migrations, signed packaging, and
   production hardening.

## 10. Non-negotiable rules

- Never add a second root for schemas, shared config, or module discovery.
- Never register an empty future module merely to reserve a folder.
- Never expose C++ or Rust object layouts across the stable plugin boundary.
- Never block, allocate without bounds, or run propagation in the real-time
  audio callback.
- Never silently discard unknown serialized data.
- Never introduce a required dependency cycle.
- Never present cached or approximate output as exact physical simulation.
- Never commit build output, credentials, generated binaries, or unlicensed
  datasets.
- Never make the GUI editor a prerequisite for headless builds or CI.
- Never treat a folder tree as evidence that a feature is implemented.
