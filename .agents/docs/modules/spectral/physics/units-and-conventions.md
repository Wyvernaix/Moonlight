# Units and conventions

**Status:** Normative draft

## Canonical representation

| Quantity | Internal unit/convention |
|---|---|
| length, wavelength | metre (m) |
| time, exposure | second (s) |
| frequency | hertz; angular frequency is rad/s |
| radiometric power | watt (W), radiance is W·m⁻²·sr⁻¹ per declared spectral measure |
| field | complex SI \(E\) in V/m and \(H\) in A/m |
| phase | radians, wrapped only at presentation |
| angles | radians internally; degrees only at boundaries |
| index, albedo, Stokes degree | dimensionless |

Spectral values declare whether they are per metre, nanometre, hertz, or
wavenumber. Conversion uses \(S_\nu = S_\lambda\lambda^2/c\); silently treating
per-nm data as per-m data is a factor-of-\(10^9\) defect.

## Coordinates and signs

World space is right-handed, Y-up, metres. A surface normal points toward the
incident medium's exterior; incoming direction points toward the interface and
outgoing points away. The baseline time convention is
\(e^{-i\omega t}\), so a passive complex index is \(n+i k\), \(k\ge0\), for the
documented propagation sign. Handedness, up-axis, and time convention are
serialized in every scene and field file.

## Normalization and precision

The rendering equation uses radiance and projected solid angle; PDFs are with
respect to the measure stated by the sampler. Stokes vectors use
\(S_0\) as total intensity and the chosen basis is transported with the ray.
Spectral tables are wavelength-sorted and never normalized unless a field named
`normalization` says how.

Use float64 for reductions, phase accumulation, material preprocessing, and
reference tests. Float32 is permitted for bounded real-time paths after an
error-budget test. FDTD uses a dimensionless Courant number below its
stability limit; FEM/BEM tolerances and conditioning diagnostics are recorded.

## API boundary

```cpp
struct SpectralSample { double wavelength_m; double value; Unit unit; };
struct Convention { Handedness handedness; Axis up; TimeSign time_sign;
                    SpectralMeasure measure; Precision precision; };
```

Reject mixed conventions at asset load. Include `unit`, `measure`,
`normalization`, and `convention_id` in every serialized array.
