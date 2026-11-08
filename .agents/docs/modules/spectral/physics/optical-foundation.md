# 1. Product definition and physical limits

## 1.1 The objective

Spectral should be a spectral, polarization-aware, wave-capable rendering engine. Its purpose is to calculate how light interacts with geometry and materials using physical parameters, rather than relying primarily on artistic approximations.

The intended workflow in Workbench is:

1. Import a 3D model with its geometry, UV coordinates and measured or artist-provided texture maps.
2. Assign one or more physical material definitions.
3. Specify the illumination, camera, environment and relevant physical conditions.
4. Choose the required simulation fidelity.
5. Render the scene using Spectral, with its results determined by the selected physical models and their input data.

A physically based renderer can make its results consistent with optical laws, but that alone does not guarantee agreement with an actual photograph. Accurate output requires accurate inputs, appropriate physics, and a numerically converged solution.

A texture is not a complete physical material. A photograph of a painted wall does not tell the renderer the wall's exact pigment absorption spectrum, microscopic roughness distribution, layer thickness, subsurface structure or fluorescence characteristics. Spectral must therefore distinguish between a material's visual appearance and its measured optical properties.

## 1.2 The three simulation regimes

Spectral should support three related regimes that share mathematics, material data and scene infrastructure but use different transport algorithms.

### Regime A — Spectral geometric optics

Production foundation

Reflection, refraction, Fresnel effects, wavelength-dependent dispersion, absorption, volumetric scattering, indirect illumination, caustics and physically based surface reflectance. Phase-sensitive wave effects are approximated or omitted except where represented by specific specialized models.

### Regime B — Coherent wave transport

Advanced simulation

Retains complex field amplitudes and phase relationships to model interference and wave-optical diffraction. It needs wave-aware transport algorithms rather than simply adding more ordinary ray bounces.

### Regime C — Full-field electromagnetic solving

Specialized solver

Numerically solves Maxwell's equations in selected regions for wavelength-scale structures, complex material boundaries and demanding diffraction or nanophotonic problems. This is much more computationally expensive than ordinary scene path tracing.

This separation is a core architectural requirement, not an implementation detail. Geometric light transport is highly effective for many visual effects, but interference depends on relationships between fields, and wavelength-scale structures can require full-field treatment. Wave-tracing research published in 2026 specifically addresses ways to bridge wave optics and path-integral rendering.

## 1.3 Physical fidelity versus computational cost

| Feature | Spectral path tracing | Coherent wave transport | Full-field solver |
| --- | --- | --- | --- |
| Wavelength-dependent colour | Native | Native | Native |
| Fresnel reflection and refraction | Native | Native | Emerges from field solution |
| Dispersion | Native with valid optical data | Native | Emerges from constitutive model |
| Absorption | Native | Native | Emerges from complex material response |
| Volume scattering | Monte Carlo transport | Wave-aware model needed | Requires appropriate scatterer or material modelling |
| Polarization | Optional Stokes/Mueller or Jones transport | Field-aware polarization | Vector electromagnetic fields |
| Interference | Not generally preserved by standard path tracing | Supported | Supported |
| Diffraction | Specialized approximations possible | Supported by suitable solver | Supported |
| Arbitrary wavelength-scale microstructures | Insufficient without specialized models | Method-dependent | Suitable in the applicable numerical regime |
| Cost for complex scenes | Relatively practical | High and method-dependent | Potentially extremely high |

This is a capability map, not a guarantee that every technique can solve every scene. Even a full-field solver needs a suitable computational domain, constitutive model, boundary conditions and numerical resolution.

# 2. Spectral architecture and module design

The renderer should be structured around physical quantities, independently replaceable models, and transport algorithms. Each physical effect must have a stable interface and a defined mathematical contract. The rendering integrator decides when to invoke a model; the model determines the physical interaction.

The architecture should not contain a monolithic `RayTracer` class that owns every optical equation.

## 2.1 Module architecture

The implementation must keep the physical models and transport methods
independent. A voxel GI implementation can be replaced without rewriting the
material system, and an electromagnetic solver can consume the same validated
material assets without inheriting the geometric path tracer's assumptions.

## 2.2 Responsibility of each module

| Module | Primary responsibility |
| --- | --- |
| `Spectral.Core` | Shared physical data types, configuration, lifecycle and error reporting |
| `Spectral.Math` | Numerically tested vector, complex, matrix, spectral and integration operations |
| `Spectral.Data` | Material databases, measurement ingestion, units, metadata and interpolation |
| `Spectral.Materials` | Composable physical materials, surface layers and material evaluation |
| `Spectral.Interfaces` | Boundary interactions, Fresnel coefficients, reflection and refraction |
| `Spectral.Geometry` | Intersections, normals, UVs, geometric acceleration and boundary classification |
| `Spectral.Volumes` | Absorption, scattering, emission, transmittance and participating media |
| `Spectral.Polarization` | Polarization state, coordinate-frame changes and polarization-dependent interactions |
| `Spectral.Lighting` | Emitters, spectral power distributions, environment illumination and light sampling |
| `Spectral.Integrators` | Monte Carlo path tracing, direct lighting, multiple scattering and convergence |
| `Spectral.Coherent` | Complex field amplitudes, phase accumulation, coherent transport and coherence |
| `Spectral.WaveOptics` | Diffraction, interference, propagation and specialized wave-optical models |
| `Spectral.Solvers` | Full-field electromagnetic solvers and coupling to the renderer |
| `Spectral.Camera` | Camera optics, exposure, aperture, sensor response and optional polarization sensitivity |
| `Spectral.ColourScience` | CIE tristimulus integration, colour transforms and display output |
| `Spectral.Backends` | Hardware abstraction and CPU/GPU execution implementations |
| `Spectral.Validation` | Analytic reference cases, regression testing and numerical error measurement |

Dependencies should point toward the shared physical foundation. Material models must not depend directly on Workbench, and a GPU backend must not redefine the meaning of a physical equation.

# 3. Physical foundation: representing electromagnetic light

Before building the individual effects, Spectral needs a shared representation of light that can support both ordinary ray transport and wave simulation.

The most important rule is:

A geometric ray represents a direction of propagation; an electromagnetic wave also has a frequency, phase, amplitude and polarization. A standard path tracer typically transports radiometric quantities such as spectral radiance. A coherent wave solver must retain additional information about the electromagnetic field.

## 3.1 Fundamental quantities

Spectral should use SI units internally wherever practical.

