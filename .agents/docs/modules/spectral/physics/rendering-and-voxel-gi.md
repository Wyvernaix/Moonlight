# Spectral Rendering Engine — Extended Design Specification

Moonlight Engine · Spectral v2 Architecture Proposal · October 26, 2026

## 1. The new objective: a physically grounded rendering and simulation platform

Spectral should now be designed as a hybrid spectral light-transport and optical-simulation platform, with voxel global illumination, hardware-accelerated ray tracing, measured material data, coherent wave optics, and dedicated electromagnetic solvers.

Adding voxel GI is a major architectural improvement for efficient indirect lighting, but one distinction is essential: voxel GI is not a replacement for physically rigorous light transport. It is a method for storing and approximating scene illumination in a spatial representation.

The original voxel cone tracing research by Crassin and colleagues uses a hierarchical voxel representation to estimate incoming illumination and visibility efficiently. It is explicitly an approximation intended to make global illumination practical for interactive rendering. Source: NVIDIA Research — Interactive Indirect Illumination Using Voxel Cone Tracing.

The design should therefore have two simultaneous goals:

- Maximum physical fidelity: use the appropriate physical equations, measured material properties, and numerically validated transport algorithms.
- Maximum practical performance: reuse illumination information, including voxel radiance and other caches, wherever doing so is compatible with the desired accuracy.

These goals are complementary, but they are not identical. Every acceleration technique should declare its approximation, expected error and supported physical regime.

### The target capability

Physically based scene illumination

Spectral direct and indirect lighting, multiple bounces, accurate visibility, measured emission, reflections, refraction, caustics, and participating media.

Voxel global illumination

Multiresolution scene voxelization, directional radiance, approximate cone tracing, dynamic updates, diffuse indirect-light caches, and hybrid ray/voxel visibility.

Optical and electromagnetic simulation

Spectral wavelength dependence, polarization, coherent interference, diffraction, dispersive materials, thin-film optics, and specialized full-field Maxwell solvers.

Measurement-grade image formation

Calibrated light sources, real camera and sensor models, spectral response, radiometric units, colour science, uncertainty reporting, and reference-based validation.

The governing principle: Spectral must never describe a visually plausible approximation as an exact electromagnetic simulation. The renderer should indicate whether each result is measured-data-driven, analytically modelled, stochastically estimated, cached, or computed with a numerical field solver.

# 2. Revised Spectral architecture

The expanded architecture should introduce voxel GI as a distinct subsystem while retaining the original modules for spectral path tracing, surface optics, volume transport, coherence and electromagnetic solving.

The key dependency rule is that voxel GI consumes scene geometry, material models and light-transport results. It should not become the authoritative owner of material physics.

The implementation must keep the physical models and transport methods
independent. A voxel GI implementation can be replaced without rewriting the
material system, and an electromagnetic solver can consume the same validated
material assets without inheriting the geometric path tracer's assumptions.

The architecture is designed around independent physical models and transport methods. A voxel GI implementation can be replaced or supplemented without rewriting the material system, and an electromagnetic solver can consume the same validated material assets without inheriting the assumptions of the geometric path tracer.

# 3. Spectral.VoxelGI — the new voxel global illumination system

## 3.1 What voxel GI should do

The new module, `Spectral.VoxelGI`, should create a spatially organized representation of scene occupancy, surface properties, visibility, and indirect illumination.

Instead of tracing every secondary ray all the way through the full scene, the renderer can query a hierarchical voxel structure to estimate incoming illumination over a range of directions.

This provides several useful capabilities:

- Dynamic indirect illumination in interactive scenes.
- Approximate diffuse bounce lighting.
- Efficient illumination queries over large areas.
- A spatial representation of illumination that can be updated as objects move.
- A useful cache for hybrid ray-tracing workflows.
- Rapid feedback in Workbench while artists are changing materials or lighting.

It should be a first-class rendering subsystem, but its results must be distinguishable from fully traced reference results.

## 3.2 The representation: sparse, hierarchical and multiresolution

A single dense voxel grid is not sufficient for a large enterprise scene. Its memory cost increases with the cube of the resolution.

For a grid with \(N\) cells along each axis, the total number of voxels is:

\[
N_{\mathrm{voxels}}=N^3
\]

Doubling the resolution along each axis requires eight times as many voxels. Spectral therefore needs sparse storage, multiresolution representations and streaming.

The proposed design combines three representations.

Dynamic Sparse Voxel Octrees for Next-Gen Real-Time Rendering

A. Sparse voxel octree

Suitable for sparsely occupied static or slowly changing environments. Empty regions can be represented compactly while occupied regions are subdivided to provide finer detail.

SRP-VXGI Discussion Thread · Issue #9 · Looooong/Unity-SRP-VXGI · GitHub

B. Cascaded voxel clipmaps

Suitable for large dynamic worlds. Multiple grids cover different spatial scales, with the finest levels following the active scene area and coarser levels representing more distant regions.

