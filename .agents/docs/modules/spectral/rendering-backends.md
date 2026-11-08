# Spectral rendering backends

**Status:** Design target
**Scope:** Vulkan-first GPU execution, with portable backend boundaries

This document defines the implementation contract between Spectral's transport
algorithms and the graphics API. Vulkan is the primary native backend because it
provides explicit synchronization, descriptor indexing, asynchronous compute,
ray-tracing extensions, and a stable cross-platform model. Direct3D 12 and
Metal may be added behind the same resource and command interfaces; neither
backend may change the physical meaning of a Spectral result.

## Backend responsibilities

The backend owns:

- `VkInstance`, physical-device selection, feature negotiation, and device
  lifetime.
- Queues and queue-family ownership for graphics, compute, transfer, and
  optional video workloads.
- `VkDeviceMemory` allocation, suballocation, residency, staging, and
  defragmentation.
- Buffer, image, image-view, sampler, acceleration-structure, and descriptor
  lifetime.
- Command-pool recycling, command-buffer recording, submission, fences, and
  timeline semaphores.
- Shader-module and pipeline creation from validated SPIR-V.
- GPU timestamps, calibrated timestamps where available, debug labels, and
  validation-layer diagnostics.

The transport and material layers own:

- Unit conversions, wavelength and spectrum sampling, probability densities,
  BSDF/volume evaluation, and numerical tolerances.
- Ray-generation policy, path state, Russian roulette, multiple importance
  sampling, and estimator construction.
- Voxel hierarchy semantics, cache invalidation, confidence, and reference
  comparisons.

The backend must not silently clamp physical values to make a GPU path appear
stable. Unsupported features must be reported and route the workload to a
declared fallback, such as CPU execution or a lower-fidelity profile.

## Device selection and feature tiers

Device selection is deterministic and recorded in the render result metadata.
The selection score should consider required features, memory budget, queue
topology, ray-tracing support, subgroup operations, floating-point atomics,
timestamp support, and a configurable vendor preference. Do not select a
device solely by name or maximum advertised memory.

Define explicit capability tiers:

| Tier | Required capabilities | Supported workloads |
| --- | --- | --- |
| CPU/reference | Host implementation and deterministic math | Validation, reference path tracing, CI |
| Vulkan compute | Vulkan 1.3, synchronization2, descriptor indexing, storage buffers/images | Voxel GI, wavefront path tracing, denoising |
| Vulkan ray tracing | Tier-1.2 acceleration structures and ray-query or pipeline support | Hardware-accelerated visibility and path tracing |
| Vulkan advanced | Subgroup controls, float atomics, sparse residency, calibrated timestamps where available | High-throughput caches and research workloads |

At startup, negotiate features through `VkPhysicalDeviceFeatures2` and the
relevant extension feature structures. Store the enabled feature set in the
runtime diagnostics report. Pipeline creation must use only negotiated
features; checking an extension name without enabling its feature structure is
not sufficient.

## Resource model

Use explicit resource classes rather than exposing raw Vulkan handles to
Spectral algorithms:

```cpp
struct GpuBuffer {
    uint64_t id;
    VkBuffer handle;
    VkDeviceSize size;
    VkDeviceSize alignment;
    VkBufferUsageFlags usage;
    uint32_t queue_family;
};

struct GpuImage {
    uint64_t id;
    VkImage handle;
    VkImageView default_view;
    VkFormat format;
    VkExtent3D extent;
    VkImageLayout layout;
    uint32_t queue_family;
};
```

Production code should wrap these handles in RAII owners and expose opaque
resource IDs across the C ABI. A resource ID is not a pointer and is never
valid after its destruction event. Every command records the resource revision
it reads or writes so the render graph can detect use-after-retire errors.

Buffers used by shaders should be separated by access pattern:

- Device-local immutable geometry and material tables.
- Device-local streaming voxel and radiance storage.
- Host-visible upload/readback rings.
- Small persistent uniform or push-constant data.
- Scratch buffers for compaction, sorting, prefix sums, and ray tracing.

Large arrays use structure-of-arrays layouts where shader access is coherent.
Every GPU struct has a declared alignment and scalar layout. C++ host structs
must be checked against generated SPIR-V reflection; never assume that a C++
ABI layout matches `std430`, scalar layout, or a language-specific shader
layout by accident.

## Bindless descriptors

Spectral scenes contain many textures, spectra, material records, and
acceleration structures. The preferred Vulkan layout uses descriptor indexing:

- A global set contains sampled images, storage images, samplers, buffers, and
  acceleration structures indexed by stable handles.
- Per-pass sets contain only the small set of buffers that changes per pass.
- Material records store integer descriptor indices, not raw pointers.
- Descriptor indices remain valid until a frame-safe retirement point.
- Updating a descriptor array is deferred when a previously submitted command
  may still read it.

Fallback hardware without descriptor indexing uses bounded descriptor pools and
material batches. This fallback may reduce throughput but must preserve output
semantics and validation coverage.

## Render graph and synchronization

Every GPU pass declares resource uses:

```text
Pass: InjectDirectLight
  reads:  SceneGeometry[shader-read], LightTable[shader-read]
  writes: VoxelRadiance[compute-write], VoxelConfidence[compute-write]
  queue:  compute
```

The render graph compiles these declarations into:

1. A dependency DAG and topological execution order.
2. Vulkan image-layout transitions and buffer access masks.
3. Synchronization-2 barriers using `VkDependencyInfo`.
4. Queue ownership transfers when a resource changes queue family.
5. Timeline-semaphore waits and signals between queue submissions.
6. Transient resource aliasing only when lifetimes do not overlap.