| Quantity | Symbol | SI unit | Meaning |
| --- | --- | --- | --- |
| Vacuum wavelength | $(\lambda_0)$ | m | Spatial period in vacuum |
| Frequency | \(f\) or \(\nu\) | Hz | Oscillations per second |
| Angular frequency | \(\omega\) | rad/s | \(2\pi f\) |
| Vacuum wavenumber | \(k_0\) | rad/m | \(2\pi/\lambda_0\) |
| Phase | \(\phi\) | rad | Phase of the oscillation |
| Electric field | \(\mathbf E\) | V/m | Electric-field vector |
| Magnetic field intensity | \(\mathbf H\) | A/m | Magnetic-field intensity |
| Phase refractive index | \(n\) | dimensionless | Determines phase velocity |
| Extinction coefficient | \(\kappa\) | dimensionless | Determines attenuation in an absorbing medium |
| Phase velocity | \(v_p\) | m/s | Velocity of a constant-phase surface |
| Group velocity | \(v_g\) | m/s | Velocity of a wave packet in a dispersive medium |
| Spectral radiance | \(L_\lambda\) | W m\(^{-2}\) sr\(^{-1}\) nm\(^{-1}\), if using per-nm units | Directional optical energy transport |
| Absorption coefficient | $(\alpha)$ | m\(^{-1}\) | Exponential attenuation of intensity |
| Scattering coefficient | $(\sigma_s)$ | m\(^{-1}\) | Probability per unit distance of scattering |

Internally, wavelength units should never be implicit. Use metres as the canonical SI representation, with an explicit conversion to nanometres for visible-spectrum interfaces and data import.

## 3.2 Maxwell's equations: the ultimate physical foundation

Maxwell's equations describe the electromagnetic field. In SI units:

$\nabla\cdot\mathbf D=\rho$

$\nabla\cdot\mathbf B=0$

$\nabla\times\mathbf E=-\frac{\partial\mathbf B}{\partial t}$
$\nabla\times\mathbf H=
\mathbf J+\frac{\partial\mathbf D}{\partial t}$

Here $(\mathbf D)$ is electric displacement, $(\mathbf B)$ is magnetic flux density, $(\rho)$ is charge density, and $(\mathbf J)$ is electric current density.

To determine how fields behave in matter, these equations need constitutive relationships. For a simple, linear, homogeneous material:
$\mathbf D=\epsilon\mathbf E,\qquad
\mathbf B=\mu\mathbf H$

where $(\epsilon)$ and $(\mu)$ describe the material's permittivity and permeability.

For a source-free, homogeneous, isotropic, non-magnetic medium, the electric field satisfies the vector wave equation:
$\nabla^2\mathbf E-\frac{n^2}{c^2}
\frac{\partial^2\mathbf E}{\partial t^2}=0$

This is the simplified lossless case. In absorbing, dispersive, anisotropic or spatially varying media, the equations require additional terms or a more general constitutive model.

### How this affects the architecture

Spectral should not force every pixel and ray to solve Maxwell's equations. Instead, Maxwell's equations define the physical reference that simpler models must approximate.

The practical rendering engine uses geometric and probabilistic formulations where they are appropriate; the wave solver handles cases where field interference or detailed boundary behaviour is essential.

This is consistent with the approach taken by modern physically based rendering and research into hybrid wave-optical transport.

# 4. Mathematical model library

This is the core of the design. Each physical model should be implemented as a separate, versioned module, with common numerical types and clearly defined inputs and outputs.

## 4.1 Wave propagation, wavelength and frequency

For a monochromatic plane wave travelling in the \(z\)-direction, a useful complex representation is:

\[
\mathbf E(z,t)=
\operatorname{Re}
\left\{
\mathbf E_0 e^{i(kz-\omega t)}
\right\}
\]

Here \(\mathbf E_0\) is a complex amplitude describing amplitude and phase.

In vacuum:

\[
c=f\lambda_0,\qquad
\omega=2\pi f,\qquad
k_0=\frac{2\pi}{\lambda_0}
\]

In a transparent, dispersive medium:

\[
v_p=\frac{c}{n(\lambda_0)}
\]

\[
\lambda_m=\frac{\lambda_0}{n(\lambda_0)}
\]

\[
k(\omega)=\frac{n(\omega)\omega}{c}
\]

The frequency remains unchanged when light crosses a stationary material boundary, while wavelength and phase velocity change.

### Phase velocity and group velocity

For a dispersive material, different frequencies propagate with different phase velocities. The group velocity for a narrow-band wave packet is:

\[
v_g=\frac{d\omega}{dk}
=\frac{c}{n+\omega\frac{dn}{d\omega}}
\]

The group index can be expressed using vacuum wavelength:

\[
n_g=n-\lambda_0\frac{dn}{d\lambda_0}
\]

Therefore,

\[
v_g=\frac{c}{n_g}
\]

This expression assumes a differentiable dispersion relation and a regime in which group velocity describes the packet propagation meaningfully. In strongly absorbing or anomalously dispersive regions, group velocity must be interpreted with care.

### Modular implementation

Create `Spectral.Propagation` with separate models:

- `PhaseVelocityModel`
- `GroupVelocityModel`
- `DispersionModel`
- `PhaseAccumulationModel`
- `AttenuationModel`

Do not hard-code refractive index as a single material constant. Evaluate it at the relevant wavelength and, where required, temperature, pressure, material direction and other conditions.

## 4.2 Complex refractive index and absorption

For a homogeneous, isotropic material, represent its complex refractive index as:

\[
\tilde n(\lambda)=n(\lambda)+i\kappa(\lambda)
\]

Under the convention used here, the field varies as \(e^{i(kz-\omega t)}\), so the positive extinction coefficient \(\kappa\) causes attenuation.

Since

\[
\tilde k=k_0\tilde n
\]

the field amplitude decays as

\[
|\mathbf E(z)|=|\mathbf E(0)|e^{-k_0\kappa z}
\]

and intensity follows:

\[
I(z)=I(0)e^{-\alpha z}
\]

where

\[
\boxed{\alpha(\lambda)=\frac{4\pi\kappa(\lambda)}{\lambda_0}}
\]

This is the connection between the extinction coefficient and the absorption coefficient.

The intensity transmitted through a uniform absorbing slab of thickness \(d\), excluding interface effects, is:

\[
T_{\mathrm{bulk}}(\lambda)=e^{-\alpha(\lambda)d}
\]

