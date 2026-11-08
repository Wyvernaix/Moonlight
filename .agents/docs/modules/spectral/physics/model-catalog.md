# Spectral model catalog

**Status:** Normative draft  
The catalog is the review index for models. A model is not production-ready
until its entry has a validity range, an independent reference, and a test
case. The reference path tracer remains independent from voxel GI and caches.

## Catalog schema

```yaml
id: fresnel-dielectric-v1
kind: interface
equations: [fresnel_unpolarized]
inputs: {n: dimensionless, cos_theta_i: [0, 1]}
outputs: {R: [0, 1], T: [0, 1]}
assumptions: [isotropic, non-magnetic, planar, lossless]
method: analytic
validity: {wavelength_m: [3.8e-7, 7.0e-7], roughness: "zero"}
reference: {type: analytic, source: "Born and Wolf, Optics"}
tests: [fresnel_normal_incidence, brewster_angle, energy_conservation]
status: validated
```

`id` and equation versions are stable API identifiers. Changing an equation,
default, interpolation rule, or random sampling sequence requires a new
version and a migration note.

## Baseline entries

| ID | Method | Preserves | Explicit limitation |
|---|---|---|---|
| `fresnel-dielectric-v1` | analytic interface equations | energy, polarization when enabled | planar interface; no roughness |
| `sellmeier-v1` | tabulated/analytic dispersion | wavelength-dependent phase index | coefficients are material-specific |
| `beer-lambert-v1` | exponential attenuation | spectral energy | homogeneous path; no multiple scattering |
| `microfacet-ggx-v1` | Monte Carlo BRDF/BSDF | reciprocal scattering when sampled correctly | geometric optics; correlation is approximate |
| `hg-phase-v1` | analytic volume phase function | normalized single scatter | not a guarantee of wave diffraction |
| `thin-film-transfer-v1` | complex 2×2 transfer matrices | coherent phase and interference | planar layers and known complex index |
| `stokes-mueller-v1` | 4-vector/4×4 matrices | partial polarization | cannot recover field phase |
| `voxel-radiance-v1` | discretized cache | bounded approximate irradiance | leakage and resolution bias |
| `maxwell-fdtd-v1` | Yee time-domain grid | full fields in its band | CFL, mesh dispersion, finite boundaries |

## Selection contract

The caller supplies `observable`, `scale_m`, `band_hz`, `coherence`, and
`polarization`. Selection returns a model ID, required input fields, estimated
error, and diagnostics. A coherent or polarization-sensitive request must not
silently select scalar radiance transport. Unsupported inputs return
`SPC_MODEL_OUT_OF_REGIME`.

```cpp
ModelChoice choose_model(const Observable&, const SceneSnapshot&,
                         const AccuracyBudget&);
Result evaluate(const ModelChoice&, Span<const Sample>);
```

The choice and model version are persisted in experiment manifests. See
[PBRT](https://pbr-book.org/) and
[Mitsuba](https://mitsuba-renderer.org/) for useful independent renderer
architecture comparisons; neither substitutes for Moonlight validation.