GigaVoxels. Voxels come into play - презентация онлайн

C. Bricked sparse voxel grids

Suitable for GPU updates and streaming. The scene is divided into small voxel blocks, allowing only affected regions to be rebuilt or refreshed.

These are alternative storage strategies within the same module, not competing physical models. Spectral can use more than one simultaneously.

For example, a large open-world game might use clipmaps around the player and a sparse hierarchy for more static parts of the environment.

### Voxel hierarchy rules

Each voxel level must define its spatial extent, resolution, filtering method and relationship to finer levels. When several levels overlap, blending must not unintentionally count the same illumination more than once.

Fine levels should preserve local occlusion, narrow corridors and small lighting features. Coarse levels should preserve the broad distribution of incoming illumination, rather than simply averaging every stored quantity indiscriminately.

Thin geometry, sub-voxel features and transparent objects need special treatment because ordinary solid voxelization can change their optical behaviour.

## 3.3 What a voxel should store

A voxel should not be just an RGB colour and an opacity value. It needs a structured representation that distinguishes the surface from the light travelling through or reflecting from the region.

| Voxel data | Purpose |
| --- | --- |
| Coverage or occupancy | Represents how geometry occupies the voxel |
| Surface orientation | Supports directional interaction and visibility estimation |
| Surface material reference | Points to the authoritative physical material |
| Spectral reflectance representation | Approximates or stores wavelength-dependent reflection |
| Spectral emission | Represents light emitted from the voxel's contents |
| Directional radiance | Represents illumination arriving from different directions |
| Visibility or distance information | Helps estimate occlusion and avoid some light leaks |
| Transmittance or extinction | Represents attenuation when using an appropriate volume representation |
| Confidence and age | Identifies stale, insufficiently sampled or uncertain cache data |
| Multilevel links | Supports lookup and filtering across voxel resolutions |

The authoritative material asset retains the original optical data. The voxel stores only the representation needed by the selected acceleration method.

For transparency, thin films, diffraction and detailed polarization effects, a voxel's compact representation generally cannot replace the full material model. Spectral should defer to ray tracing or specialized solvers when the cached description is insufficient.

# 4. Voxel GI algorithms and equations

The new module should implement the following pipeline.

1. Scene voxelization

Convert geometry and material data into the spatial hierarchy.

2. Direct-light injection

Evaluate or sample illumination from physical light sources.

3. Radiance propagation and updates

Propagate indirect illumination into the selected cache representation.

4. Directional cone tracing

Estimate incoming radiance and visibility over directional footprints.

5. Hybrid transport integration

Combine cache estimates with explicitly traced paths under a defined estimator.

## 4.1 The physical transport equation behind global illumination

Voxel GI must be grounded in the same light-transport equations as the rest of Spectral.

For a surface point \(\mathbf x\), the indirect reflected radiance is expressed in terms of incoming illumination:

\[
L_o(\mathbf x,\omega_o,\lambda)
=
L_e(\mathbf x,\omega_o,\lambda)
+
\int_{\Omega}
f_s(\mathbf x,\omega_i,\omega_o,\lambda)
L_i(\mathbf x,\omega_i,\lambda)
|\mathbf n\cdot\omega_i|\,d\omega_i
\]

This is the surface rendering equation. The direct and indirect components of incoming light can be estimated separately, but their sum must represent the correct transport solution.

Voxel GI approximates some of those incoming-light contributions using its stored spatial representation.

It does not change the underlying physical equation.

## 4.2 Voxel cone tracing

A cone represents a range of nearby directions rather than one infinitesimally thin ray. It can gather increasingly filtered voxel data as its footprint grows with distance.

Let \(C_i\) be the radiance contribution represented by a sampled voxel segment, \(\alpha_i\) the segment's effective opacity, and \(T_i\) the transmittance remaining before the segment.

Front-to-back accumulation can be written as:

\[
C_{\mathrm{acc}}
\leftarrow
C_{\mathrm{acc}}+T_i\alpha_i C_i
\]

\[
T_{i+1}=T_i(1-\alpha_i)
\]

The accumulated contribution can be interpreted as an approximation to radiance travelling along the cone through the voxel hierarchy.

For opacity represented at voxel-scale length \(d\), a step of length \(\Delta s\) can use the correction:

\[
\alpha_{\mathrm{step}}
=
1-(1-\alpha_{\mathrm{voxel}})^{\Delta s/d}
\]

This correction is appropriate under the corresponding homogeneous-opacity interpretation. A solid-surface coverage value is not automatically a physical extinction coefficient, so the engine must distinguish coverage filtering from genuine volume attenuation.

For a homogeneous participating medium, physical transmittance is instead:

\[
T(\Delta s,\lambda)=e^{-\sigma_t(\lambda)\Delta s}
\]

where \(\sigma_t\) is the extinction coefficient. A volume voxel should use this physical formulation when its data actually describes an extinction field.

