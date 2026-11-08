# Numerical solver selection

Select the least expensive method that preserves the observable and fits its
validity regime. Record the decision; do not infer it from performance.

| Observable/problem | Preferred method | Main controls and failure modes |
|---|---|---|
| broadband radiance and visibility | spectral path tracing | samples, PDFs, MIS, fireflies, geometric-optics scale |
| cached diffuse transport | voxel/radiance cache | voxel size, leakage, update age, anisotropy |
| thin films/interference | transfer matrix or wave solver | phase convention, complex index, layer thickness |
| periodic or near-field EM | FDTD/FEM/BEM | mesh dispersion, CFL/conditioning, boundary/PML reflection |
| polarization | Jones for coherent fields; Stokes/Mueller for partial coherence | basis transport, depolarization, matrix physicality |
| room impulse response | geometric/hybrid acoustics | band limits, diffraction approximation, material absorption |

FDTD requires a mesh resolving the shortest wavelength and a time step below
the CFL limit; FEM requires element-order/mesh refinement and residual checks;
BEM requires a well-conditioned boundary formulation and an open-boundary
treatment. Report solver version, linear algebra backend, preconditioner,
stopping criteria, boundary conditions, and estimated discretization error.

The selection API returns `{solver_id, assumptions, controls, estimated_cost,
validity, fallback}` and fails if the observable cannot be represented.
References: [Khronos Vulkan](https://registry.khronos.org/vulkan/) for explicit
GPU/device capability contracts, [PBRT](https://pbr-book.org/), and
[Mitsuba](https://mitsuba-renderer.org/).