If the slab has significant interface reflections, its total transmission must also account for Fresnel effects and potentially multiple internal reflections.

### Dielectric response

For non-magnetic media, the complex relative permittivity is approximately related to the complex refractive index by:

\[
\tilde\epsilon_r(\omega)=\tilde n^2(\omega)
\]

Therefore, a material database that supplies complex refractive index can provide a useful foundation for reflection and absorption models.

For anisotropic materials, this scalar relationship must be replaced by a tensor treatment.

## 4.3 Dispersion models

Spectral needs a common `IDispersionModel` interface, with several implementations.

### Cauchy dispersion

A commonly used approximation for transparent materials over a limited spectral interval is:

\[
n(\lambda_0)=A+\frac{B}{\lambda_0^2}
+\frac{C}{\lambda_0^4}
\]

The coefficients depend on the material and the units used for wavelength. This is an approximation and should only be used within a validated wavelength range.

### Sellmeier dispersion

A widely used model for transparent optical materials is:

\[
n^2(\lambda_0)=1+
\sum_{j=1}^{N}
\frac{B_j\lambda_0^2}
{\lambda_0^2-C_j}
\]

This supports wavelength-dependent refraction and chromatic dispersion.

### Measured spectral data

For enterprise production, the engine should also accept measured tables:

\[
\{(\lambda_i,n_i,\kappa_i)\}_{i=1}^{N}
\]

with a declared interpolation method and valid wavelength range.

Use measured data whenever possible, rather than assuming a convenient analytic approximation accurately describes the actual material.

An established starting point for optical constants is the RefractiveIndex.INFO database. It contains literature-derived and manufacturer-supplied data, including refractive-index and extinction-coefficient datasets, and is distributed under CC0. Individual datasets still require checking for their measurement conditions, applicable range and accuracy. 【turn466372view1】

## 4.4 Reflection and refraction at material interfaces

This module calculates what happens when a ray or wave reaches a boundary between two materials.

### Snell's law

For transparent, isotropic media:

\[
n_1\sin\theta_i=n_2\sin\theta_t
\]

where \(\theta_i\) and \(\theta_t\) are the incident and transmitted angles measured from the surface normal.

For \(n_1>n_2\), the critical angle is:

\[
\theta_c=\arcsin\left(\frac{n_2}{n_1}\right)
\]

When \(\theta_i>\theta_c\), total internal reflection occurs in the ideal lossless case.

Complex or anisotropic media require a more general wavevector-based treatment. Do not use the elementary form of Snell's law indiscriminately for all materials.

### Ray reflection

Let \(\mathbf d\) be a normalized incident direction pointing toward the surface, and let \(\mathbf n\) be the unit normal pointing against the incident ray. Then:

\[
\mathbf d_r=\mathbf d-
2(\mathbf d\cdot\mathbf n)\mathbf n
\]

### Ray refraction

For the same direction convention, set:

\[
\eta=\frac{n_1}{n_2},\qquad
\cos\theta_i=-\mathbf n\cdot\mathbf d
\]

\[
\cos\theta_t=\sqrt{1-\eta^2(1-\cos^2\theta_i)}
\]

When the square-root argument is non-negative:

\[
\mathbf d_t=
\eta\mathbf d+
(\eta\cos\theta_i-\cos\theta_t)\mathbf n
\]

The implementation must explicitly handle total internal reflection and invalid numerical inputs.

### Fresnel equations

The Fresnel equations determine the reflected and transmitted electric-field amplitudes for an electromagnetic wave incident on an interface.

For non-magnetic, lossless media:

\[
r_s=
\frac{n_1\cos\theta_i-n_2\cos\theta_t}
{n_1\cos\theta_i+n_2\cos\theta_t}
\]

\[
r_p=
\frac{n_2\cos\theta_i-n_1\cos\theta_t}
{n_2\cos\theta_i+n_1\cos\theta_t}
\]

The subscripts \(s\) and \(p\) represent polarization perpendicular to and parallel to the plane of incidence, respectively. The sign of an amplitude coefficient depends on the adopted polarization basis convention, which must be defined consistently throughout Spectral.

Power reflectance is obtained from field amplitudes:

\[
R_s=|r_s|^2,\qquad R_p=|r_p|^2
\]

For unpolarized incident light:

\[
R=\frac{R_s+R_p}{2}
\]

For lossless media, the corresponding power transmittance must include the ratio of normal transmitted to incident energy flux. It is not generally correct to equate transmitted power with the square of the transmitted field amplitude alone.

For absorbing materials, Fresnel coefficients become complex and the transmitted wavevector may also be complex.

### Module structure

`Spectral.Interfaces` should contain independently testable implementations of:

- `SnellRefractionModel`
- `FresnelDielectricModel`
- `FresnelConductorModel`
- `TotalInternalReflectionModel`
- `ThinFilmInterfaceModel`
- `AnisotropicInterfaceModel`, as an advanced extension

The basic dielectric and conductor models should be implemented before the more specialized cases.

# 5. Polarization: modelling orientation of the electric field

Polarization should be optional in the execution configuration, but supported at the data-model level from the start. Otherwise, adding it later can force a redesign of the spectral transport representation.

Two representations are especially useful.

## 5.1 Jones representation

For a fully polarized monochromatic wave, use a complex Jones vector:

\[
\mathbf J=
\begin{bmatrix}
E_x\\
E_y
\end{bmatrix}
\]

An optical element can transform that field using a complex Jones matrix:

\[
\mathbf J_{\mathrm{out}}=
\mathbf J_{\mathrm{operator}}\mathbf J_{\mathrm{in}}
\]

This representation naturally retains phase and polarization relationships for coherent fields. It is not sufficient by itself for arbitrary partially polarized light.

## 5.2 Stokes representation

For radiometric transport involving fully or partially polarized light, use the Stokes vector:

\[
\mathbf S=
\begin{bmatrix}
I\\Q\\U\\V
\end{bmatrix}
\]

It describes total intensity and the polarization state. The Stokes convention, including the sign of \(V\), must be globally documented because conventions differ.

A Mueller matrix describes the effect of an optical element on the Stokes state:

\[
\mathbf S_{\mathrm{out}}=
\mathbf M\mathbf S_{\mathrm{in}}
\]

Reflection, refraction, scattering and polarization-dependent sensors can therefore use the same representation.

