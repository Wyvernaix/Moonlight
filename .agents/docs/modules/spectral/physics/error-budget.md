# Spectral error budget

**Status:** Normative draft  
**Scope:** Spectral transport, wave optics, polarization, and Maxwell
experiments. This document describes what must be measured; it does not choose
a quality target for a particular product.

## Error model

For an observable \(y\), report

\[
 e_\mathrm{total} = e_\mathrm{model}+e_\mathrm{disc}
 +e_\mathrm{stat}+e_\mathrm{round}+e_\mathrm{data}+e_\mathrm{instrument}.
\]

The terms are not assumed independent. When independence is justified, combine
standard uncertainties in quadrature; otherwise publish the covariance or a
conservative bound. A comparison is incomplete without the observable, units,
normalization, wavelength sampling, confidence level, and reference path.

| Term | Typical cause | Measurement |
|---|---|---|
| Model | geometric optics used for a sub-wavelength feature | compare with wave/full-field reference in the valid overlap |
| Discretization | finite rays, voxels, mesh, or time step | Richardson-style refinement or a convergence plot |
| Statistical | Monte Carlo samples and Russian roulette | repeated seeds; mean and 95% confidence interval |
| Round-off | float precision, ill-conditioned solve | float32/64 comparison and residual |
| Data | interpolation, calibration, or unknown material provenance | source uncertainty and sensitivity sweep |
| Instrument | sensor response, display transform, or microphone | calibrated response and calibration uncertainty |

## Required report

Every benchmark emits a machine-readable record:

```yaml
observable: hemispherical_reflectance
quantity: dimensionless
region: {lambda_nm: [400, 700], angle_deg: [0, 80]}
reference: {method: independent_path_tracer, commit: "…", tolerance: 1.0e-5}
estimate: {value: 0.7341, standard_uncertainty: 0.0012, coverage: 0.95}
components: {model: 0.0, discretization: 0.0007, statistical: 0.0008,
             roundoff: 0.00001, data: 0.0005}
settings: {seed: 17, samples: 4194304, precision: float64}
validity: pass
```

For images report linear-light spectral or tristimulus error (not only a
perceptual score): mean absolute error, relative error with a zero-safe
denominator, max error, and a 95th percentile. For fields also report
\(\|\nabla\times E-i\omega B\|\), boundary residual, and energy balance.

## Acceptance and diagnostics

Each test declares `absolute_tolerance`, `relative_tolerance`, and the
confidence interval policy. If the denominator is below `scale_floor`, use an
absolute tolerance. A failed convergence or an out-of-regime input is a
diagnostic (`SPC_VALIDITY`, `SPC_CONVERGENCE`, or `SPC_DATA_UNCERTAIN`), never a
silent pass. Performance (wall time, peak memory, GPU, and cache state) is
reported separately from physical error.

Reference datasets and scripts are versioned using the manifest in
`../../../research/experiment-manifest.md`. Cite PBRT's validation-oriented renderer
documentation and Mitsuba's differentiable/spectral renderer as independent
implementation references, not as ground truth:
[PBRT](https://pbr-book.org/), [Mitsuba](https://mitsuba-renderer.org/).
