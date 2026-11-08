# Experiment manifest

An experiment is identified by an immutable manifest, not by a directory name
or a rendered image. The manifest is YAML (UTF-8) and is validated before a
solver starts.

```yaml
schema: moonlight.experiment/1
experiment_id: fresnel-slab-2026-01
question: "Does the dielectric interface conserve spectral energy?"
observable: {name: reflectance, unit: "1", wavelength_m: [4.0e-7, 7.0e-7]}
source: {repository: "Moonlight", commit: "…", toolchain: "…"}
scene: {atlas_snapshot: "sha256:…", asset_manifest: "sha256:…"}
models: [fresnel-dielectric-v1]
datasets: [{id: refractive-index-glass, version: "2026.01",
            sha256: "…"}]
solver:
  backend: reference-path-tracer
  precision: float64
  samples: 1048576
  seeds: [17, 23, 41]
  tolerances: {absolute: 1.0e-5, relative: 1.0e-4}
conditions: {temperature_K: 293.15, pressure_Pa: 101325}
camera: {sensor: "linear-spectral", response_manifest: "sha256:…"}
outputs: {format: "exr", checksums: "outputs.sha256"}
provenance: {license: "CC BY 4.0", citations: ["doi:…"]}
```

Required fields are `schema`, `experiment_id`, question/observable, source
commit, scene and dataset hashes, model/solver versions, numerical settings,
conditions, seeds (when stochastic), output format, and provenance. The
manifest also records mesh/voxel resolution, boundary conditions, GPU
capabilities, camera/color transforms, and uncertainty when those apply.

The runner stores the exact manifest beside logs and outputs. A changed input,
toolchain, or default creates a new experiment ID. A manifest can reference
large immutable objects but may not reference a mutable “latest” URL.