## 4.3 Directional radiance storage

A major requirement is preserving directionality.

If a voxel stores only one average radiance value, it cannot adequately distinguish illumination arriving from different directions. It becomes difficult to model orientation-dependent materials and directional occlusion.

Possible representations include:

- Directional radiance lobes.
- Spherical harmonics for low-frequency angular distributions.
- Multiple directional samples.
- Higher-resolution angular representations in important regions.

Spherical harmonics can be effective for diffuse illumination because diffuse reflection filters away high-frequency directional information. They are less suitable as a complete representation of sharp specular lighting.

Spectral must support a choice of representation based on the selected rendering profile, material and storage budget.

## 4.4 Spectral voxel data

An RGB voxel representation is not sufficient for a fully spectral GI engine.

The indirect light returning from a red material, a dispersive prism or a wavelength-selective coating depends on the spectral distribution of the incoming illumination and the object's optical response.

Storing all wavelength values independently in every voxel can be prohibitively expensive. Spectral therefore needs compact spectral encodings, such as:

- Coefficients in a validated spectral basis.
- A selected number of wavelength bands.
- Sampled spectral cache entries for particular wavelengths.
- Adaptive spectral refinement in regions with sharp wavelength-dependent behaviour.

For a basis representation, stored radiance could be approximated as:

\[
L_\lambda(\mathbf x,\omega)
\approx
\sum_{j=1}^{M} a_j(\mathbf x,\omega)b_j(\lambda)
\]

where \(b_j\) are basis functions and \(a_j\) are their coefficients.

This is a storage approximation. The basis must represent the wavelengths and dynamic range required by the scene, and the approximation error must be measured. Sharp spectral features may require more basis functions or direct wavelength samples.

Important: Voxel GI should target primarily incoherent indirect illumination. It must not average complex wave fields into an ordinary radiance cache and assume that phase-dependent interference is preserved. Coherent interference needs a separate field representation.

# 5. Voxel GI module specification

Implement Voxel GI as its own library with independently replaceable stages.

| Module | Responsibilities |
| --- | --- |
| `Spectral.VoxelGI.Core` | Settings, voxel coordinates, interfaces and lifecycle |
| `Spectral.VoxelGI.Geometry` | Conservative voxelization, occupancy and geometry updates |
| `Spectral.VoxelGI.Hierarchy` | Sparse octrees, clipmaps, bricked grids and multilevel lookup |
| `Spectral.VoxelGI.Materials` | Material summaries and physical-property sampling |
| `Spectral.VoxelGI.Spectrum` | Spectral basis encoding, decoding and error evaluation |
| `Spectral.VoxelGI.Radiance` | Directional radiance storage, emission and illumination injection |
| `Spectral.VoxelGI.Visibility` | Approximate occlusion, distance data and ray-traced fallback |
| `Spectral.VoxelGI.ConeTracing` | Cone generation, footprint filtering and front-to-back accumulation |
| `Spectral.VoxelGI.Bounce` | Iterative or incremental indirect-light propagation |
| `Spectral.VoxelGI.Dynamic` | Dirty-region tracking, movement, invalidation and temporal updates |
| `Spectral.VoxelGI.Streaming` | Large-world paging, memory budgets and cache residency |
| `Spectral.VoxelGI.Validation` | Leakage tests, energy checks, reference comparisons and cache statistics |
| `Spectral.VoxelGI.Debug` | Voxel views, cone visualizers, radiance displays and error heatmaps |

### Public API proposal

```
namespace Moonlight::Spectral::VoxelGI
{
    struct VoxelGISettings
    {
        float baseVoxelSizeMeters;
        std::uint32_t hierarchyLevels;
        std::uint32_t spectralBasisCount;
        std::uint32_t directionalLobeCount;
        bool enableDynamicUpdates;
        bool enableMultiBounce;
    };

    struct VoxelGIQuery
    {
        Vector3 worldPosition;
        Vector3 surfaceNormal;
        float wavelengthMeters;
        Vector3 outgoingDirection;
    };

    struct VoxelGIResult
    {
        SpectralRadiance radiance;
        float confidence;
        bool cacheValid;
    };

    class IVoxelRadianceCache
    {
    public:
        virtual ~IVoxelRadianceCache() = default;

        virtual void Update(
            const SceneSnapshot& scene,
            const VoxelGISettings& settings) = 0;

        virtual VoxelGIResult Evaluate(
            const VoxelGIQuery& query) const = 0;

        virtual void Invalidate(
            const WorldBounds& affectedRegion) = 0;
    };
}
```

This is a conceptual interface. Production code must define coordinate conventions, lifetime rules, synchronization, spectral data ownership, numerical tolerances, and memory layout. A high-throughput implementation should also use GPU-friendly data structures rather than relying on virtual calls for every voxel sample.

# 6. The right combination of global illumination technologies

