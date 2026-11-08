# Moonlight Monorepo — Architecture & Developer Tooling

**Version:** 1.0  
**Date:** 2026-10-26  
**Status:** Proposed target architecture. Planned directories are not implemented features.

## 1. Architectural principles

1. One monorepo, with explicit module IDs, versioned manifests, and an acyclic required-dependency graph.
2. C++ owns performance-critical runtime systems; C# owns Workbench and managed tooling; Rust is used for selected isolated tools and utilities where its value justifies FFI complexity.
3. CMake/Ninja build native targets; Cargo manages Rust; the .NET CLI manages managed targets. A root task runner provides one developer-facing command vocabulary.
4. Core owns module discovery, lifecycle, service registration, jobs, resources, diagnostics, platform abstractions, and the stable C ABI.
5. Atlas owns scene identity, entity/component storage, hierarchy, transforms, serialization, and immutable scene snapshots. Spectral, Orcha, Flywheel and Physics consume extracted scene data rather than maintaining unrelated copies of authoring state.
6. Spectral keeps a reference-capable path tracer distinct from voxel GI, caches, coherent transport and Maxwell solvers. Approximation error is measured and documented.
7. Research inputs, protocols, source data provenance, result manifests and analysis scripts are versioned. Large datasets and rendered results live in managed storage or CI artifacts, not as arbitrary binaries in Git.
8. A repository is not considered working merely because its folder tree exists. Every implemented module must build, have a valid manifest, pass contract tests, and document its assumptions.
9. Registered engine modules are implemented in C++, Rust, or a deliberate combination of both. C# owns the Editor, scripting language/runtime, Workbench and managed tooling; Python owns repository automation, build orchestration and project management.

