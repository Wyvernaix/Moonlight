# Reference data

**Status:** Normative draft

Reference data is an input with uncertainty, not an unqualified truth. Large
files and rendered outputs live in controlled storage or CI artifacts; the
repository contains a manifest, acquisition/processing code, and small
fixtures.

## Manifest

```yaml
dataset_id: refractive-index-glass
version: 2026.01
uri: https://example.invalid/object
sha256: "64 hexadecimal characters"
license: "CC BY 4.0"
source: {publisher: "…", citation: "…", accessed: "2026-01-15"}
conditions: {temperature_K: 293.15, pressure_Pa: 101325}
schema: {columns: [wavelength_nm, n, k, sigma_n, sigma_k], encoding: utf-8}
processing: [{tool: "scripts/convert_material.py", commit: "…"}]
valid_range: {wavelength_nm: [400, 700]}
```

CI verifies the SHA-256 digest, schema, license allow-list, monotonic
wavelength ordering, finite values, and declared range. A URI alone is not
provenance because remote content can change.

## Material data rules

Store original values and derived values separately. Preserve units and
conditions; never silently convert a measured reflectance into an index.
Interpolation is part of the dataset contract (linear in wavelength is not
equivalent to linear in frequency). Record extrapolation policy, missing-value
handling, smoothing, digitization uncertainty, and any Kramers–Kronig or
resampling transform. Prefer measured complex \(n+i k\), BRDF/BSDF, phase
functions, and covariance where available.

## Provenance API

```text
Dataset resolve(DatasetId, Version)
ValidatedTable load(Dataset, Schema, Range)
Digest sha256(ByteStream)
UncertaintyModel uncertainty(Dataset, Column)
```

`resolve` is immutable and offline-capable. A missing or license-incompatible
dataset fails before rendering. Cite authoritative repositories such as
[NIST Atomic Spectra Database](https://physics.nist.gov/asd) and
[refractiveindex.info](https://refractiveindex.info/) with their terms; verify
the license and measurement conditions for each imported record.
