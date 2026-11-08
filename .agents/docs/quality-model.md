# Documentation quality and proof obligations

**Status:** Normative documentation policy

Moonlight documentation is useful only when an engineer can determine what is
guaranteed, what is measured, what is assumed, and what happens when the
assumption is false. This page defines the minimum proof obligation for
architecture, runtime, numerical, and tooling claims.

## Claim classes

Every non-trivial claim belongs to one class:

| Class | Meaning | Required evidence |
| --- | --- | --- |
| Contract | A caller or implementer may rely on it | Schema/header, pre/postconditions, compatibility rule, and contract test |
| Invariant | Must remain true across all valid executions | State or data invariant, enforcement point, negative test |
| Measurement | Observed property of an implementation | Reproducible command, environment, sample count, result artifact |
| Model assumption | Simplification with a bounded domain | Validity range, error budget, fallback, out-of-domain diagnostic |
| Design target | Accepted future direction | Owner, dependencies, acceptance criteria, and explicit non-availability |
| Example | Illustrative only | Label it as non-normative; never use as test evidence |

Do not use a design target as an API promise. Do not use a single successful
run as proof of a universal invariant.

## Minimum technical proof

A new runtime or module contract must document:

1. **Inputs:** types, units, coordinate system, valid ranges, encoding, and
   whether values are trusted or externally supplied.
2. **Outputs:** ownership, lifetime, revision, validity/confidence state, and
   whether partial results are possible.
3. **Preconditions:** required module capabilities, thread/queue affinity,
   resource state, feature tier, and model regime.
4. **Postconditions:** state changes, emitted events, resource transitions,
   revision increments, and observable diagnostics.
5. **Failure semantics:** typed error categories, retryability, fallback,
   cleanup, and whether the operation is idempotent.
6. **Concurrency:** read/write sets, synchronization boundary, cancellation,
   ordering, and permitted reentrancy.
7. **Compatibility:** schema/API/ABI version, forward and backward behavior,
   migration path, and rejection conditions.
8. **Evidence:** unit tests, property tests, integration tests, fault-injection
   tests, benchmark or numerical reference where applicable.

If any item is unknown, the document remains Draft and must name the missing
decision rather than inventing a default.

## Invariants and enforcement

An invariant is incomplete without an enforcement location. Use this pattern:

```text
Invariant: required module dependencies form an acyclic graph.
Enforced at: manifest validation and runtime registry activation.
Failure: ML_GRAPH_REQUIRED_CYCLE; activation is rejected atomically.
Evidence: cycle fixture, transitive graph test, and CI validation job.
```

For each invariant, add at least one invalid fixture. Valid-only tests prove
that one path works; they do not prove that invalid state is rejected.

## Resilience levels

| Level | Required properties |
| --- | --- |
| R0 | Errors are returned and resources are released on the normal failure path |
| R1 | Invalid input, cancellation, timeout, and missing capability are tested |
| R2 | Process restart, device loss, corrupt cache, partial output, and repeated invocation are handled |
| R3 | Fault injection, fuzz/property tests, portability matrix, and recovery evidence exist |

Core lifecycle, module loading, asset ingestion, GPU submission, audio
callbacks, and persistent-data migrations target R2 or higher. Research
prototypes may remain R0/R1 only when marked experimental and excluded from
production profiles.

## Numerical and physical claims

Numerical documentation must include:

- Dimensional analysis and canonical units.
- Conditioning risks and stable evaluation form.
- Precision and accumulation policy.
- Boundary, degenerate, singular, and overflow behavior.
- Convergence metric and stopping rule.
- Reference solution or analytical fixture.
- Absolute and relative tolerances with rationale.
- Uncertainty propagation and confidence reporting.
- A declared model-validity range and fallback.

Energy conservation, reciprocity, non-negative probability density,
transmittance bounds, monotonic attenuation, and causality are common
properties, but each algorithm must state which apply and how they are tested.

## Review checklist

Before accepting a documentation change, reviewers should be able to answer:

- Which statements are guarantees, measurements, assumptions, or targets?
- Can a clean checkout reproduce the claimed command and artifact?
- Is every external input validated before it reaches native or GPU code?
- Are ownership, thread affinity, cancellation, and shutdown order explicit?
- What happens on unsupported hardware, corrupt data, timeout, device loss, or
  partial failure?
- Is the fallback measurably different and clearly reported?
- Is there a negative test for each stated invariant?
- Are links, schemas, generated outputs, and version references current?

## Evidence records

For substantial changes, attach an evidence record to the change request:

```yaml
claim: "Voxel cache never reports stale data as valid"
status: measured
implementation:
  - modules/spectral/rendering-backends.md
tests:
  - cache_stale_brick_rejected
  - revision_mismatch_returns_invalid
command: "ctest --test-dir build/debug --output-on-failure"
environment:
  os: linux
  gpu: "recorded by test artifact"
result: passed
limitations:
  - "Does not cover device loss during compaction"
follow_up: "Add allocation-failure injection"
```

Evidence records may be generated or stored with CI artifacts, but a document
must link to the stable test or command that produced the evidence. Never
claim that a test passed when it was unavailable on the current platform.