## 2. Complete target folder architecture

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
├── vcpkg.json
├── vcpkg-configuration.json
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
├── schemas/
│   ├── module-manifest.schema.json
│   ├── module-catalog.schema.json
│   ├── module-graph.schema.json
│   ├── component-manifest.schema.json
│   ├── scene.schema.json
│   ├── prefab.schema.json
│   ├── material.schema.json
│   ├── render-config.schema.json
│   ├── acoustic-material.schema.json
│   ├── asset-manifest.schema.json
│   └── experiment.schema.json
│
├── cmake/
│   ├── MoonlightOptions.cmake
│   ├── MoonlightWarnings.cmake
│   ├── MoonlightSanitizers.cmake
│   ├── MoonlightDependencies.cmake
│   ├── MoonlightModule.cmake
│   ├── MoonlightCodegen.cmake
│   ├── MoonlightInstall.cmake
│   └── toolchains/
│       ├── windows-msvc.cmake
│       ├── linux-clang.cmake
│       └── linux-gcc.cmake
│
├── config/
│   ├── default-engine.toml
│   ├── default-renderer.toml
│   ├── default-audio.toml
│   ├── logging.toml
│   └── profiles/
│       ├── development.toml
│       ├── production.toml
│       ├── reference-render.toml
│       ├── research-render.toml
│       ├── acoustic-lab.toml
│       └── headless-ci.toml
│
├── src/
│   ├── native/
│   │   ├── core/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── include/moonlight/core/
│   │   │   │   ├── runtime/
│   │   │   │   ├── modules/
│   │   │   │   ├── services/
│   │   │   │   ├── capabilities/
│   │   │   │   ├── plugins/
│   │   │   │   ├── jobs/
│   │   │   │   ├── resources/
│   │   │   │   ├── events/
│   │   │   │   ├── logging/
│   │   │   │   ├── memory/
│   │   │   │   ├── platform/
│   │   │   │   ├── diagnostics/
│   │   │   │   └── public_api/
│   │   │   └── src/
│   │   │       ├── runtime/
│   │   │       ├── modules/
│   │   │       ├── services/
│   │   │       ├── capabilities/
│   │   │       ├── plugins/
│   │   │       ├── jobs/
│   │   │       ├── resources/
│   │   │       ├── events/
│   │   │       ├── logging/
│   │   │       ├── memory/
│   │   │       ├── platform/
│   │   │       └── diagnostics/
│   │   ├── public-api/
│   │   │   ├── include/moonlight/module_api.h
│   │   │   └── src/
│   │   └── generated/
│   │       └── .gitkeep
│   ├── managed/
│   │   ├── Moonlight.Interop/
│   │   ├── Moonlight.ManagedSDK/
│   │   └── Moonlight.Diagnostics/
│   └── shaders/
│       ├── common/
│       └── generated/
│
├── modules/
│   ├── atlas/
│   │   ├── module.json
│   │   ├── CMakeLists.txt
│   │   ├── include/moonlight/atlas/
│   │   │   ├── api/
│   │   │   ├── ecs/
│   │   │   ├── scene/
│   │   │   ├── hierarchy/
│   │   │   ├── transform/
│   │   │   ├── components/
│   │   │   ├── reflection/
│   │   │   ├── systems/
│   │   │   ├── extraction/
│   │   │   ├── serialization/
│   │   │   ├── prefabs/
│   │   │   ├── events/
│   │   │   └── diagnostics/
│   │   ├── src/
│   │   │   ├── ecs/
│   │   │   ├── scene/
│   │   │   ├── hierarchy/
│   │   │   ├── transform/
│   │   │   ├── reflection/
│   │   │   ├── systems/
│   │   │   ├── extraction/
│   │   │   ├── serialization/
│   │   │   ├── prefabs/
│   │   │   ├── events/
│   │   │   └── diagnostics/
│   │   ├── schemas/
│   │   ├── tests/{unit,integration,contracts,failure}/
│   │   ├── benchmarks/
│   │   ├── docs/
│   │   └── examples/
│   │
│   ├── spectral/
│   │   ├── README.md
│   │   ├── CMakeLists.txt
│   │   ├── cmake/
│   │   ├── foundation/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/{units,spectrum,radiometry,complex,polarization,geometry,probability,numerical}/
│   │   │   ├── src/
│   │   │   ├── tests/
│   │   │   └── docs/
│   │   ├── data/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/data/
│   │   │   ├── src/{ingestion,interpolation,provenance,uncertainty,validation}/
│   │   │   ├── schemas/
│   │   │   └── tests/
│   │   ├── materials/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/materials/
│   │   │   ├── src/{models,interfaces,composition,evaluation,layers,validation}/
│   │   │   ├── schemas/
│   │   │   └── tests/
│   │   ├── lighting/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/lighting/
│   │   │   └── src/{emitters,area-lights,environment,spectral-power,light-sampling,calibration}/
│   │   ├── transport/
│   │   │   ├── path-tracing/
│   │   │   │   ├── module.json
│   │   │   │   ├── include/moonlight/spectral/path_tracing/
│   │   │   │   ├── src/{camera-rays,intersections,direct-lighting,indirect-lighting,mis,path-state,integrators}/
│   │   │   │   ├── tests/
│   │   │   │   └── benchmarks/
│   │   │   ├── bidirectional/                 # future
│   │   │   ├── photon-mapping/                # future
│   │   │   └── transient/                     # future or separate registered module
│   │   ├── gi/
│   │   │   ├── voxel-gi/
│   │   │   │   ├── module.json
│   │   │   │   ├── include/moonlight/spectral/voxel_gi/
│   │   │   │   ├── src/{voxelization,hierarchy,clipmaps,sparse-grids,radiance-injection,directional-radiance,spectral-encoding,visibility,cone-tracing,multibounce,dynamic-updates,cache-validation}/
│   │   │   │   ├── shaders/{voxelize,injection,propagation,cone-tracing}/
│   │   │   │   ├── tests/{unit,numerical,leakage,reference-comparison}/
│   │   │   │   ├── benchmarks/
│   │   │   │   ├── docs/
│   │   │   │   └── research/
│   │   │   ├── probe-gi/                      # optional module
│   │   │   ├── radiance-cache/                # optional module
│   │   │   ├── caustics/                      # specialized transport
│   │   │   └── sampling/                      # internal helper until separately needed
│   │   ├── optics/
│   │   │   ├── interfaces/
│   │   │   ├── polarization/
│   │   │   ├── wave-optics/
│   │   │   ├── thin-films/
│   │   │   ├── diffraction/
│   │   │   └── fluorescence/
│   │   ├── volumes/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/volumes/
│   │   │   └── src/{absorption,extinction,scattering,phase-functions,volume-emission,heterogeneous-media,subsurface,wavelength-conversion}/
│   │   ├── solvers/maxwell/
│   │   │   ├── module.json
│   │   │   ├── include/moonlight/spectral/maxwell/
│   │   │   ├── src/{common,fdtd,frequency-domain,finite-elements,boundary-conditions,meshing,convergence,renderer-coupling}/
│   │   │   ├── tests/
│   │   │   └── benchmarks/
│   │   ├── imaging/{camera,sensors,colour-science}/
│   │   ├── backends/{cpu,vulkan,dx12}/
│   │   ├── diagnostics/
│   │   └── integration-tests/
│   │
│   ├── orcha/
│   │   ├── README.md
│   │   ├── CMakeLists.txt
│   │   ├── foundation/
│   │   ├── dsp/
│   │   ├── sources/
│   │   ├── spatial/
│   │   ├── acoustics/
│   │   │   ├── scene/
│   │   │   ├── materials/
│   │   │   ├── propagation/                   # internal contracts initially
│   │   │   ├── ray-tracing/
│   │   │   ├── diffraction/
│   │   │   ├── wave-solver/                   # optional
│   │   │   ├── room-response/
│   │   │   └── baking/
│   │   ├── output/
│   │   ├── diagnostics/
│   │   └── integration-tests/
│   │
│   ├── flywheel/
│   │   ├── module.json
│   │   ├── CMakeLists.txt
│   │   ├── include/moonlight/flywheel/{animation,graph,skeleton,pose,ik,events,serialization}/
│   │   ├── src/{runtime,graph-evaluation,blending,skeleton,pose-cache,ik,root-motion,retargeting}/
│   │   ├── tests/{unit,integration,determinism}/
│   │   ├── benchmarks/
│   │   └── docs/
│   │
│   ├── physics/
│   │   ├── module.json
│   │   ├── include/moonlight/physics/
│   │   ├── src/{world,rigid-body,colliders,queries,solver,constraints,character-controller}/
│   │   ├── tests/
│   │   └── docs/
│   │
│   ├── graphics/
│   │   ├── module.json
│   │   ├── include/moonlight/graphics/{device,resources,commands,shaders,capabilities}/
│   │   ├── src/{rhi,resource-lifetime,shader-compiler,validation}/
│   │   ├── backends/{vulkan,dx12,reference-cpu}/
│   │   └── tests/
│   │
│   ├── assets/
│   │   ├── module.json
│   │   ├── include/moonlight/assets/{ids,handles,metadata,importers,catalog}/
│   │   ├── src/{database,dependency-tracking,import,conversion,watcher,streaming,versioning}/
│   │   ├── importers/{gltf,images,audio,fonts}
│   │   ├── tests/
│   │   └── docs/
│   │
│   ├── input/{module.json,include,src,tests}/
│   ├── scripting/{module.json,include,src,tests}
│   ├── networking/{module.json,include,src,tests}
│   ├── ui/{module.json,include,src,tests}
│   └── telemetry/{module.json,include,src,tests}
│
├── apps/
│   ├── workbench/
│   │   ├── Workbench.sln
│   │   ├── src/
│   │   │   ├── Workbench.App/
│   │   │   ├── Workbench.Shell/
│   │   │   ├── Workbench.Interop/
│   │   │   ├── Workbench.ProjectSystem/
│   │   │   ├── Workbench.SceneEditor/
│   │   │   ├── Workbench.Inspector/
│   │   │   ├── Workbench.AssetBrowser/
│   │   │   ├── Workbench.RenderView/
│   │   │   ├── Workbench.AudioEditor/
│   │   │   ├── Workbench.AnimationEditor/
│   │   │   ├── Workbench.PhysicsTools/
│   │   │   ├── Workbench.Profilers/
│   │   │   └── Workbench.PluginHost/
│   │   ├── tests/
│   │   └── docs/
│   ├── player/
│   ├── headless-runner/
│   ├── asset-compiler/
│   ├── render-cli/
│   ├── audio-bake-cli/
│   └── module-inspector/
│
├── plugins/
│   ├── graphics/
│   ├── asset-importers/
│   ├── asset-exporters/
│   ├── integrations/
│   └── experimental/
│
├── rust-crates/
│   ├── module-graph/
│   ├── module-manifest/
│   ├── asset-metadata/
│   ├── material-database/
│   ├── asset-conversion/
│   └── experiment-manifest/
│
├── tools/
│   ├── module-generator/
│   ├── module-validator/
│   ├── dependency-graph/
│   │   ├── src/{cli,extractors,graph,validation,generators,server}/
│   │   ├── tests/
│   │   └── docs/
│   ├── schema-generator/
│   ├── abi-checker/
│   ├── dependency-policy-checker/
│   ├── shader-toolchain/
│   ├── reference-renderer/
│   ├── benchmark-runner/
│   ├── dataset-validator/
│   ├── scene-diff/
│   └── crash-symbolizer/
│
├── apps/module-graph-web/
│   ├── package.json
│   ├── package-lock.json
│   ├── tsconfig.json
│   ├── vite.config.ts
│   ├── index.html
│   ├── src/{app,components,graph,views,api,styles}/
│   └── tests/
│
├── tests/
│   ├── native/
│   ├── managed/
│   ├── rust/
│   ├── module-contracts/
│   ├── api-compatibility/
│   ├── integration/
│   ├── end-to-end/
│   ├── asset-pipeline/
│   ├── render-regression/
│   ├── audio-regression/
│   ├── scene-regression/
│   └── failure-injection/
│
├── research/
│   ├── spectral/{hypotheses,protocols,experiments,datasets,scenes,analysis,figures,papers}/
│   ├── orcha/{scenes,datasets,protocols,experiments,analysis,figures,papers}/
│   └── atlas/{workloads,experiments,analysis}/
│
├── benchmarks/
│   ├── runtime/
│   ├── ecs/
│   ├── rendering/
│   ├── voxel-gi/
│   ├── acoustic-simulation/
│   ├── electromagnetic/
│   └── datasets/
│
├── packaging/
│   ├── manifests/
│   ├── cmake/
│   ├── containers/
│   ├── signing/
│   └── release-notes/
│
├── devcontainer/
│   ├── devcontainer.json
│   ├── Dockerfile
│   └── post-create.sh
│
└── python/
    ├── bootstrap/
    ├── scripts/
    ├── ci/
    ├── build/
    ├── test/
    ├── release/
    └── policies/
