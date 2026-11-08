# Orcha: acoustic simulation and audio

**Status:** Design target
**Sources:** `design-specification.md`

Orcha is Moonlight's physically grounded audio subsystem. It separates acoustic
propagation, spatial rendering, DSP, content management, and Workbench tools.
The listener result depends on source, listener, geometry, materials,
environment, and propagation method—not only on a hand-authored volume curve.

## Module boundaries

| Module | Responsibility |
| --- | --- |
| `Orcha.Acoustics` | Direct paths, reflections, transmission, diffraction, room response |
| `Orcha.Spatial` | HRTF, binaural, Ambisonics, head tracking, source directivity |
| `Orcha.DSP` | Real-time mixing, filters, convolution, routing, output |
| `Orcha.Sources` | Audio assets, events, voices, procedural sources |
| `Orcha.Workbench` | Authoring, path inspection, baking, profiling, diagnostics |

Atlas supplies scene snapshots. Core supplies jobs, assets, platform services,
and scheduling. Workbench is a client and never a runtime dependency.

## Propagation regimes

| Regime | Purpose |
| --- | --- |
| Geometric acoustics | Direct paths, visibility, reflections, portals, suitable reverb |
| Hybrid wave-aware | Edge diffraction, waveguides, low-frequency corrections |
| Numerical wave solving | FDTD, FEM, BEM, and specialist offline studies |

An acoustic ray represents an energy path, not a complete pressure wave. The
selected solver and its validity limits must be visible in diagnostics.

## Runtime pipeline

1. Read source/listener transforms from Atlas.
2. Test direct visibility and compute distance/delay.
3. Apply air absorption and frequency-dependent material transmission.
4. Trace early reflections and estimate diffuse/late energy.
5. Search diffraction or portal paths when enabled.
6. Build a revisioned impulse-response representation.
7. Spatialize each arriving path.
8. Publish immutable results to the real-time DSP graph.

Results contain path length, delay, arrival direction, path kind, transfer
handle, confidence, scene revision, and simulation revision. No audio callback
waits for simulation workers.

## Acoustic materials

Materials may define frequency-dependent:

- Absorption.
- Scattering.
- Transmission loss.
- Complex impedance.
- Thickness and layers.
- Density, elasticity, porosity, and flow resistivity.
- Measurement provenance and uncertainty.

Optical and acoustic materials may share asset/provenance infrastructure, but
their physical properties are distinct. A “concrete” preset is an authoring
convenience, not universal measured truth.

## Spatial rendering

Support direct and reflected arrival directions, measured HRTF/SOFA data,
head-tracked binaural output, Ambisonics, speaker layouts, directivity,
distance cues, ITD/ILD, Doppler, and output-device calibration. Reflections
must be spatialized from their arrival paths rather than from the original
source position.

## Operating profiles

- **Interactive:** bounded direct-path queries, selective tracing, async updates.
- **Production:** higher-quality reflections, material transmission, baking,
  cached impulse responses.
- **Acoustic Lab:** calibrated data, wave solvers, controlled boundaries,
  convergence studies, and exportable research results.

Artistic overrides are allowed but must be labelled separately from physically
derived settings.

## Acoustic validation and units

Internal lengths are metres, time is seconds, pressure is pascals, and
frequency is hertz. Impulse responses declare sample rate, channel order,
reference pressure, calibration gain, band filters, and room temperature.
Material coefficients are frequency-dependent and retain measurement
conditions and uncertainty; optical material fields are never reused as
acoustic absorption.

Every propagation result includes a scene revision, solver ID, band limits,
path-length tolerance, seed, and confidence. Validation uses direct-path delay,
energy conservation, reflection/transmission against analytic slabs, and
measured room impulse responses (including RT60 by band). Compare at least two
mesh/ray resolutions and report microphone calibration and background noise.
Unsupported low-frequency wave behavior must select a documented hybrid/wave
solver or produce a validity diagnostic rather than an apparently precise IR.

## Proof obligations and failure semantics

Every path carries metres, seconds, hertz, pascals, and a declared reference
pressure. Check the dimensional identity `delay_s = path_length_m / c_m_s`,
that a transfer function is dimensionless relative to its calibration, and
that air/material losses are non-negative. For a closed passive test room,
integrated outgoing energy cannot exceed injected energy after accounting for
the source and microphone calibration. At a reciprocal boundary, swapping
source and listener must produce the same transfer magnitude and reversed
arrival direction within the declared tolerance; non-reciprocal media must
declare the exception.

Convergence is reported by increasing rays/order, mesh resolution, or solver
time step until the observable and its uncertainty meet the manifest
tolerance. FDTD time steps satisfy the solver's Courant bound; convolution
buffers and sample-rate conversions are checked for finite latency and no
aliasing. Monte Carlo results include seed, effective samples, confidence
interval, and a zero-safe relative error. A failed bound, missing acoustic
material, invalid calibration, stale snapshot, or non-finite IR returns a
typed failure (`ORCHA_OUT_OF_REGIME`, `ORCHA_NONCONVERGED`, or
`ORCHA_INVALID_INPUT`) and no result is published.

Reference tests run the same Atlas snapshot through CPU and GPU backends,
with deterministic seeds and fixed channel ordering. GPU reductions may differ
in the last bits, so acceptance uses physical tolerances and confidence
intervals; deterministic impulse-response metadata and path classifications
must still match. Adversarial fixtures include zero-distance source/listener,
coincident and silent sources, grazing paths, disconnected rooms, zero-thickness
and perfectly absorbing walls, extreme temperatures, NaN material tables,
sample-rate changes, and truncated impulse responses.

The shared [numerical robustness](../../research/numerical-robustness.md) and
[cross-module validation](../../research/cross-module-validation.md)
protocols define parity, uncertainty, and failure handling.
