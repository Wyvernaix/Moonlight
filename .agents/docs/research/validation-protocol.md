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

Each protocol names fixture provenance, expected result, command, and schema
version. Calibrated optical data should cite the relevant
[CIE](https://cie.co.at/) publication; color exchange metadata should follow
[W3C Color on the Web](https://www.w3.org/Color/). Acoustic measurements must
include microphone, sample rate, calibration tone, room condition, and band
definition. Validation artifacts are immutable and linked from the experiment
manifest.