```

The tree above is the intended shape. Do not create every future subsystem as an empty registered module on day one. Each registered module must have a real manifest, API, implementation, tests and known capabilities. Internal helper libraries should remain unregistered until they need an independent lifecycle or capability contract.

## 3. Standard internal layout for each registered module

```text
<module>/
├── module.json
├── CMakeLists.txt
├── README.md
├── CHANGELOG.md
├── include/moonlight/<module>/   # Stable public C++ API
├── src/                          # Private implementation
│   ├── internal/
│   ├── algorithms/
│   ├── services/
│   └── entry/
├── schemas/                      # Versioned module-owned data schemas
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── contracts/
│   └── failure/
├── benchmarks/
├── docs/
└── examples/
```

Add `shaders/` for GPU modules, `research/` for research-heavy systems, and `assets/` only for small built-in assets. Do not store generated build output in module source folders.

## 4. Core registry and build graph

Use these distinct concepts:

- **ModuleRegistry:** catalog of discovered module descriptors.
- **ModuleManager:** dependency-resolved load/start/stop/unload lifecycle.
- **ServiceRegistry:** explicit service interfaces and handles.
- **CapabilityRegistry:** capability discovery and provider resolution.
- **Module Graph Compiler:** reads module manifests and build-system metadata, validates graph policy, and emits canonical JSON, generated module catalog, Mermaid diagrams and a validation report.
- **Browser Graph Explorer:** reads the generated graph document; displays runtime dependencies, build dependencies, capabilities, dependents, cycles, missing dependencies and graph diffs.

Keep runtime module dependencies and build target dependencies as separate graph layers. A CMake target is not automatically a runtime module. A capability provider may satisfy a declared service dependency without creating a direct compile-time link.

Use a versioned C-compatible ABI for dynamic cross-language plugins. Do not expose compiler-specific C++ class layouts across the plugin boundary. Validate ABI major versions, struct sizes, ownership, callback lifetime, exception boundaries, threading rules and plugin signatures before loading.

## 5. Developer toolchain

### Recommended defaults

| Area | Default | Notes |
|---|---|---|
| C++ standard | C++20 initially | Raise per-target only after compiler/platform support is tested. |
| Native build | CMake + Ninja | Shared presets and target helper functions. |
| Native dependencies | vcpkg manifest mode | Commit baseline and declarative dependency manifest. |
| Rust | Cargo workspace | Workspace-level lockfile; fmt, Clippy and dependency/license checks. |
| .NET | .NET 10 LTS | Use `global.json`, centralized package management and SDK-style projects. |
| Workbench UI | C# desktop UI spike, with Avalonia as a candidate | Validate native render-view integration, DPI, input and docking before committing to a UI framework. |
| Browser graph | TypeScript + Vite + Cytoscape.js; Mermaid export | Graph content comes from validated JSON, not manually drawn node data. |
| Python automation | `uv` + pinned lockfile, Ruff and pytest | For dataset validation, analysis and CI scripts only. |
| Formatting | clang-format, rustfmt, dotnet format, Ruff | Enforce in CI; avoid style changes unrelated to a patch. |
| Native tests | GoogleTest or Catch2 + CTest | Pick one primary C++ unit framework. |
| Managed tests | xUnit | Keep managed interop and Workbench tests separated. |
| Native safety | ASan/UBSan; TSan where supported | Debug/CI configurations; not production instrumentation. |
| Profiling | Tracy + platform/GPU vendor tools | Keep instrumentation toggled for measuring overhead. |
| Docs | Markdown, Mermaid, Doxygen, DocFX, rustdoc | Generate API docs in CI. |
| CI | GitHub Actions or GitLab CI | Start with one CI provider and one canonical set of scripts. |
| Dev environment | Dev Container + bootstrap scripts | Reproducible tool versions and setup instructions. |

### Toolchain policy

- Pin SDK/compiler/dependency versions where practical and document upgrade windows.
- Commit `Cargo.lock` for the workspace and use a vcpkg baseline.
- Centralize NuGet package versions in `Directory.Packages.props`.
- Use CMake `CMakePresets.json` for checked-in build configurations and never commit a developer's `CMakeUserPresets.json`.
- Keep `build/`, `out/`, `target/`, `bin/`, `obj/`, shader intermediates and local caches out of source control.
- Keep a source-controlled SBOM and signed/checksummed release artifacts for production builds.
- Never require the GUI editor to build the headless runtime or run CI reference tests.
- Keep language boundaries explicit: module code must not depend on Editor or scripting implementation details, and C# must consume native functionality through the stable C ABI or generated interop layer.

## 6. One-command developer workflow

Define a root `justfile` and `python/build.py`. Python is the canonical project
orchestrator and may invoke CMake, Cargo and the .NET SDK, but it does not
replace those language-specific build systems. The orchestrator must build and
test every current target by default, while allowing individual stages to be
skipped for fast local iteration.

```make
set dotenv-load

