# Architecture conformance

**Status:** Design target

Conformance is the automated check that implementation, manifests, and
documentation still agree. It supplements unit tests and does not change any
accepted decision.

## Rules

| Rule | Checkable evidence |
| --- | --- |
| Module IDs are unique and required edges are acyclic | Registry validation and graph export |
| Dependency direction is respected | Build/runtime graph diff; forbidden-edge test |
| Lifecycle order is legal | State-machine contract tests |
| Boundary types are ABI-safe | Header inspection, ABI fixtures, sanitizer run |
| Snapshots are immutable and deterministic | Concurrent-reader and same-input hash tests |
| Data is bounded and verified | Schema, size/quota, hash and fuzz tests |
| Runtime failures are observable | Correlated event/metric/span assertions |
| Security checks precede execution | Negative admission and traversal fixtures |

## Conformance workflow

1. Discover manifests and source-owned contracts.
2. Validate schemas, IDs, dependency graphs, ABI metadata, and links.
3. Generate catalogs and graph artifacts; compare against deterministic output.
4. Run state-machine, property/fuzz, compatibility, and fault scenarios.
5. Build each supported configuration and execute targeted tests.
6. Publish an evidence bundle with command, commit, toolchain, platform,
   fixture/corpus hashes, results, and known design targets.

No generated artifact is hand-edited to make a check pass. A conformance
failure blocks startup or release according to the affected boundary. A
documented design target may be temporarily non-conforming only when tracked
as `TODO(CONTRACT:<id>)` with an owner and due release.

## Required checks

The repository validation job must check Markdown fences and trailing
whitespace, resolve relative links, reject edits to
`graph-system-specification.md` for documentation-only conformance changes,
and verify that every architecture state machine names failure behavior.
The exact command and result belong in the CI evidence bundle; this document
does not claim that an unimplemented checker already exists.

See [contract verification](contract-verification.md), [dependency rules](dependency-rules.md),
[observability](observability.md), and [security](security.md).