Prefer synchronization2 and timeline semaphores. Binary semaphores are reserved
for external acquire/present integration. A pass must not rely on an implicit
global device idle. `vkDeviceWaitIdle` is a shutdown or exceptional recovery
operation, never a frame-to-frame synchronization strategy.

The frame scheduler uses a bounded number of frames in flight. Each frame owns
an upload ring, command pools, transient descriptor allocations, timestamp
queries, and a retirement value. Deferred destruction waits until the associated
timeline value has completed. This applies equally to buffers, images,
pipelines, descriptor pools, acceleration structures, and shader modules.

## Ray tracing path

The ray-tracing backend separates:

- Bottom-level acceleration structures for mesh geometry.
- Top-level instances for transforms, visibility masks, and hit-group selection.
- Shader binding tables for ray generation, miss, closest-hit, any-hit, and
  callable programs.
- Scratch and compaction buffers.
- Shader-record data containing material and geometry indices.

Build flags distinguish fast-build/update structures from fast-trace structures.
Dynamic objects use updateable BLASes only when measured rebuild cost justifies
the additional memory and restrictions. TLAS updates are preferred for
transform-only changes.

The path tracer must define its payload layout and recursion policy. Recursive
ray tracing is optional; the default production path should support a wavefront
variant where ray queues are generated, compacted, sorted, and consumed by
separate compute passes. This makes queue pressure, memory use, and scheduling
observable and allows the same material kernels to run without hardware ray
tracing.

Any-hit shaders must be used selectively. Alpha-tested geometry, procedural
transparency, and thin-film evaluation can cause severe divergence. Material
flags should allow the accelerator to choose an opaque, masked, or transmissive
hit path.

## Voxel GI GPU pipeline

Voxel GI uses separate passes for:

1. Geometry injection into a sparse or bricked occupancy structure.
2. Material-reference and emission injection.
3. Direct-light injection.
4. Radiance propagation or cone-trace preparation.
5. Cache filtering and multilevel transition.
6. Surface queries and hybrid estimator integration.
7. Confidence, age, dirty-region, and invalidation updates.

Voxel updates are region-based. A scene revision produces dirty bounds and a
dependency list; unchanged bricks are not rewritten. Bricks are allocated from
a GPU freelist, compacted with prefix sums, and retired only after all queues
complete their reads. A clipmap origin change invalidates only the affected
rings, not the entire cache.

Sparse residency is an optimization, not a correctness requirement. The
fallback bricked representation must produce a valid result on devices without
sparse image support. Cache validity and confidence are part of the public
result, so a missing or stale brick cannot be mistaken for black physical
radiance.

## Shader compilation and ABI

Shader sources are compiled to SPIR-V in a reproducible toolchain. The build
records compiler version, target environment, optimization flags, source hash,
descriptor layout hash, and specialization constants. CI validates SPIR-V and
rejects shaders with unexpected capabilities or descriptor bindings.

Use generated reflection to produce:

- C++ descriptor-layout declarations.
- C# inspection metadata for Workbench.
- Resource binding validation.
- Push-constant size and alignment checks.
- Pipeline compatibility hashes.

Pipeline caches are keyed by device identity, driver version, shader hash,
layout hash, render profile, and specialization constants. A cache miss is
normal; a corrupt cache is deleted and rebuilt with a diagnostic event.

## Numerical precision and determinism

Spectral wavelengths, radiance, phase, and probability densities have explicit
precision policies. A fast interactive profile may use FP16 storage for
directional cache data while accumulating in FP32 or FP64 where required.
Reference and validation profiles must define whether fused operations,
subgroup ordering, atomics, and GPU vendor differences are permitted.

GPU execution is not assumed bitwise deterministic. Validation compares
statistical estimators with confidence intervals and separately tests
deterministic CPU fixtures. Seeds, sample counts, frame index, device identity,
and profile are recorded in output metadata.

## Interop and presentation

The native graphics module exposes opaque frame, image, and synchronization
objects through the versioned C ABI. C# receives handles and immutable metadata;
it never owns Vulkan objects directly. The Workbench requests render products
through commands and receives completion events or read-only mapped results.

Presentation integration handles swapchain acquire, format selection, color
space, HDR metadata, resize, minimized windows, and present-mode policy. Scene
rendering is decoupled from presentation so headless CI can render into
off-screen images and export EXR or diagnostic buffers without a window system.

## Diagnostics and failure handling

Enable validation layers and GPU-assisted validation in development builds when
available. Attach object names and debug labels to every resource and pass.
Capture:

- Device and enabled-feature report.
- Per-pass CPU/GPU timings and queue waits.
- Memory allocations, residency, and transient aliasing.
- Descriptor exhaustion and pipeline-cache misses.
- Barrier and queue-ownership statistics.
- Shader compilation and validation errors.
- Ray count, queue occupancy, cache hit rate, and voxel confidence.

Device loss, out-of-memory, unsupported feature, and shader validation errors
are explicit failures. The engine may switch to a declared fallback profile,
but it must emit the reason and mark the resulting render metadata with the
actual backend and fidelity regime.

## Minimum backend test matrix

- Vulkan instance and feature negotiation on every supported desktop platform.
- Render-graph barrier tests for every resource usage transition.
- Timeline semaphore ordering across graphics, compute, and transfer queues.
- Descriptor-indexing and bounded-descriptor fallback paths.
- BLAS/TLAS build, update, compaction, and deferred destruction.
- Shader reflection/layout mismatch detection.
- Voxel brick allocation, invalidation, streaming, and stale-data rejection.
- Headless EXR rendering without a swapchain.
- Device-loss and out-of-memory diagnostic paths where test injection exists.
- Reference scenes compared against the CPU path within profile-specific error
  budgets.
