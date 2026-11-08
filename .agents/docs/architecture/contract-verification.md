# Contract verification

**Status:** Design target
**Scope:** Runtime, module, data, and tool boundaries

This document defines how an architectural statement becomes an executable
check. It does not replace the contracts in the linked documents.

## Verification vocabulary

| Term | Required evidence |
| --- | --- |
| Invariant | A property that remains true before and after every successful operation |
| Precondition | Input, state, capability, and ownership checks required before an operation |
| Postcondition | Observable state and result guaranteed after successful completion |
| Design target | Intended behavior not yet enforced by an implementation; it must have an owner and test ID |
| Contract test | A black-box test that can run against every conforming provider |

Claims without a test ID, fixture, or measurable acceptance criterion are
non-normative prose, not architecture.

## Boundary checklist

Every public operation must document:

1. **Preconditions:** schema/version, state-machine state, thread affinity,
   deadline, cancellation, capabilities, and ownership.
2. **Postconditions:** result status, state transition, resource ownership,
   publication/visibility, and whether retries are safe.
3. **Errors:** stable category, retryability, remediation, and redaction rule.
4. **Concurrency:** permitted readers/writers, ordering, linearization point,
   and shutdown behavior.
5. **Evidence:** unit/contract/property test, fixture, metric, or trace event.

Example: `scene.publish(snapshot)` requires a sealed snapshot, an owner-held
handle, and the publication thread. Success atomically changes the current
snapshot and emits `snapshot.published`; failure leaves the prior snapshot
current and emits `snapshot.publish_failed`.

## Test layers

| Layer | Checks | Minimum cadence |
| --- | --- | --- |
| Schema/golden | Required fields, canonical encoding, migrations | Every change |
| Contract | State transitions and error taxonomy through public APIs | Every provider |
| Property/fuzz | Bounds, idempotence, determinism, malformed inputs | PR and nightly corpus |
| Integration | Dependency ordering, shutdown, cross-language ownership | PR |
| Compatibility | Previous release readers, ABI fixtures, platform matrix | Release |
| Evidence | Correlation, redaction, bounded queues, health transitions | PR and fault runs |

Property tests must assert that valid inputs preserve invariants and invalid
inputs never escape the documented error boundary. Fuzzers must cap input size,
nesting, allocations, execution time, and decompression ratio.

## Required acceptance record

For each contract, CI records: test command, fixture/corpus revision, toolchain
digest, platform, seed (if randomized), pass/fail, and failing artifact hash.
An unimplemented design target is marked `TODO(CONTRACT:<id>)`; it must not be
described as an enforced guarantee.

See [architecture conformance](architecture-conformance.md),
[fault injection and recovery](fault-injection-recovery.md), and
[ABI compatibility testing](abi-compatibility-testing.md).