For Spectral, the goal should not be to choose a single GI algorithm. Different approaches solve different problems.

Recent systems demonstrate a useful range of options: voxel cone tracing approximates illumination using voxel hierarchies, Dynamic Diffuse Global Illumination (DDGI) reuses probe-based irradiance and distance information, and spatially hashed or neural radiance caches reuse illumination during path tracing. NVIDIA RTXGI — radiance-cache approaches.

Voxel GI

Best role: fast, spatially broad indirect illumination and interactive scene feedback. Its main limitations are finite voxel resolution, filtering errors, and approximated visibility.

DDGI probes

Best role: efficiently reusing diffuse irradiance in dynamic environments. Probe visibility and placement need careful handling: thin walls, insufficient probe density and poorly configured offsets can cause leakage. Implementation guidance.

Spatial radiance cache

Best role: reusing indirect radiance encountered by path-traced rays. Cached values should include confidence and validity information; learned or approximate caches must be treated as estimators, not ground truth.

Hardware ray tracing

Best role: accurate scene intersections and visibility, sharp reflections, selected transmission paths and reference transport. It supplies the geometric information that voxel and probe caches often cannot preserve.

Spectral path tracing

Best role: the authoritative general-purpose rendering reference, particularly for converged offline rendering, measured materials and transport effects that caches do not represent adequately.

### How the systems should cooperate

For the high-quality real-time mode, use ray tracing for primary visibility and important sharp interactions, with voxel GI or probes providing approximate diffuse indirect illumination.

For offline final rendering, use a reference-capable spectral path tracer and allow caches to accelerate the computation only when the chosen algorithm has a known error model or an explicitly accepted bias.

For optical experiments, bypass caches that cannot preserve the required physical quantities. A voxel GI cache should not replace the coherent wave solver for an interference experiment, nor should a diffuse probe be expected to reproduce a sharp caustic accurately.

The renderer must also prevent double counting. Cached estimates and traced contributions must be combined through a defined estimator, not simply added together. Where a technique is biased, Spectral should report that trade-off.

# 7. Additional physical technologies required for maximum realism

Voxel GI solves an important part of the global-illumination problem. To make Spectral a general optical simulation platform, the wider engine must also cover the physical effects that voxel GI cannot faithfully reproduce.

## 7.1 Measured materials and physically meaningful surface models

The material system should be expanded beyond the usual base-colour, metallic, roughness and normal maps.

For each material, support the most appropriate available description:

| Material data | Physical purpose |
| --- | --- |
| Complex refractive index \(n(\lambda)+i\kappa(\lambda)\) | Wavelength-dependent reflection, transmission and absorption |
| Spectral reflectance and transmittance | Measured optical response over a declared wavelength range |
| BRDF and BTDF measurements | Direction-dependent reflected and transmitted light |
| BSDF or BSSRDF measurements | More general surface and subsurface transport |
| Surface height and slope statistics | Microgeometry and microsurface scattering |
| Layer thickness and layer composition | Coatings, paint, glass layers and multilayer optics |
| Anisotropic optical parameters | Materials whose optical behaviour varies with orientation |
| Absorption and scattering coefficients | Participating media and volume transport |
| Fluorescence data | Wavelength-changing light emission |
| Emissivity and temperature | Thermal emission where applicable |

The asset pipeline should permit measured BRDF, BSDF and spectral datasets alongside analytic material models. When using imported datasets, Spectral should preserve their conditions, units, valid range and uncertainty.

For interoperability, implement a compatible material-description layer such as OpenPBR, whose specification was updated in July 2026. However, OpenPBR is a shared surface-shading model, not a guarantee that every material is represented by complete laboratory measurements or all possible electromagnetic effects. Spectral should extend it with physically richer models where necessary.

Use RefractiveIndex.INFO as one possible source of optical constants, after checking each dataset's provenance and applicability.

## 7.2 Better microgeometry and surface reconstruction

A major limitation of ordinary material maps is that they cannot uniquely determine a real surface's microscopic structure.

Spectral should support three levels of geometric detail:

1. Explicit geometry: when the shape is resolved and its geometric effects matter.
2. Statistical microgeometry: when the surface is too small to resolve directly but its measured structure can be represented by a scattering model.
3. Measured directional response: when it is more practical to use a validated measured BSDF or richer appearance dataset.

The renderer must avoid double-counting features. For example, a surface bump should not be represented in explicit geometry and then again in a microfacet distribution that assumes the same unresolved structure.

Normal maps, height maps and roughness maps remain useful authoring tools, but their physical interpretation must be declared.

## 7.3 Layered materials, coatings and birefringence

Add dedicated support for:

- Thin-film interference and multilayer optical coatings.
- Dielectric and conductive materials.
- Anisotropic and birefringent optical media.
- Polarization-dependent reflection and transmission.
- Layered surface scattering.
- Wavelength-dependent subsurface transport.

