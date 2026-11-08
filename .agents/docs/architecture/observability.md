# Observability

**Status:** Draft

Moonlight diagnostics are structured, correlated and safe to collect. A
`RunContext` supplies `run_id`, `trace_id`, `module_id`, `build_id` and
`snapshot_id`; every log, metric and span carries these fields where known.

## Contracts

```text
log(level, event_name, attributes)
counter(name, value, attributes)
histogram(name, value, attributes)
span(name, start, end, status, attributes)
```

Events use stable names (`module.start`, `asset.hash_mismatch`,
`job.timeout`, `snapshot.published`) and bounded attribute sizes. Secrets,
tokens, raw user content and untrusted paths are redacted. Logging must not
block the audio callback or lifecycle shutdown.

## Lifecycle and failure

`Created → Active → Flushed → Closed`; crash-safe sinks may remain `Degraded`
without blocking runtime work. Export failures increment a local counter and
surface one rate-limited diagnostic. Trace sampling is deterministic when a
research seed is supplied.

Metrics include module start failures, job latency/queue depth, asset cache
hits, frame time, audio underruns, memory high-water mark and snapshot age.
Health probes report `Ready`, `Degraded` or `Failed` with remediation.

Tests assert schema validity, correlation, redaction, bounded queues,
shutdown flush, sampling determinism and behavior when the sink is unavailable.
See [module lifecycle](module-lifecycle.md), [threading and jobs](threading-and-jobs.md)
and [security](security.md).

## Evidence requirements

The minimum evidence for a lifecycle or fault test is one correlated event,
one metric change, and (where a span exists) a terminal span status. Required
attributes are `run_id`, `build_id`, `module_id`, operation, outcome, and
monotonic duration; `snapshot_id` is required for snapshot operations. Missing
correlation or unbounded attributes fail the contract test.

Design target SLOs must be stated as measurable bounds (for example, queue
depth, shutdown flush deadline, or audio underrun rate), with platform and
workload recorded. They are not production guarantees until a release
evidence bundle demonstrates them.
