# Fault injection and recovery

**Status:** Design target

Fault injection validates that failures follow the documented state machines
instead of merely exercising happy paths. Injection is test-only and must be
explicitly enabled; production inputs cannot activate it.

## Failure taxonomy

| Category | Examples | Retry | Required outcome |
| --- | --- | --- | --- |
| `InvalidInput` | Schema, range, or state violation | No | Reject without mutation |
| `Incompatible` | ABI, platform, or version mismatch | No | Isolate provider and explain remediation |
| `Unavailable` | Missing capability, worker, or sink | Maybe | Select documented fallback or degrade |
| `Transient` | Timeout, interrupted I/O, backpressure | Bounded | Retry with deadline and jitter |
| `Corrupt` | Hash/decode/serialization failure | No automatic retry | Quarantine and preserve prior valid state |
| `Internal` | Assertion, panic, unexpected exception | No | Capture evidence, fail closed, recover host if possible |
| `Security` | Signature, policy, traversal, quota violation | No | Deny and avoid sensitive detail |

Every error has a stable category, operation, correlation ID, retryability,
deadline, and redacted diagnostic. Categories must not be inferred from text.

## Recovery invariants

- A failed operation is atomic: either its postcondition holds or observable
  state is unchanged.
- Rollback follows reverse dependency order and is idempotent.
- A timeout never extends the caller's deadline.
- The last known-good snapshot/catalog remains usable after a failed update.
- Recovery cannot bypass ABI, signature, schema, or capability checks.
- Repeated fault injection yields the same state transition for the same seed.

## State and scenario matrix

| Injection point | Expected transition | Evidence |
| --- | --- | --- |
| Manifest parse/graph cycle | `Validated → Failed` | diagnostic + no load |
| Plugin hash/signature | `Admitted → Rejected` | security event |
| Dependency start | dependent rollback | reverse `module.stop` spans |
| Snapshot extraction | `Extracting → Failed` | previous snapshot remains current |
| Asset read/decode | `Fetching → Rejected` | quarantine + `asset.hash_mismatch` |
| Worker deadline | `Running → TimedOut` | cancellation and queue metric |
| Observability sink | `Active → Degraded` | local counter, no runtime block |
| Audio queue saturation | bounded drop/coalesce | underrun/backpressure metric |

Design-target recovery budgets (to be measured per platform) are: no leaked
handles, no callbacks after unregister, and shutdown completion by the host
deadline. A test fails if any budget is exceeded.

## Test protocol

Fault runs record seed, injection point, build/toolchain digest, state-machine
trace, logs/metrics/traces, and final resource census. Run every scenario with
single-threaded deterministic scheduling and with stress scheduling. Recovery
tests must include repeated stop, cancellation races, process restart, partial
asset stores, and unavailable telemetry sinks.

See [module lifecycle](module-lifecycle.md), [data formats](data-formats.md),
[observability](observability.md), and [security](security.md).