The Mitsuba 3 renderer provides a useful reference for spectral and polarized rendering architecture, including Stokes/Mueller transport, basis rotations and the challenges of maintaining consistent polarization conventions. 【turn954986search1】【turn954986search12】

### Proposed module

`Spectral.Polarization` should contain:

- Jones vectors and Jones matrices.
- Stokes vectors and Mueller matrices.
- Polarization coordinate frames and rotations.
- Polarized Fresnel interface models.
- Polarization-aware surface and volume scattering.
- Polarized camera and sensor response.

Important: Ordinary ray tracing can carry polarization states, but that does not by itself enable general interference. Retaining field phase and retaining polarization are related but different capabilities.

# 6. Surface materials: reflection, roughness and scattering

A practical rendering engine cannot geometrically model every microscopic feature in every surface. It therefore needs statistical models of unresolved surface structure.

This is where Spectral's physical material layer connects microscopic properties to practical rendering.

## 6.1 The BRDF and BSDF

The bidirectional reflectance distribution function (BRDF) describes how a surface reflects incident radiance into outgoing directions.

For a spectral BRDF:

\[
f_r(\lambda,\omega_i,\omega_o)
\]

the outgoing radiance is determined by integrating incident illumination over incoming directions.

A Lambertian diffuse surface is a simple special case:

\[
f_r(\lambda)=\frac{\rho(\lambda)}{\pi}
\]

where \(\rho(\lambda)\) is spectral diffuse reflectance.

For general reflection and transmission, use a BSDF, which includes scattering across the relevant outgoing directions and can represent both reflection and transmission.

This immediately establishes one of Spectral's most important data rules: spectral reflectance is a function of wavelength, not inherently an RGB colour.

## 6.2 Microfacet reflection

For common rough reflective surfaces, a microfacet model is a useful approximation. It treats a surface as a distribution of small facets whose orientations are statistically described.

A common reflective microfacet BRDF has the form:

\[
f_r=
\frac{D(\mathbf h)\,F(\omega_i,\mathbf h)\,
G(\omega_i,\omega_o,\mathbf h)}
{4|\mathbf n\cdot\omega_i|
|\mathbf n\cdot\omega_o|}
\]

where:

- \(D\) is the microfacet normal distribution.
- \(F\) is the Fresnel reflection factor.
- \(G\) represents masking and shadowing between facets.
- \(\mathbf h\) is the appropriately normalized half-vector.

The distribution and masking model determine the appearance of roughness. They should be separate from the underlying optical constants of the material.

Possible implementations include GGX/Trowbridge–Reitz and Beckmann distributions. Spectral should include at least one validated microfacet model before adding more sophisticated alternatives.

## 6.3 Roughness is not a universal physical parameter

A slider labelled `roughness = 0.4` does not represent a universal measurable property of a real material.

It is meaningful only in conjunction with the particular statistical scattering model used by the renderer. Physical measurements of a real surface can include:

- Surface height distribution.
- Surface slope distribution.
- Spatial correlation length.
- Anisotropy and directionality.
- Wavelength-scale surface structures.
- Larger-scale shape and geometry.

For optical accuracy, measured surface statistics or appropriate microgeometry should replace arbitrary artistic roughness wherever possible.

A normal map alone does not contain all this information.

# 7. Absorption, scattering and transmission through volumes

Surface reflection is only part of the problem. Light passing through smoke, fog, glass, liquids, skin or translucent solids may be absorbed or scattered internally.

## 7.1 Beer–Lambert absorption

For a uniform medium with absorption coefficient \(\sigma_a\):

\[
T(d,\lambda)=e^{-\sigma_a(\lambda)d}
\]

For a spatially varying medium:

\[
T(d,\lambda)=
\exp\left(
-\int_0^d \sigma_a(s,\lambda)\,ds
\right)
\]

This describes loss of the unscattered beam through absorption alone.

## 7.2 Scattering and total extinction

When absorption and scattering are both present, define:

\[
\sigma_t(\lambda)=
\sigma_a(\lambda)+\sigma_s(\lambda)
\]

The unscattered transmittance is then:

\[
T_r(d,\lambda)=e^{-\sigma_t(\lambda)d}
\]

The distance to the next interaction in a homogeneous medium can be sampled from:

\[
p(s)=\sigma_t e^{-\sigma_t s},\qquad s\geq0
\]

At a sampled interaction, the scattering probability for the usual absorption-versus-scattering model is:

\[
P_{\mathrm{scatter}}=\frac{\sigma_s}{\sigma_t}
\]

The complementary probability represents absorption.

These are probabilistic descriptions of photon or radiometric transport, not exact trajectory descriptions of an individual electromagnetic field.

## 7.3 Scattering direction

The scattering phase function describes the angular distribution of scattered light.

A common approximate phase function is Henyey–Greenstein:

\[
p(\cos\theta)=
\frac{1-g^2}
{4\pi(1+g^2-2g\cos\theta)^{3/2}}
\]

where \(g\) is the asymmetry parameter. This model is widely useful for participating media, but it does not exactly represent every physical scattering mechanism.

Other models should include Rayleigh scattering for appropriate small-particle regimes and Mie-based scattering for suitable spherical particles.

### Proposed volume module

`Spectral.Volumes` should have separate implementations for:

- `HomogeneousAbsorptionModel`
- `GridAbsorptionModel`
- `HomogeneousScatteringModel`
- `HenyeyGreensteinPhaseFunction`
- `RayleighScatteringModel`
- `MeasuredVolumeScatteringModel`
- `SubsurfaceTransportModel`
- `VolumeEmissionModel`

Use physical coefficients with declared units and wavelength dependence.

A generic RGB tint for smoke or glass is acceptable as an artistic fallback, but it must not be labelled as measured physical absorption.

# 8. Emission and thermal radiation

Materials can also generate light. A complete optical renderer therefore needs emission models in addition to reflection and transmission.

## 8.1 Thermal radiation

The spectral radiance of an ideal blackbody at temperature \(T\), expressed per unit wavelength, is given by Planck's law:

\[
B_\lambda(T)=
\frac{2hc^2}{\lambda^5}
\frac{1}{e^{hc/(\lambda k_B T)}-1}
\]

where:

- \(h\) is Planck's constant.
- \(c\) is the speed of light in vacuum.
- \(k_B\) is Boltzmann's constant.
- \(\lambda\) is the wavelength in metres.

Real materials have emissivities that depend on wavelength and can depend on direction and polarization. For a material in thermal equilibrium with an appropriate reciprocal optical response, Kirchhoff's law relates directional emissivity to absorptivity under matching conditions.

