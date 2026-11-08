# Research reference index

**Status:** Maintained index  
**Purpose:** Record external sources that inform Moonlight design decisions.

This index records source identity and engineering consequence, not copied
source text. A source may inform a design without becoming a runtime
dependency. Access dates are recorded because specifications and URLs change.

## Graphics and Vulkan

| Source | Used for | Consequence |
| --- | --- | --- |
| Khronos Group, *Vulkan Documentation Project*, Vulkan 1.3 reference and synchronization tutorials, accessed 2026-11-08 | Synchronization 2 and timeline semaphores | The renderer models explicit resource hazards and timeline values; it does not use device-wide idle waits per frame. |
| Khronos Group, *Vulkan Guide*, descriptor indexing and ray-tracing chapters, accessed 2026-11-08 | Bindless descriptors and acceleration structures | Resource indices are stable handles with frame-safe retirement; BLAS/TLAS lifetime is explicit. |
| Khronos Group, *SPIR-V Specification*, accessed 2026-11-08 | Shader ABI and capability validation | Shader builds record target environment and reflection data; unsupported capabilities fail validation. |

## Light transport and material models

| Source | Used for | Consequence |
| --- | --- | --- |
| Pharr, Jakob, Humphreys, *Physically Based Rendering: From Theory to Implementation*, 4th ed., 2023 | Rendering equation, Monte Carlo estimators, MIS, BSDFs, volumes | Spectral separates estimators from material evaluation and reports sampling/error policy. |
| Jakob et al., *Mitsuba 3: A Retargetable Forward and Inverse Renderer*, ACM TOG, 2022 | Differentiable and physically based renderer architecture | Experimental workflows use explicit parameterization and provenance rather than mutating production assets. |
| Nicodemus et al., *Geometrical Considerations and Nomenclature for Reflectance*, NBS Monograph 160, 1977 | BRDF terminology | Material contracts identify incident/outgoing directions and test reciprocity and energy constraints. |
| CIE 015:2018, *Colorimetry*, 4th ed. | CIE XYZ and colorimetric conversion | Display conversion is an image-formation stage, not a substitute for spectral transport. |

## Audio and acoustics

| Source | Used for | Consequence |
| --- | --- | --- |
| ISO 9613-1, *Acoustics — Attenuation of sound during propagation outdoors*, 1993 | Atmospheric absorption | Air absorption is frequency-dependent and requires environmental parameters, not a scalar volume fade. |
| ISO 3382-1, *Acoustics — Measurement of room acoustic parameters*, 2009 | Reverberation validation | Orcha reference scenes report RT60 and response metrics with measurement configuration. |
| Microsoft Research, *Interactive Sound Propagation with Directional Sources and Listeners*, accessed 2026-11-08 | Directional propagation fields | Precomputation and runtime spatialization are separate products; source/listener directivity is retained. |
| Blauert, *Spatial Hearing*, revised edition | HRTF and auditory localization | HRTF selection, interpolation, head orientation, and output layout are explicit runtime inputs. |

## Data-oriented runtime architecture

| Source | Used for | Consequence |
| --- | --- | --- |
| Berger, *Build your own ECS: change detection, events, tags, and commands*, accessed 2026-11-08 | Practical archetype ECS mechanisms | Atlas uses deferred structural changes, change tracking, event channels, and schedule-owned access declarations. |
| C++ Core Guidelines, accessed 2026-11-08 | Ownership, lifetime, and error handling | Native APIs make ownership and failure explicit; borrowed views cannot outlive their snapshot or command scope. |

## Build and reproducibility

| Source | Used for | Consequence |
| --- | --- | --- |
| CMake documentation, *cmake-presets(7)*, accessed 2026-11-08 | Shared configure/build/test workflows | Checked-in presets are portable defaults; machine-specific values belong in user presets or toolchain files. |
| ISO/IEC 14882 and Rust Reference, accessed 2026-11-08 | Language and ABI boundaries | C++ object layout and Rust ABI are not exposed across the stable plugin boundary; the C ABI owns interop. |
| Wilkinson et al., *The FAIR Guiding Principles for scientific data management and stewardship*, Scientific Data, 2016 | Research data stewardship | Experiments carry provenance, identifiers, schemas, checksums, and enough metadata to rerun or audit a result. |

## Citation policy

Each new technical claim should cite a primary standard, paper, or official
implementation reference where practical. Search-result summaries are not
citations. If a source cannot be accessed or its version is uncertain, mark the
claim as a design assumption and create a research task rather than presenting
it as settled fact.
