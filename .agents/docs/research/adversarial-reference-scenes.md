# Adversarial and reference scene suites

**Status:** Design target  
**Scope:** Small, versioned scenes for proof obligations and regressions

The suite is intentionally synthetic. Each scene has a manifest, canonical
Atlas snapshot, expected invariant checks, valid model regimes, and a
deterministic seed. Large measured datasets remain external immutable
artifacts and are referenced by hash. Results follow the
[experiment manifest](experiment-manifest.md) and [workflow](workflow.md).

## Scene families

| ID | Purpose | Required observables |
|---|---|---|
| `atlas-hierarchy` | parent order, reparenting, generation safety | transform/hash equality, cycle rejection |
| `optical-slab` | interface and absorption sanity | `R+T+A`, Fresnel/Brewster, Beer–Lambert slope |
| `optical-wave` | phase and resolution | fringe position, phase error, refinement slope |
| `acoustic-room` | paths, reciprocity, reverberation | delay, energy bound, RT60 by band |
| `cross-material` | prevent optical/acoustic aliasing | independent channel hashes and units |
| `voxel-boundary` | clipmap/cache boundaries | dirty-region coverage, stale-brick rejection |
| `backend-parity` | CPU/GPU execution | deterministic exact fields and tolerant observables |
| `invalid-inputs` | failure semantics | typed error, no result publication |

Each family includes empty, minimal, nominal, maximum, and just-outside-regime
variants. The invalid set covers NaN/Inf, duplicate GUIDs, cycles, singular
transforms, missing data, zero-width spectra, degenerate geometry, impossible
calibration, non-convergent solver settings, out-of-memory injection, and
device loss. Expected diagnostics include the violated invariant, entity or
record, snapshot revision, and suggested fallback where applicable.

## Execution contract

Run every scene on the CPU reference and all supported GPU backends with fixed
scene/data hashes, seeds, sample counts, precision, compiler/toolchain, and
measurement definitions. Run at least two resolutions and three seeds for
stochastic scenes. Canonical metadata, schema, units, and provenance must
match exactly. Numerical results use the manifest's absolute/relative
tolerance and uncertainty interval; no tolerance is changed per backend.

Archive the manifest, input hash, logs, metrics, residuals, invariant report,
output digest, hardware/driver report, and validity status. A failure is
actionable only when the suite identifies whether it is a model-boundary,
dimensional, conservation/reciprocity, convergence, uncertainty, numerical,
parity, or infrastructure failure. Never treat a black/empty output as a
valid zero unless the scene's observable explicitly has that value.