Spectral should implement `BlackbodyEmissionModel` and `MeasuredEmissivityModel` separately, allowing them to be composed where the physical assumptions are satisfied.

## 8.2 Non-thermal emission

The architecture should also allow other emission models, including:

- Area lights and calibrated emitters.
- Spectrally measured light sources.
- Fluorescence and wavelength-changing emission.
- Other specialized luminescent or stimulated emission models where the intended application requires them.

A simple emissive texture is not a substitute for a calibrated spectral power distribution. Emission should be expressed in physical radiometric units when physical output is required.

# 9. Spectral rendering equation and light transport

Now the individual physical models need to be brought together by the integrators.

For a surface, the spectral rendering equation can be written as:

\[
\begin{aligned}
L_o(\mathbf x,\omega_o,\lambda)
={}&L_e(\mathbf x,\omega_o,\lambda)\\
&+\int_{\Omega}
f_s(\mathbf x,\omega_i,\omega_o,\lambda)
L_i(\mathbf x,\omega_i,\lambda)\\
&\qquad\cdot|\mathbf n\cdot\omega_i|\,
d\omega_i
\end{aligned}
\]

Here:

- \(L_o\) is outgoing spectral radiance.
- \(L_e\) is emitted spectral radiance.
- \(f_s\) is the relevant spectral BSDF.
- \(L_i\) is incident spectral radiance.
- \(\Omega\) is the domain of incoming directions.
- \(\mathbf n\) is the surface normal.

This integral describes how emitted light and reflected or transmitted incident light contribute to the outgoing radiance.

In participating media, the rendering equation must be extended with volume interactions and transport along the path.

The equation generally cannot be evaluated analytically for a complex scene. Monte Carlo path tracing estimates it using samples. This is the foundation of modern spectral renderers such as PBRT v4, which combines spectral transport, volumetric methods and GPU support. 【turn617040view1】

## 9.1 Monte Carlo estimator

For a single directional integral, an unbiased Monte Carlo estimator has the general form:

\[
\widehat L_o =
L_e+
\frac{1}{N}\sum_{j=1}^{N}
\frac{
f_s(\omega_{i,j},\omega_o,\lambda)
L_i(\omega_{i,j},\lambda)
|\mathbf n\cdot\omega_{i,j}|
}{
p(\omega_{i,j})
}
\]

where \(p\) is the probability density used to sample each incoming direction.

This is a conceptual estimator for the surface integral; a complete path tracer also accounts for multiple bounces, light sampling, visibility, Russian roulette, and the probability densities used along the entire path.

### Integrator responsibilities

`Spectral.Integrators` should implement, at a minimum:

1. Direct-light estimation.
2. Indirect spectral path tracing.
3. Multiple importance sampling.
4. Russian roulette termination.
5. Volumetric path tracing.
6. Caustic-focused techniques where needed.
7. Adaptive sampling and convergence estimation.

These are transport algorithms. They should not own their own independent implementations of the Fresnel equations or the material database.

# 10. Wave optics: phase, interference and coherence

This is the feature that fundamentally distinguishes Spectral from a standard physically based path tracer.

A traditional path tracer accumulates contributions to radiometric quantities. It normally does not retain the relative phase of every field contribution arriving at a sensor. Therefore, it cannot reproduce arbitrary coherent interference simply by adding more ray bounces.

## 10.1 Phase accumulation

For a monochromatic wave travelling through a non-absorbing medium along a path \(\Gamma\), the accumulated propagation phase is:

\[
\phi_{\mathrm{prop}}=
\frac{2\pi}{\lambda_0}
\int_\Gamma n(\mathbf x,\lambda_0)\,ds
\]

The integral is the optical path length multiplied by the vacuum wavenumber.

The total phase must also include phase shifts produced by interfaces and other optical interactions where applicable.

For two paths, the phase difference is:

\[
\Delta\phi=
\frac{2\pi}{\lambda_0}
\Delta\mathrm{OPL}
+\Delta\phi_{\mathrm{interfaces}}
\]

The phase relationship determines whether the field contributions reinforce or cancel.

## 10.2 Superposition and interference

The electric fields add linearly:

\[
\mathbf E_{\mathrm{total}}=
\sum_j \mathbf E_j
\]

The measured intensity is proportional to the time-averaged squared magnitude of the total field. For two scalar fields at the same optical frequency:

\[
I=I_1+I_2+
2\operatorname{Re}
\left\{
\left\langle E_1E_2^*\right\rangle
\right\}
\]

For fully coherent contributions with equal frequency, this becomes:

\[
I=I_1+I_2+
2\sqrt{I_1I_2}\cos(\Delta\phi)
\]

This explains constructive and destructive interference.

When the fields are mutually incoherent, or the phase relationship varies sufficiently over the relevant measurement interval, the cross term can average toward zero. This is why an engine cannot treat all light as mutually coherent all the time.

## 10.3 Temporal and spatial coherence

Spectral must support a declared illumination coherence model, including as appropriate:

- Central wavelength and spectral bandwidth.
- Spatial coherence.
- Temporal coherence.
- Polarization coherence.
- Phase correlation between emitters or paths.

A real extended light source may be only partially coherent. Correctly modelling it requires more than assigning a single phase to the entire scene.

### Coherent data types

The coherent subsystem needs data structures such as:

```
struct ComplexFieldSample
{
    ComplexVector3 electricField;
    double vacuumWavelengthMeters;
    double angularFrequency;
};

struct CoherentPathState
{
    double accumulatedPhaseRadians;
    double opticalPathLengthMeters;
    double travelledDistanceMeters;
    ComplexFieldSample field;
};
```

These are illustrative starting types, not the complete field representation for every optical situation. A production implementation must also define coordinate frames, frequency conventions, polarization basis, field normalization and the meaning of amplitude.

Crucially, `CoherentPathState` should not be forced into every conventional geometric ray. The geometric path tracer and coherent transport solver can use shared geometry and material data while maintaining different transport state.

# 11. Diffraction and wavefront propagation

Diffraction occurs when the wave interacts with apertures, edges, obstacles, or structures whose geometry is relevant to the wavelength.

A geometrical ray model alone cannot represent general diffraction. Dedicated wave-aware techniques can, however, integrate diffraction into rendering more efficiently than a full electromagnetic solution for every object.