setup:
    python/get_python.sh

configure:
    cmake --preset dev

build:
    cmake --build --preset dev

test-native:
    ctest --preset dev

test-rust:
    cargo test --workspace --locked

test-managed:
    dotnet test Moonlight.sln

lint-rust:
    cargo fmt --all -- --check
    cargo clippy --workspace --all-targets -- -D warnings

lint-managed:
    dotnet format Moonlight.sln --verify-no-changes

validate-modules:
    cargo run --locked -p module-graph -- validate

write-graph:
    cargo run --locked -p module-graph -- export --format json --out build/graph/module-graph.json

serve-graph:
    cargo run --locked -p module-graph -- serve --watch

validate:
    just validate-modules
    just test-native
    just test-rust
    just test-managed
```

These are proposed commands. Implement the corresponding CMake presets, Cargo package names, graph CLI and test presets before treating the commands as executable.

## 7. Module generator and validator

Create `tools/module-generator/` to generate a compiling starter module with:

- a schema-valid `module.json`;
- a CMake library target;
- a minimal public API and private implementation;
- lifecycle/ABI entry point where required;
- unit and module-contract tests;
- a README with dependencies and capability list.

Create `tools/module-validator/` to detect:

- duplicate module IDs and invalid manifests;
- missing required dependencies and invalid semver ranges;
- cycles in the required runtime dependency graph;
- duplicate or unresolved capability providers;
- unsupported ABI and platform combinations;
- dependency-policy violations;
- unregistered component IDs or schema versions;
- generated catalogs/graphs that are stale compared with source manifests.

The validator should emit machine-readable JSON and a concise human-readable report. A CI failure must say which module and dependency caused the failure and give a suggested fix.

## 8. Dependency graph explorer

```text
apps/module-graph-web/
├── package.json
├── package-lock.json
├── vite.config.ts
├── src/
│   ├── app/
│   ├── components/
│   ├── graph/
│   │   ├── model.ts
│   │   ├── filters.ts
│   │   ├── traversal.ts
│   │   └── diff.ts
│   ├── views/
│   │   ├── interactive-graph.ts
│   │   ├── module-details.ts
│   │   ├── dependency-path.ts
│   │   ├── validation-results.ts
│   │   └── mermaid-view.ts
│   └── api/
└── tests/
```

Required features: pan/zoom, node selection, search, layer filters, direct/transitive dependencies, reverse dependents, dependency path, manifest/version details, validation errors, graph diff between Git revisions, Mermaid export, SVG/PNG export, watch mode and a live-development mode. Default the local server to loopback. Use normal authentication and authorization if hosted remotely.

## 9. Research and correctness structure

For Spectral and Orcha, keep research material separate from runtime code:

```text
research/spectral/
├── hypotheses/
├── protocols/
├── experiments/
├── datasets/manifests/
├── scenes/
│   ├── analytic/
│   ├── material-reference/
│   ├── voxel-gi/
│   ├── polarization/
│   ├── diffraction/
│   └── transient/
├── results/README.md
├── analysis/
├── figures/
└── papers/

