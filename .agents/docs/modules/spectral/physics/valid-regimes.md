# Valid regimes

**Status:** Normative draft

Validity is a runtime contract, not a comment. A model advertises bounds in
length scale, wavelength, bandwidth, coherence, incidence angle, index,
roughness, polarization, geometry, and numerical resolution.

## Regime record

```yaml
model: thin-film-transfer-v1
valid:
  feature_to_wavelength: [0, 1.0e6]
  layer_count: [1, 128]
  index_imaginary: [0, 10]
requires: [planar_layers, complex_index, coherent_source]
excludes: [lateral_patterning, time_varying_medium]
fallback: maxwell-fdtd-v1
diagnostics: [SPC_MISSING_COMPLEX_INDEX, SPC_MODEL_OUT_OF_REGIME]
```

Geometric optics is appropriate when relevant features and curvature are large
relative to wavelength and phase coherence is not an observable. Wave optics is
needed for diffraction, interference, thin films, sub-wavelength structure, or
coherence-sensitive polarization. Full-field Maxwell solving is reserved for
near-field coupling, resonances, dispersive/anisotropic media, or when boundary
fields are the observable.

## Enforcement

At model selection, validate all ranges and required fields. At runtime, track
the worst margin to a bound and expose it in diagnostics and manifests. The
engine may return `approximation_with_warning` only when the caller explicitly
allows it and the error budget includes the extrapolation. Otherwise return
`unsupported_regime` with the first violated bound and a suggested fallback.

Test boundary, just-inside, just-outside, and missing-data cases for every
catalog entry. A result generated outside a declared range cannot be used as a
reference datum.
