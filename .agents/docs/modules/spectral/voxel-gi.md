# Spectral Voxel GI

**Status:** Design target
**Sources:** `physics/rendering-and-voxel-gi.md`

Voxel GI is a first-class acceleration module for interactive and hybrid
rendering. It stores a spatial approximation of occupancy, material summaries,
visibility, and indirect illumination. It does not own authoritative material
physics and it does not replace the reference path tracer.

## Representation

A dense grid scales as \(N^3\), so the module supports:

- Sparse voxel octrees for sparse static environments.
- Cascaded clipmaps for large dynamic worlds.
- Bricked sparse grids for GPU updates and streaming.

Representations can be combined. Fine levels preserve narrow corridors and local
occlusion; coarse levels preserve broad illumination without double counting
overlapping levels.

## Voxel payload

The payload may include:

- Occupancy or coverage.
- Surface orientation.
- Authoritative material reference.
- Spectral reflectance and emission summaries.
- Directional radiance.
- Visibility or distance information.
- Transmittance/extinction for actual volume fields.
- Confidence and age.
- Links to neighboring or coarser levels.

Surface coverage must not be interpreted as physical extinction. Participating
media use wavelength-dependent transmittance such as
\(T=e^{-\sigma_t\Delta s}\).

## Pipeline

1. Voxelize scene geometry and material references.
2. Inject direct light.
3. Propagate or update indirect radiance.
4. Cone-trace directional footprints.
5. Combine cache estimates with traced paths using a defined estimator.
6. Track dirty regions, confidence, age, and invalidation.

Directional radiance may use lobes, spherical harmonics, directional samples, or
adaptive angular resolution. Spherical harmonics are useful for diffuse light
but do not preserve sharp specular directions.

Spectral caches can use validated basis coefficients, wavelength bands, or
adaptive samples. Basis reconstruction error is measured and reported.
Complex fields and phase are not averaged into ordinary radiance voxels.

## Public contract

The conceptual interface is:

```cpp
struct VoxelGIQuery {
    Vector3 worldPosition;
    Vector3 surfaceNormal;
    float wavelengthMeters;
    Vector3 outgoingDirection;
};

struct VoxelGIResult {
    SpectralRadiance radiance;
    float confidence;
    bool cacheValid;
};
```

Production code must define synchronization, coordinate conventions, memory
ownership, invalidation, GPU layout, tolerances, and revision compatibility.

## Anti-leakage and reference tests

Test thin walls, narrow corridors, dynamic movement, transparent surfaces,
multi-level transitions, energy conservation, stale cache invalidation, and
double-count prevention. Compare against converged reference scenes and report
the approximation budget rather than presenting voxel output as ground truth.