Anisotropic materials require tensor-valued constitutive properties where scalar refractive index is insufficient. Birefringent materials, for example, can have different propagation properties along different material directions.

These models must plug into the physical material API, rather than adding special-case code directly to the path tracer.

## 7.4 Subsurface scattering

Skin, wax, marble, translucent plastics and many biological materials do not behave like opaque surfaces. Light enters the material, scatters internally, and exits at a different position.

Spectral should support a general bidirectional subsurface scattering reflectance distribution function (BSSRDF):

\[
S(\mathbf x_i,\omega_i;\mathbf x_o,\omega_o,\lambda)
\]

Unlike a BRDF, which relates incident and outgoing directions at a surface location, the BSSRDF also relates different entry and exit positions.

Implement multiple models:

- Diffusion approximations for suitable highly scattering materials.
- Random-walk transport for more general layered and heterogeneous media.
- Measured subsurface response where reliable data are available.
- Dedicated transport for biological or otherwise structurally complex materials, when required.

The choice of model should depend on the material and length scales. Diffusion theory is not a universal replacement for the underlying radiative transfer equation.

## 7.5 Fluorescence and phosphorescence

Ordinary elastic scattering changes the propagation direction without changing the photon's wavelength. Fluorescence can absorb light at one wavelength and emit it at another.

Introduce a wavelength-coupling model:

\[
K(\lambda_i,\lambda_o)
\]

where \(\lambda_i\) is the incoming wavelength and \(\lambda_o\) the outgoing wavelength. A fluorescent material's emission can be described using the absorbed excitation and a wavelength-changing response kernel, with explicit energy and quantum-yield conventions.

Spectral should validate the response for:

- Absorption wavelength range.
- Emission spectrum.
- Quantum yield.
- Energy conservation and wavelength conversion.
- Optional temporal emission behaviour.

Phosphorescence and other delayed emission effects also require a temporal response model. A conventional elastic RGB or spectral reflectance texture cannot reproduce these behaviours by itself.

## 7.6 Volume transport and atmosphere

Complete the volume system with physically based models for:

- Absorbing and scattering homogeneous media.
- Heterogeneous density fields and grids.
- Fog, smoke, dust and aerosols.
- Heterogeneous subsurface transport.
- Rayleigh scattering in suitable atmospheric regimes.
- Mie-based scattering where its particle assumptions are applicable.
- Molecular and particulate spectral absorption.
- Atmospheric and environmental emission where relevant.

The scalar radiative transfer equation for a stationary, non-polarizing medium can be expressed as:

