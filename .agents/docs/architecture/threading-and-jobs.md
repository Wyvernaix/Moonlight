# Threading and jobs

**Status:** Draft

## Execution model

The host owns a bounded worker pool and explicit queues: `main` (lifecycle and
UI), `render` (GPU submission), `worker` (parallel jobs), `IO` (blocking
filesystem/network), and `audio` (real-time callback). A job declares its
queue, dependencies, cancellation token, deadline and ownership of outputs.

```text
Queued → Running → Succeeded
                 ↘ Failed
                 ↘ Cancelled
```

Dependencies form a DAG. Workers may steal only jobs marked thread-safe.
Services document affinity and reentrancy; violations are diagnostics in debug
and rejected where possible in release.

## Real-time contract

Audio callbacks must be bounded, lock-free with respect to blocking locks,
allocation-free, filesystem-free and free of logging I/O. They read an atomic
snapshot or preallocated ring buffer and enqueue work for workers. Backpressure
uses drop/coalesce policies, never unbounded growth.

## Failure and testing

Cancellation is cooperative and idempotent. A deadline yields `TimedOut`;
dependents are cancelled unless they declare a fallback. Panics/exceptions are
captured at job boundaries and reported to [observability](observability.md).
Test ordering, cancellation, starvation, queue saturation, affinity,
deterministic replay and audio underrun behavior under load.