research/orcha/
├── scenes/
├── datasets/manifests/
├── protocols/
├── experiments/
├── analysis/
├── figures/
└── papers/
```

Each run should record source commit, build/renderer ID, material and scene hashes, input conditions, random seeds where relevant, solver parameters, numerical settings and output metadata. Keep large datasets and render outputs in controlled data storage; commit their manifests and checksums.

For Spectral, keep the reference path tracer independent of Voxel GI and cache modules. Compare approximate modes against converged references with error metrics, runtime and memory measurements. For Orcha, compare predicted impulse responses and propagation delays against analytical cases or measured data. A visually convincing image or plausible sound is not a substitute for physical validation.

## 10. CI stages

1. Format, schema and manifest validation.
2. Dependency and architecture policy validation.
3. Native, Rust and .NET builds.
4. Unit tests and public API/module contract tests.
5. Sanitizer jobs and failure-injection tests.
6. Shader compile/validation where a backend is available.
7. Deterministic analytic tests and asset pipeline tests.
8. Reference render, voxel GI, acoustic and scene regression tests.
9. Performance tests on dedicated runners; report trends before making them hard merge gates.
10. Security/dependency/license review, SBOM and release signing.

Use GPU-specific runners for GPU-only validation, but keep CPU reference tests and schema/API validation runnable on ordinary CI machines. Missing GPU hardware must produce an explicit skipped/unavailable status, not a false pass for a backend advertised as supported.

## 11. Build in vertical slices

Do not implement the entire target tree before the first working build. Suggested milestones:

1. Root build/toolchain, Core module registry, one static and one dynamic example module, schema validation and basic CI.
2. Atlas ECS, mandatory Transform, hierarchy, serialization and a headless scene test.
3. Managed interop and a minimal Workbench capable of opening a scene and inspecting components.
4. Spectral foundation, measured-data schema, Fresnel/absorption tests and a reference path tracer.
5. Orcha audio output/DSP baseline, spatialization and direct-path occlusion.
6. Dependency graph compiler, browser explorer and module generator.
7. Voxel GI against reference rendering; then advanced optical/acoustic solvers.
8. Production hardening, compatibility policy, reproducible research pipeline and signed packaging.

## 12. Non-negotiable architecture rules

- No direct references between arbitrary modules; dependencies must be declared and validated.
- No Workbench dependency in shipped runtime targets.
- No C++ object layout exposed as a stable cross-language plugin ABI.
- No blocking locks, unbounded allocations or expensive simulation work in real-time audio callbacks.
- No silently dropping unknown serialized components when a plugin is missing.
- No circular required module dependencies.
- No claim that an approximation is exact; record assumptions, validity regime and measured error.
- No manual, unvalidated duplicate of the dependency graph. Generate catalog, Mermaid and browser graph data from canonical validated sources.
- No generated build outputs or unlicensed datasets committed accidentally.
```