\[
\begin{aligned}
\frac{dL_\lambda(\mathbf x,\omega)}{ds}
={}&-\sigma_t(\mathbf x,\lambda)L_\lambda(\mathbf x,\omega)\\
&+\sigma_s(\mathbf x,\lambda)
\int_{4\pi}
p(\omega',\omega)
L_\lambda(\mathbf x,\omega')\,d\omega'\\
&+j_\lambda(\mathbf x,\omega)
\end{aligned}
\]

where:

- \(\sigma_t=\sigma_a+\sigma_s\) is the extinction coefficient.
- \(\sigma_a\) is the absorption coefficient.
- \(\sigma_s\) is the scattering coefficient.
- \(p\) is the normalized angular scattering phase function.
- \(j_\lambda\) represents volume emission under the chosen units and formulation.

This equation is a radiometric transport model. Coherent fields, polarization-coupled media and inelastic wavelength-changing interactions require richer equations.

### Why this matters for voxel GI

A volume's density and extinction should not be treated like the opacity of an opaque triangle. They describe different physical phenomena.

Voxel GI should therefore support separate representations for:

- Surface coverage and approximate surface visibility.
- Volumetric extinction and transmission.
- Directional radiance.
- Emission and, where practical, scattering contributions.

A single opacity channel is insufficient for a general physical material representation.

# 8. Source lighting must be physical, too

Even the best transport algorithm cannot deliver measurement-grade results if its light sources are inaccurate.

Spectral should support a source definition based on measurable radiometric properties rather than requiring artists to work exclusively with arbitrary intensity values.

## 8.1 Light-source data

Implement:

- Spectral power distributions (SPDs) for light sources.
- Absolute radiant power, intensity and radiance where applicable.
- Measured light-emission profiles.
- Direction-dependent emission distributions.
- Temperature-dependent thermal sources.
- Environment maps with defined calibration and units.
- Optional time-dependent source power.
- Laser models with specified coherence and polarization characteristics.

The lighting system should distinguish radiant quantities from photometric quantities. Radiant power is an optical energy quantity; luminous flux weights that energy according to a human-vision function.

For measured or calibrated emitters, spectral distribution matters: two sources with the same apparent RGB colour can have different spectra and produce different appearances on real materials.

## 8.2 Importance sampling and difficult light paths

For efficient path tracing, add dedicated light-sampling systems, including:

- Hierarchical light selection.
- Multiple importance sampling.
- Specialized sampling for area lights and emissive geometry.
- Efficient environment-light sampling.
- Caustic-focused transport methods.
- Spatiotemporal reservoir resampling for appropriate real-time lighting tasks.

ReSTIR is a useful technique for efficiently resampling candidate light samples. Its original research demonstrated substantial acceleration for dynamic direct lighting, with an explicit accuracy/performance trade-off depending on the estimator. Source: ReSTIR research.

ReSTIR should be implemented as a sampling system, not confused with a new material model or a complete solution for all global illumination.

# 9. Camera, sensor and image formation

A renderer that simulates optical behaviour but uses an arbitrary image-generation pipeline cannot automatically produce a measurement-correct photograph.

Spectral should separate scene radiance, physical camera optics, the sensor response and final display output.

## 9.1 Camera optics

The camera module should support:

- Aperture shape and size.
- Depth of field.
- Lens transmission and coatings.
- Lens distortion.
- Vignetting.
- Multiple lens elements where required.
- Focus distance and focus breathing where applicable.
- Chromatic aberration derived from wavelength-dependent optical behaviour.
- Exposure time and motion blur.
- Optional shutter and rolling-shutter models.

An idealized pinhole camera remains useful for a baseline renderer, but it should not be treated as a complete camera model.

## 9.2 Sensor response

For a simplified sensor channel \(c\), expected photoelectrons can be approximated by:

\[
N_{e,c}\propto
t_{\mathrm{exp}}A\Omega
\int
L_\lambda(\lambda)
\tau(\lambda)
QE_c(\lambda)
\frac{\lambda}{hc}\,d\lambda
\]

Here \(t_{\mathrm{exp}}\) is exposure time, \(A\) is the relevant collecting area, \(\Omega\) is the pixel's accepted solid angle under the adopted geometry, \(\tau\) is optical transmission, and \(QE_c\) is the channel's quantum efficiency.

This is a simplified imaging relation; a complete model must handle its optical geometry, sensor architecture and calibration carefully.

The sensor simulation should additionally support:

- Quantum efficiency across wavelength.
- Pixel sampling and integration.
- Photon shot noise.
- Read noise and dark current where relevant.
- Saturation and sensor dynamic range.
- Colour filters or multispectral sensor responses.
- Optional demosaicing and raw-image processing.

## 9.3 Colour science

For perceptual output, Spectral should integrate the spectral radiance against standard observer colour-matching functions before transforming into the chosen output colour space.

The relevant standards should be implemented and documented with exact versions, including the CIE standard colorimetric observers.

Keep these stages distinct:

1. Physical spectral transport.
2. Camera and sensor response.
3. Exposure and image formation.
4. Colourimetric conversion.
5. Display encoding and tone mapping.

Tone mapping and denoising must be optional post-processing operations. Neither should silently change the physical simulation state.

# 10. Additional systems needed for a complete simulation workflow

Beyond voxel GI, Spectral should gain several systems that turn it from a rendering algorithm into a reusable scientific and production platform.

## 10.1 Transient light transport

Ordinary rendering normally integrates light over time and produces a steady-state image. Some real-world measurements, such as time-of-flight imaging, lidar and ultrafast optical experiments, depend on when light arrives.

Add `Spectral.Transient` to track optical path length, group delay, temporal emission profiles and sensor timing.

For a path through dispersive media, the propagation delay is approximated by:

\[
t_{\mathrm{flight}}=
\int_\Gamma \frac{ds}{v_g(s,\lambda)}
=
\frac{1}{c}\int_\Gamma n_g(s,\lambda)\,ds
\]

where \(n_g\) is the group refractive index under the applicable propagation assumptions.

This enables transient radiance output and time-resolved transport. For applications requiring coherent pulses, the subsystem must additionally preserve the field and pulse information needed by the experiment.

## 10.2 Inverse rendering and material calibration

Create `Spectral.Calibration` to estimate physical scene parameters from measurements.

Potential inputs include:

- Photographs of calibrated test scenes.
- Spectral reflectance and transmission measurements.
- Camera calibration data.
- Interferometry or profilometry measurements.
- Measured light-source spectra.
- Angular reflectance and transmission measurements.

The system can compare simulated measurements with observations and optimize parameters within the selected model.

However, inverse rendering is not guaranteed to find a unique material description. Different combinations of illumination, geometry and material properties can produce similar images. Spectral must report ambiguity and uncertainty instead of claiming to have recovered a unique physical material from a photograph.

## 10.3 Physical scene units and calibration

All spatial, temporal, radiometric and material quantities must have explicit units.

The engine should enforce units for:

- Geometry and layer thickness.
- Wavelength and frequency.
- Time and propagation delay.
- Spectral radiance and radiant power.
- Absorption and scattering coefficients.
- Temperature and thermal properties.
- Surface and volume measurements.

In the editor, all values should be converted to internal units at asset boundaries. Unit validation should be integrated into the compiler, asset importer and material inspector.

## 10.4 Numerical solvers and convergence management

The optical solver subsystem needs dedicated tools for numerical accuracy:

- Adaptive mesh generation.
- Error estimation.
- Solver residual monitoring.
- Stability checks.
- Double-precision reference calculations.
- Boundary-condition validation.
- Mesh and timestep refinement.
- Numerical dispersion analysis for wave solvers.
- Memory and runtime estimates before expensive simulations.

These tools are especially important when coupling rendering with full-field electromagnetic solvers.

PBRT v4 is a useful reference for the architectural separation of spectral rendering, material and volume models, and numerical light-transport algorithms. Its documentation describes spectral path tracing, participating media and multiple integrator choices, but Spectral should implement its own modular contracts around the required physical models. PBRT v4 User Guide.

# 11. Revised rendering profiles

Spectral should expose clearly defined profiles. Each profile selects a known combination of physical models, acceleration techniques and accuracy requirements.

## Profile A — Interactive Preview

Performance first

Use for: scene editing, material authoring and rapid iteration.

Uses hardware ray tracing where appropriate, voxel GI, radiance caches and optional denoising. Prioritizes responsive feedback.

Limitations: cached illumination, finite voxel resolution, incomplete convergence, and unsupported coherent effects must be clearly reported.

## Profile B — Hybrid Production

Balanced

Use for: interactive production scenes and game-engine output.

Uses spectral transport where supported, explicit ray-traced visibility and reflections, voxel or probe-based diffuse GI, and radiance caching for selected indirect paths.

Limitations: the selected caches and approximations impose a declared fidelity ceiling.

## Profile C — Reference Path Tracing

Accuracy-oriented

Use for: final offline rendering, material evaluation and reference images.

Uses converged spectral path tracing or a more appropriate transport integrator, measured material data, explicit physical lighting and validation against reference scenes.

Limitations: spectral path tracing alone does not preserve general wave interference or solve arbitrary electromagnetic field problems.

## Profile D — Coherent Optical Simulation

Wave-aware

Use for: interference, diffraction, phase-sensitive imaging and specialized optical experiments.

Uses coherent field representations, polarization-aware transport and validated wave-propagation or diffraction methods.

Limitations: each solver is valid only for its stated approximations and boundary conditions.

## Profile E — Electromagnetic Lab

Numerical field solving

Use for: detailed electromagnetic validation, wavelength-scale structures and advanced material characterization.

Invokes selected FDTD, finite-element or frequency-domain solvers with field outputs and convergence reporting.

Limitations: computational cost can be very high; the valid simulation domain and material constitutive models must be specified.

The profiles are presets rather than mutually exclusive products. A project may use Profile A while editing, Profile B during development, Profile C for final images and Profile D or E for selected optical components.

A high-fidelity mode should not silently fall back to a faster approximation when a requested physical model is unavailable. It should either reject the configuration or explicitly ask the caller to choose an allowed alternative.

# 12. Data formats and the rendering API

The material system and the GI system should use versioned assets so that changes to the algorithms do not invalidate every project.

A proposed scene configuration could be:

```
{
  "spectralVersion": 2,
  "renderProfile": "reference_path_tracing",
  "spectrum": {
    "representation": "sampled",
    "wavelengthRangeNm": [380, 780],
    "adaptiveSampling": true
  },
  "transport": {
    "integrator": "spectral_path",
    "maximumPathDepth": 16,
    "multipleImportanceSampling": true
  },
  "globalIllumination": {
    "mode": "voxel_cache_assisted",
    "voxelCache": {
      "enabled": true,
      "representation": "sparse_clipmap",
      "spectralEncoding": "basis",
      "directionalRepresentation": "spherical_harmonics",
      "cacheConfidenceRequired": true
    },
    "fallbackToExplicitTransport": true
  },
  "physicalModels": {
    "dispersion": true,
    "polarization": true,
    "volumeScattering": true,
    "coherentTransport": false,
    "thinFilmInterference": true,
    "transientTransport": false
  },
  "validation": {
    "validateMaterialRanges": true,
    "reportCacheBias": true,
    "reportConvergence": true,
    "preservePhysicalDiagnostics": true
  }
}
```

These settings are illustrative, not production defaults. The system must reject combinations that cannot be supported by the selected integrator.

For example, enabling coherent transport should not cause Spectral to interpret a standard directional radiance voxel as a complex field. An incompatible configuration needs a clear diagnostic.

### Required output metadata

Every render should be able to record:

- Spectral version and renderer build identifier.
- Integrator and physical models used.
- Material asset revisions and measurement provenance.
- Wavelength sampling strategy.
- Lighting and camera calibration data.
- Voxel GI settings and cache usage.
- Whether biased estimators were used.
- Sampling and solver convergence information.
- Unit conventions and output colour space.
- Denoising, tone mapping and post-processing settings.

This makes renders reproducible and more suitable for production review or scientific comparison.

# 13. Quality assurance and validation

The goal is not simply to produce an image that looks realistic. It is to establish how closely the renderer agrees with a known physical result and to understand the source of any difference.

## 13.1 Voxel GI test suite

Add dedicated tests for:

| Test | What it verifies |
| --- | --- |
| Empty voxel volume | No spurious indirect energy |
| Uniform diffuse enclosure | Expected convergence of diffuse illumination |
| Thin-wall scene | Detects voxelization and light-leak problems |
| Small emissive object | Tests spatial resolution and light injection |
| Moving object | Tests voxel invalidation and update correctness |
| Varying voxel resolution | Measures discretization and filtering error |
| Spectral basis reconstruction | Measures error in cached spectra |
| Directional illumination | Tests angular representation and filtering |
| Dynamic light changes | Tests cache aging and convergence |
| Hybrid cache/path-tracing comparison | Quantifies the effect of acceleration |

## 13.2 Physical reference suite

Continue the broader reference tests from the original Spectral design and add:

- Measured spectral reflectance and transmission cases.
- Layered coating measurements.
- Polarization-dependent interface tests.
- Subsurface scattering reference cases.
- Atmospheric extinction and scattering checks.
- Fluorescence energy and wavelength-conversion checks.
- Real camera and sensor calibration scenes.
- Transient time-of-flight cases.
- Electromagnetic solver mesh-convergence studies.

For every test, record the expected numerical result, source data, tolerances, applicable assumptions and test conditions.

## 13.3 Error budgets

Spectral should distinguish at least five error sources:

1. Input error: uncertain geometry, lighting and material measurements.
2. Model error: limitations of the selected physical model.
3. Discretization error: voxel size, mesh spacing, spectral basis and sampling resolution.
4. Estimator error: Monte Carlo noise and stochastic approximation.
5. Output error: sensor modelling, image reconstruction, denoising and display transformation.

This distinction is especially important when comparing voxel GI with path tracing. A sharper or less noisy image is not necessarily more physically accurate.

# 14. Development roadmap for the extended engine

The recommended build order is to establish a trustworthy baseline first, then use the voxel system to improve performance without allowing it to dictate the entire architecture.

1. Foundation — physical material and transport API

    Finalize unit types, spectral representations, optical material interfaces, reference tests and the initial spectral path tracer.

2. Voxel GI foundation

    Implement conservative voxelization, a sparse hierarchy, directional radiance storage, cone tracing and debugging visualizers.

3. Hybrid indirect illumination

    Add direct-light injection, multibounce updates, dynamic invalidation and integration with ray-traced visibility. Quantify leakage and energy errors.

4. Advanced GI acceleration

    Add probe-based GI and radiance caching, then evaluate spatiotemporal resampling for appropriate lighting workloads. Keep accuracy assumptions separate for each method.

5. Advanced material physics

    Add measured BSDFs, dispersion, layered optics, subsurface transport, polarization and more detailed volume models.

6. Wave-optics capability

    Implement coherent propagation, validated diffraction models, thin-film interference and the interfaces needed for specialized solvers.

7. Numerical electromagnetic simulation

    Integrate a selected field solver with controlled boundary conditions, convergence tools and explicit coupling contracts.

8. Measurement-grade output and enterprise validation

    Finalize calibrated cameras, physical lighting, provenance, reproducible render metadata, reference datasets and automated cross-backend validation.


Do not begin by trying to simulate every possible physical phenomenon. First ensure that the core abstractions permit those phenomena to be added correctly.

# 15. The boundary between the ambition and physical reality

There is an important limitation to the goal of making Spectral perfect.

A universal, exact simulator of any real-world object cannot be obtained merely by importing a 3D model, adding a texture and supplying a few material values. A photograph does not uniquely reveal the object's material structure, microscopic geometry, spectral optical constants, internal composition or the illumination under which it was photographed.

Even when all inputs are known, the appropriate solution method depends on the physical scale and the behaviour being studied. Geometric ray transport is suitable for many macroscopic optical effects. Coherent wave methods are needed for interference and diffraction. Detailed wavelength-scale electromagnetic behaviour may require a numerical field solver.

These are different levels of the same physical problem, not features that can be made exact by increasing a graphics quality slider.

Spectral should therefore aim for the highest verifiable accuracy for a specified scene and physical regime, not claim universal exactness. Its strongest differentiator will be that each render can explain which physics it used, which measurements informed it, what it approximated, and how the result was validated.

## Final design decision

Keep the existing Spectral architecture and add `Spectral.VoxelGI` as a first-class but explicitly approximate indirect-illumination subsystem. Pair it with a spectral path tracer, hardware ray tracing, probe-based or radiance-cache acceleration where appropriate, and the specialized wave-optical and electromagnetic solvers needed for more demanding simulations.