# Validation protocol

Validation compares an implementation with an analytic result, a converged
independent implementation, or a calibrated measurement. Visual similarity
alone is not validation.

## Test matrix

| Family | Fixture | Observable | Required checks |
|---|---|---|---|
| interface | dielectric slab | Fresnel R/T vs angle and wavelength | energy, Brewster angle, polarization |
| volume | homogeneous absorber | transmittance | Beer–Lambert slope and units |
| dispersion | known glass | phase/group index | interpolation and range rejection |
| polarization | polarizer/retarder | Stokes/Mueller output | degree of polarization and basis |
| wave | double slit/thin film | intensity fringes | phase, convergence, fringe position |
| solver | cavity/PML | field/residual | convergence, boundary reflection, energy |
| camera | calibrated chart | sensor/XYZ/RGB | response, white point, transfer metadata |
| acoustics | measured room IR | RT60/arrival paths | bandwise error and confidence |

For every fixture run at two resolutions and three seeds where stochastic.
Compare scalar observables with absolute/relative tolerances and images in
linear light with a documented color transform. Include uncertainty intervals,
runtime, memory, hardware, and all validity diagnostics. A failed test records
the failure; it is not hidden by changing the tolerance.

## Invariants, dimensions, and parity

Before comparing values, verify dimensions and normalization: lengths are
metres, delays seconds, frequencies hertz, spectral measures are explicit, and
zero-valued observables use an absolute scale floor. Passive optical fixtures
check `R + T + A = 1`; acoustic fixtures check non-negative loss and bounded
outgoing energy. Reciprocal fixtures swap source and receiver and compare the
transfer magnitude and reversed direction. Exceptions require a declared
non-reciprocal medium or active source.

The CPU reference and GPU backend use identical canonical inputs, seeds, sample
counts, and output definitions. Deterministic fixtures require exact canonical
metadata and field equality. Stochastic or reduction-order-dependent paths
compare confidence intervals and physical tolerances, while recording device,
driver, compiler, precision, and fast-math settings. A backend that cannot
meet the declared invariant returns a typed failure rather than a clamped or
partial result.

Each refinement must show convergence of the observable and uncertainty, not
just a smaller residual. Propagate material, calibration, discretization,
statistical, and round-off uncertainties; retain covariance or use a
conservative bound when independence is not established. Exercise boundary
and just-outside validity cases, NaN/Inf data, degenerate geometry, zero
distance, singular transforms, missing assets, stale revisions, and simulated
device loss. Expected failure codes and diagnostic context are part of the
fixture's acceptance record.

Each protocol names fixture provenance, expected result, command, and schema
version. Calibrated optical data should cite the relevant
[CIE](https://cie.co.at/) publication; color exchange metadata should follow
[W3C Color on the Web](https://www.w3.org/Color/). Acoustic measurements must
include microphone, sample rate, calibration tone, room condition, and band
definition. Validation artifacts are immutable and linked from the experiment
manifest.