Research published in 2024 demonstrated an edge-based Fraunhofer diffraction BSDF that can reproduce diffraction lobes using ray-tracing-compatible techniques for suitable configurations. This is a useful specialized model to investigate for Spectral. 【turn466372view0】

## 11.1 Fraunhofer diffraction

In the far-field approximation, the scalar diffracted field from an aperture can be expressed as a Fourier-transform-like integral:

\[
U(\theta_x,\theta_y)\propto
\iint_A U_0(x,y)
e^{-ik[
x\sin\theta_x+y\sin\theta_y
]}\,dx\,dy
\]

where \(U_0(x,y)\) is the complex field across the aperture and \(A\) is the aperture area.

The intensity distribution is proportional to:

\[
I(\theta_x,\theta_y)\propto
|U(\theta_x,\theta_y)|^2
\]

This formulation assumes suitable scalar and far-field approximations. It is not a universal exact solution for arbitrary electromagnetic fields.

For a single slit of width $(a)$, the familiar intensity pattern is:

$I(\theta)=I(0)
\left(\frac{\sin\beta}{\beta}\right)^2,
\qquad
\beta=\frac{\pi a\sin\theta}{\lambda_0}$

The minima occur when $(\beta)$ is a nonzero integer multiple of $(\pi)$.

## 11.2 Fresnel diffraction

For nearer-field propagation under the paraxial approximation, a Fresnel integral can be used:

$\begin{aligned}
U(x,y,z)\approx{}&
\frac{e^{ikz}}{i\lambda_m z}
\iint U_0(\xi,\eta)\\
&\cdot
\exp\left[
\frac{ik}{2z}
\big((x-\xi)^2+(y-\eta)^2\big)
\right]d\xi\,d\eta
\end{aligned}$

Here $(\lambda_m)$ and $(k=2\pi/\lambda_m)$ refer to the propagation medium, assuming a homogeneous, lossless medium for this expression.

This approximation must not be applied blindly to near-field problems with high numerical aperture, evanescent fields, or other conditions outside the paraxial regime.

## 11.3 Thin-film interference

Thin films require a separate multilayer optical model. For a single film of refractive index \(n_f\), thickness \(d\) and internal propagation angle \(\theta_f\), the round-trip propagation phase is:

$\delta_{\mathrm{round}}=
\frac{4\pi n_f d\cos\theta_f}{\lambda_0}$

The total interference also depends on the phase shifts and amplitudes at the interfaces. For multiple layers, a transfer-matrix method is appropriate for many planar, layered structures.

This can reproduce optical effects such as interference colours from thin transparent films, coatings and suitable multilayer surfaces.

## 11.4 Wave-optics module breakdown

`Spectral.WaveOptics` should offer distinct algorithms with explicit assumptions:

| Model | Main use |
| --- | --- |
| `CoherentPathModel` | Phase accumulation and coherent path contributions |
| `FraunhoferDiffractionModel` | Far-field aperture and suitable edge diffraction |
| `FresnelDiffractionModel` | Paraxial near-field diffraction |
| `ThinFilmInterferenceModel` | Interference in planar layered media |
| `PeriodicStructureModel` | Suitable diffraction gratings and periodic surfaces |
| `WavefrontPropagationModel` | Numerical propagation of complex wavefronts |
| `WaveTracingIntegrator` | Wave-aware light transport across scene geometry |

Workbench should display which of these models was used, which approximations apply, and which physical effects were not simulated.

# 12. Full-field electromagnetic solver

Some problems cannot be solved adequately by a surface BSDF or a wave-tracing approximation.

Examples include light interaction with nanoscale structures, complex periodic optical surfaces, and situations where near-field interactions or detailed electromagnetic boundary effects matter.

For these cases, Spectral should support a separate solver subsystem.

## 12.1 Candidate numerical methods

Finite-Difference Time-Domain (FDTD) discretizes Maxwell's equations in space and time. It is useful for modelling transient electromagnetic fields, but spatial resolution and time-step stability can make large domains costly.

Frequency-domain finite-element methods (FEM) solve electromagnetic field equations for a specified frequency, using a mesh and suitable boundary conditions. They are useful for geometrically complex domains and material interfaces.

Frequency-domain finite-difference methods offer another approach for structured discretizations.

No single solver should be considered best for all geometry, frequency ranges and boundary conditions. The selection should depend on the physics being modelled, the shape of the computational domain, material behaviour, accuracy requirements and computational budget.

## 12.2 Solver inputs and outputs

The solver interface should accept:

- A selected geometric region.
- Boundary conditions.
- Material constitutive models.
- Incident field or source definition.
- Frequency or wavelength configuration.
- Required numerical accuracy.
- Domain discretization parameters.

It should return validated field samples, power flux, scattering data, or a derived optical response suitable for coupling with the renderer.

A solver should report mesh and discretization quality, convergence information, residuals where applicable, and its assumptions. A result without numerical-quality information is not sufficient for a demanding production workflow.

## 12.3 Hybrid coupling

The most practical architecture is to allow a high-fidelity electromagnetic calculation to generate a reusable optical response for a structure, which can then be applied by the rendering system under compatible illumination, geometry, wavelength, incidence, and polarization conditions.

However, such a response cannot necessarily be reduced to a conventional BRDF: coherent, near-field, or position-dependent effects can require richer data representations.

A specialized field solver should never pretend that it is an exact drop-in replacement for every ordinary surface material.

# 13. Physically meaningful material assets

Now connect the mathematics to the actual editor workflow.

A Spectral material asset must hold physical optical properties and the information needed to interpret them correctly.

## 13.1 Three classes of material data

| Data class | Examples | Meaning |
| --- | --- | --- |
| Appearance data | Base-colour texture, roughness map, normal map | Authoring inputs and visual approximations |
| Physical property data | \(n(\lambda)\), \(\kappa(\lambda)\), spectral reflectance, layer thickness, scattering coefficients | Optical properties used by the simulation |
| Structural data | Mesh geometry, measured surface-height field, layer arrangement, particle distribution | The physical structure that produces the optical response |

These categories should not be silently conflated.

For example, a base-colour texture may be an authored reflectance approximation rather than a measurement. A normal map is not necessarily a physical surface-height measurement. A roughness map only becomes meaningful after Spectral defines the scattering model and parameter mapping.

## 13.2 Example material definition

A material descriptor could look like this:

