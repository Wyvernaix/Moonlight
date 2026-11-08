# Material data provenance

Material properties are versioned observations with conditions and uncertainty.
An authoring name such as “glass” is not a measurement.

## Record schema

```yaml
material_id: glass/window
revision: 3
properties:
  - name: complex_refractive_index
    dataset: refractive-index-glass@2026.01
    columns: [wavelength_nm, n, k, sigma_n, sigma_k]
    range_nm: [400, 700]
    interpolation: linear_wavelength
conditions: {temperature_K: 293.15, incidence: normal}
processing: {script: convert.py, commit: "…", input_sha256: "…"}
uncertainty: {type: covariance, file: "…"}
license: "CC BY 4.0"
```

Keep raw and processed tables, conversion scripts, and checksums. Document
digitization, smoothing, resampling, extrapolation, and unit conversion.
Reject missing conditions for measurement-grade experiments. Optical,
acoustic, and thermal properties may share an asset identity but never share
unnamed numeric fields.

Reviewers should be able to follow `material -> dataset -> publication ->
measurement conditions -> processing commit`. See NIST's
[Atomic Spectra Database](https://physics.nist.gov/asd) and
[refractiveindex.info](https://refractiveindex.info/) as examples of sources;
their terms and record-level applicability must be checked before import.
