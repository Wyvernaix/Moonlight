# Architecture overview

**Status:** Draft
**Owners:** Runtime and tools maintainers
**Normative language:** “must” is a contract; “should” is a default.

Moonlight is a cross-platform, modular game and research engine. C++ and Rust
implement runtime modules, C# implements Editor, Workbench, scripting and
managed tooling, and Python owns repository automation. The module manifest is
the source of truth; generated build and graph artifacts are never edited by
hand.

## System boundaries

| Boundary | Contract |
| --- | --- |
| Authoring | Atlas owns mutable scene state and emits immutable snapshots |
| Runtime | Modules communicate through registered services/capabilities |
| Native/managed | Versioned C ABI, generated bindings, explicit ownership |
| Data | Schema-versioned documents with checksums and migration policy |
| Platform | `Platform` services hide OS, window, filesystem, clock and GPU APIs |
| Operations | Structured logs, metrics, traces and reproducible run manifests |

The dependency direction is `platform → core → runtime modules → applications`;
Editor and Workbench may consume runtime APIs but runtime targets must not
depend on them. See [dependency rules](dependency-rules.md),
[module lifecycle](module-lifecycle.md), and [platform abstraction](platform-abstraction.md).

## Invariants

1. Every runtime module has one globally unique manifest ID.
2. Required dependencies are acyclic and start before their dependents.
3. Optional dependencies never change whether a module can start.
4. No public boundary passes compiler-owned C++/Rust/C# layout or allocator.
5. Consumers never mutate an acquired scene snapshot.
6. External artifacts are addressed by content hash and are never trusted solely
   by filename.
7. A failed validation prevents startup; it is not downgraded to a warning.

## Startup and shutdown

`Discovered → Validated → Resolved → Loaded → Started → Quiescing → Stopped →
Unloaded` is the only legal module state machine. A failure transitions to
`Failed`, records a diagnostic, and rolls back in reverse dependency order.
The host owns the transition; modules cannot start dependents directly.

## Verification

The minimum gate is `python/build.py --skip rust --skip managed` followed by
`ctest --test-dir build/debug --output-on-failure`. CI additionally validates
schemas, graph cycles, ABI fixtures, Markdown links, and reproducibility
manifests. New architecture decisions belong in [`adr/`](adr/).

## Compatibility matrix

| Consumer | Stable surface | Compatibility rule | Evidence |
| --- | --- | --- | --- |
| Runtime module | Manifest, capability, ABI | Required range and ABI major must match | Graph + ABI fixtures |
| Editor/Workbench | Managed API and snapshot schema | Current and migratable prior schema | Migration/golden tests |
| Asset store | Hash, media type, byte length | Hash and decoder support required | Store and fuzz tests |
| Platform | `Platform` service contract | Target triple and capability policy | Platform matrix |

Compatibility is evaluated before execution; no fallback may silently change
semantics. See [versioning](versioning.md) and [ABI compatibility testing](abi-compatibility-testing.md).

## Threat-model boundary

The host trusts its own validated code, build metadata, and policy roots. It
does not trust plugins, manifests, assets, captures, network data, generated
files, or telemetry sinks. Authentication, operating-system sandboxing,
signing-key custody, and denial-of-service protection outside the process are
system boundaries, not claims of this architecture. See [security](security.md).

## Observable acceptance evidence

Each startup, publication, rejection, rollback, timeout, and degraded-mode
transition must have a correlated event and a bounded metric. A release record
must identify the command, commit, toolchain, platform, fixture/corpus hashes,
and result. These are design targets until enforced by CI; the verification
method is defined in [contract verification](contract-verification.md).
