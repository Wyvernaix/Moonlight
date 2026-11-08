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