```
{
  "schemaVersion": 1,
  "name": "MeasuredTransparentDielectric",
  "model": "Spectral.Interfaces.Dielectric",
  "physicalData": {
    "refractiveIndex": {
      "model": "tabulated",
      "asset": "optical/n_data.csv",
      "wavelengthUnit": "nm"
    },
    "extinctionCoefficient": {
      "model": "tabulated",
      "asset": "optical/k_data.csv",
      "wavelengthUnit": "nm"
    },
    "dataConditions": {
      "temperatureK": 293.15
    }
  },
  "surface": {
    "roughnessModel": "GGX",
    "roughnessSource": "measured_or_authored"
  },
  "validation": {
    "requireValidSpectralRange": true,
    "requireDataProvenance": true
  }
}
```

This is a proposed configuration format, not an existing engine API. Its exact schema will evolve as the physical models mature.

The material should also store measurement provenance, units, uncertainty if known, valid wavelength range, interpolation method, and licensing or distribution metadata where relevant.

For materials that cannot be described by a scalar complex refractive index, the schema must support richer constitutive models instead of forcing them into this simplified format.

## 13.3 Material validation

Before rendering, the engine should check that:

- The wavelength range of the material covers the range used by the integrator, or a documented fallback is selected.
- Complex optical data are finite and physically interpretable under the selected convention.
- Reflectance and probability distributions satisfy the required bounds and normalization.
- Absorption, scattering and emission units are correct.
- Surface and volume models are compatible with the declared material structure.
- The selected transport algorithm supports the requested effects.
- The data conditions are appropriate for the simulation conditions.

Invalid material parameters should trigger a diagnostic or a deliberate fallback, not silently generate plausible-looking but physically meaningless output.

# 14. Implementing the modular physics API

The implementation should use stable interfaces for model composition while permitting optimized, specialized execution in the hot loops.

## 14.1 Shared interfaces

A simplified C++ API could begin with the following:

```
#pragma once

#include <memory>
#include <span>

namespace Moonlight::Spectral
{
    struct OpticalQuery
    {
        double wavelengthMeters;
        double temperatureKelvin;
    };

    struct ComplexIndex
    {
        double refractiveIndex;
        double extinctionCoefficient;
    };

    class IOpticalPropertyModel
    {
    public:
        virtual ~IOpticalPropertyModel() = default;

        virtual ComplexIndex Evaluate(
            const OpticalQuery& query) const = 0;
    };

    struct InterfaceQuery
    {
        OpticalQuery optical;
        double cosIncident;
        double indexIncident;
    };

    struct FresnelResult
    {
        double reflectanceS;
        double reflectanceP;
        double transmittanceS;
        double transmittanceP;
        bool totalInternalReflection;
    };

    class IInterfaceModel
    {
    public:
        virtual ~IInterfaceModel() = default;

        virtual FresnelResult Evaluate(
            const InterfaceQuery& query) const = 0;
    };
}
```

This is only a starting interface for simplified optical models. Real implementations must handle transmitted medium properties, polarization conventions, complex refractive indices, power normalization, wavelength-dependent refraction and applicable edge cases.

Likewise, a volume interaction should be modelled separately from a boundary interaction. Do not add every optical parameter to one enormous `Material` class.

## 14.2 Model composition

A material should compose its optical properties and physical structure. The transport algorithm should then query the relevant model for the current interaction.

A conceptual model might be:

```
PhysicalMaterial
├── OpticalProperties
│   ├── DispersionModel
│   ├── ComplexIndexModel
│   └── SpectralReflectanceModel
├── SurfaceStructure
│   ├── MicrofacetDistribution
│   └── LayerStructure
├── BoundaryModels
│   ├── ReflectionModel
│   └── RefractionModel
├── VolumeModels
│   ├── AbsorptionModel
│   ├── ScatteringModel
│   └── EmissionModel
└── OptionalWaveModels
    ├── CoherenceModel
    ├── ThinFilmModel
    └── DiffractionModel
```

Not every material implements every interface. A metal, a transparent dielectric, a smoke volume and a diffractive coating have different physical behaviour.

## 14.3 C++, Rust and C#

To fit Moonlight's wider technology stack:

- C++ should own the primary rendering kernels, geometry traversal, GPU integration, physically based model implementations and performance-critical numerical algorithms.
- Rust is a good candidate for isolated systems that benefit from strong memory-safety guarantees, such as material dataset ingestion, schema validation, asset conversion, metadata handling and standalone data-processing utilities. It should only be used where its integration overhead is justified.
- C# should provide Workbench integration, asset authoring, model configuration, diagnostics, visualizers and managed editor tooling.

Avoid defining three separate versions of the same formula in three languages. The authoritative mathematical implementation should live in one place, with shared reference tests, stable APIs, and clearly specified numerical semantics.

# 15. GPU and CPU execution model

A renderer that offers multiple physical fidelity modes needs to manage a range of different computational workloads.

A conventional spectral path tracer is dominated by ray traversal, material evaluation, sampling and transport. A wave-optics solver can instead be dominated by complex arithmetic, discretized field updates, matrix operations, or large numerical systems.

These workloads should not be forced into one backend implementation.

## 15.1 Backend architecture

`Spectral.Backends` should define hardware-independent contracts for:

- Ray-tracing acceleration structures.
- Buffer and texture management.
- Shader compilation and dispatch.
- Spectral data access.
- Compute kernels.
- Numerical solver execution.
- Synchronization and resource lifetime.

Then implement the necessary CPU and GPU backends.

Possible GPU approaches include Vulkan ray-tracing interfaces, DirectX Raytracing, or vendor-specific APIs behind an appropriate abstraction. Solver backends may need different execution capabilities from ray-tracing backends.

## 15.2 Wavefront path tracing

For complex scenes, implement a wavefront path tracer in which ray generation, intersection, material evaluation, light sampling, and path continuation are separated into stages.

This offers a practical foundation for GPU execution, debugging, and the integration of more complex transport models.

However, wavefront ray tracing does not automatically preserve coherent phase. The coherent path representation and its algorithm must be designed explicitly.

## 15.3 Numerical precision

Use:

- Single precision for appropriately conditioned high-throughput geometry and GPU operations.
- Double precision for reference calculations, sensitive accumulations and solver operations where required.
- Stable complex-number and matrix operations.
- Explicit dimensional tolerances and convergence criteria.
- Deterministic or reproducible sampling modes for testing.

Accuracy must be measured against a reference problem, not inferred from the number of decimal places stored.

# 16. Workbench integration

Workbench should make the physical complexity of Spectral accessible without hiding the limits of the underlying simulation.

