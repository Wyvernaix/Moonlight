# Reproducible research workflow

**Status:** Normative protocol

1. State the question, observable, units, acceptance tolerances, and validity
   regime before implementation.
2. Pin the repository commit, toolchain/container digest, GPU/CPU, dataset
   manifests, scene revision, and random seeds.
3. Generate an experiment manifest and run a dry validation of inputs.
4. Execute a baseline and at least one refinement or independent implementation.
5. Archive logs, configuration, numerical residuals, uncertainty estimates, and
   output digests. Never overwrite a run.
6. Publish a short interpretation that distinguishes measured, simulated,
   inferred, and artistic values, plus known limitations.

The canonical run directory is `runs/<experiment-id>/` and contains
`manifest.yaml`, `inputs.json`, `stdout.log`, `metrics.json`, `outputs.sha256`,
and `README.md`. Large binaries are referenced by immutable object ID.

```text
create -> validate inputs -> baseline -> sweep/refine -> analyze -> review -> archive
```

The command API is intentionally boring:

```text
moonlight research validate manifest.yaml
moonlight research run manifest.yaml --output runs/<id>
moonlight research compare runs/<a> runs/<b>
moonlight research verify runs/<id>
```

`verify` checks hashes, schema, licenses, environment, and reproducibility
metadata. It does not claim that a numerical result is physically correct.