## 16.1 Material inspector

The material inspector should be divided into several panels.

## Spectral Material Inspector

Identity & Structure

Material type, physical layers, surface structure, volume definition and linked assets.

Optical Constants

Plots of \(n(\lambda)\), \(\kappa(\lambda)\), reflectance, transmission, and material data validity.

Wave Properties

Phase models, coherence settings, wavelength ranges, diffraction and thin-film configuration.

Simulation & Validation

Integrator selection, numerical precision, unsupported features, convergence, warnings and measured-data provenance.

## 16.2 Simulation configuration

The renderer should expose a configuration that states which physical effects are active, rather than simply offering a single universal quality slider.

```
{
  "integrator": "spectral_path",
  "spectrum": {
    "mode": "sampled",
    "wavelengthRangeNm": [380, 780]
  },
  "features": {
    "dispersion": true,
    "polarization": true,
    "volumeScattering": true,
    "coherentTransport": false,
    "diffraction": "specialized"
  },
  "sampling": {
    "samplesPerPixel": 1024,
    "maximumPathDepth": 16
  },
  "diagnostics": {
    "validateMaterials": true,
    "reportConvergence": true
  }
}
```

These values are illustrative starting settings, not universal production defaults. Real applications need workload-specific presets, and the correct wavelength range depends on the simulation objective.

## 16.3 Render diagnostics

Workbench should display:

- Which mathematical model handled each interaction.
- Whether material properties came from measurements or approximations.
- Which approximations were applied.
- Where a material's data range was exceeded.
- Sampling noise and convergence estimates.
- Energy conservation or solver-residual diagnostics where meaningful.
- Estimated runtime and memory consumption.
- Physical parameters used for the final render.

This makes the engine useful for engineering and research workflows as well as visual production.

# 17. Colour output and camera modelling

Spectral should perform transport in a spectral representation, then convert the results to the output expected by the camera or display.

For CIE tristimulus values, a simplified integration is:

$X=K\int L_\lambda(\lambda)\,
\overline{x}(\lambda)\,d\lambda$

$Y=K\int L_\lambda(\lambda)\,
\overline{y}(\lambda)\,d\lambda$

$Z=K\int L_\lambda(\lambda)\,
\overline{z}(\lambda)\,d\lambda$

where \(\overline{x}\), \(\overline{y}\) and \(\overline{z}\) are the selected colour-matching functions and \(K\) is the appropriate normalization for the intended quantity.

In a practical renderer, sampled spectral integration may be used instead of continuous integration. PBRT v4 provides an important architectural precedent for doing spectral transport internally rather than conducting all transport directly in RGB. 【turn617040view1】

Spectral should also distinguish:

1. Spectral scene radiance.
2. The camera or sensor response.
3. Exposure and image formation.
4. Colour-space conversion.
5. Display encoding and tone mapping.

These are separate stages. Tone mapping should not be used to conceal physical errors in the renderer.

For physical camera simulation, add aperture shape, lens transmission, sensor spectral sensitivity and other relevant optical properties as separate modules.

# 18. Validation and testing strategy

For an engine intended for enterprise production, visual plausibility is not a sufficient test.

Every physical module needs mathematical unit tests, analytic reference cases, integration tests and reproducibility checks. The high-fidelity solver also needs convergence studies.

## 18.1 Minimum test suite

| Test | Expected result |
| --- | --- |
| Vacuum propagation | Correct relationship between wavelength, frequency and phase velocity |
| Dispersion | Wavelength-dependent refractive index matches the model's reference data |
| Normal-incidence Fresnel reflection | Matches the analytic normal-incidence formula |
| Total internal reflection | Correct threshold and physically consistent reflected behaviour |
| Absorbing slab | Matches Beer–Lambert attenuation for the homogeneous bulk |
| Energy conservation | Lossless interface and scattering cases satisfy applicable power-conservation constraints |
| Volume transmittance | Matches the analytic homogeneous-medium result |
| Polarization | Matches established Jones/Stokes/Mueller reference cases |
| Two-path interference | Matches the expected phase-dependent intensity distribution |
| Single-slit diffraction | Matches the analytic Fraunhofer intensity distribution in its validity regime |
| Thin-film interference | Matches a transfer-matrix reference |
| Spectral integration | Converges toward a high-sample or otherwise trusted reference |
| Full-field solver | Demonstrates convergence under mesh and time-step or frequency-domain discretization refinement |

## 18.2 Numerical requirements

The engine should define tolerances per model and test case. For example, normal-incidence reflectance can be compared against an analytic result, while a stochastic global-illumination render needs a statistical error metric or confidence interval.

For full-field calculations, require evidence of convergence as the mesh is refined or the time step is reduced.

For path tracing, track sampling uncertainty separately from model approximation error. A noiseless render is not necessarily physically accurate, and a physically correct integrator can produce noisy estimates at insufficient samples.

## 18.3 Automated reference tests

Every new model should include:

- Tests with known analytic solutions where available.
- Tests for limiting cases.
- Invalid-input tests.
- Conservation and normalization tests.
- Performance benchmarks.
- Cross-platform consistency tests when relevant.

The most sensitive algorithms should have independent reference implementations so that a shared mistake does not pass every test.

# 20. Final engineering decisions

The design should be governed by a few non-negotiable rules.

First, physical models must be modular. Reflection, refraction, absorption, scattering, emission, polarization, interference and diffraction must be independent implementations that can be tested and replaced without rewriting the entire renderer.

Second, every model must declare its assumptions. An analytic microfacet BRDF, a stochastic volume model, a wave-tracing algorithm and a finite-difference electromagnetic solver are not interchangeable merely because they can all produce an image.

Third, measured data must be a first-class part of the asset system. A physical material definition needs units, validity ranges, measurement conditions, provenance and uncertainty where available. The editor must distinguish measured properties from artist-authored approximations.

Fourth, spectral transport must precede advanced wave simulation. The spectral path tracer, physical material library and numerical validation framework establish the foundation for the substantially more difficult phase-aware and full-field implementations.

Fifth, never promise universal exactness. A scientifically grounded engine can produce results that agree closely with reality for a well-specified experiment or scene. An arbitrary textured model, without adequate material measurements and physical structure, cannot determine the unique appearance of the real object.

The recommended implementation target is therefore:

Spectral = measured-data-driven spectral path tracing + optional polarization + coherent wave transport + specialized full-field solvers, all coordinated through a modular physical-model API.