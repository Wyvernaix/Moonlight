# Orcha acoustics, DSP, and validation

**Status:** Design target
**Sources:** `design-specification.md`

## Acoustic path algorithms

Orcha uses more than one propagation algorithm:

| Algorithm | Role |
| --- | --- |
| Direct path | Visibility, propagation delay, distance attenuation |
| Multi-ray occlusion | Partial obstruction for extended emitters |
| Specular reflections | Directional early reflections |
| Stochastic path tracing | Multibounce energy and diffuse reverberation |
| Image sources | Efficient specular paths in suitable rooms |
| Edge diffraction | Propagation around occluding edges |
| Portal/waypoint paths | Connected rooms, doors, corridors |
| Wave solver | Frequency-dependent behavior outside geometric validity |

Each path records distance, delay, arrival direction, materials encountered,
frequency transfer, reflection order, method classification, and confidence.

Dynamic geometry uses static acceleration structures for unchanged content and
refittable or incremental structures for moving objects. Gain and filter
changes are smoothed or crossfaded to prevent clicks and pumping.

## DSP boundary

Simulation workers publish immutable, bounded results into a lock-free or
otherwise non-blocking handoff. The audio callback may consume the latest
complete snapshot but must never wait for ray tracing, asset loading, room
baking, memory allocation, or a contended lock.

The DSP graph includes:

```text
Sources -> gain/source filters -> propagation delay/filters
        -> spatializer -> buses/sends -> effects -> master -> device
```

Required services include sample-accurate scheduling, buses, sends, EQ,
filters, convolution, delays, dynamics, resampling, automation, streaming,
voice virtualization, device recovery, latency diagnostics, and controlled
plugin processing.

## API shape

```cpp
struct AcousticPath {
    double pathLengthMeters;
    double arrivalTimeSeconds;
    Vector3 arrivalDirection;
    AcousticPathKind kind;
    AcousticTransferHandle transfer;
    float confidence;
};

struct AcousticFrameResult {
    AcousticPath directPath;
    Span<const AcousticPath> earlyReflections;
    RoomResponseHandle lateResponse;
    std::uint64_t sceneRevision;
    std::uint64_t simulationRevision;
};
```

Simulation requests return tickets so callers can distinguish pending,
complete, stale, cancelled, and failed work. The audio thread consumes
double-buffered or immutable results rather than synchronously querying a
worker.

## Validation scenes

Maintain reference scenes for free field, a single wall, layered transmission,
two-room portals, rectangular rooms, corridors, edge diffraction, porous
absorbers, and moving sources. Record geometry, source/listener coordinates,
material measurements, environmental conditions, solver settings, software
version, and expected tolerances.

Essential tests cover distance attenuation, propagation time, reflection and
transmission, air absorption, room response, Doppler, occlusion, diffraction,
HRTF directionality, convolution accuracy, callback deadlines, and failure
recovery. Analytical and measured references are preferred over comparing one
game-engine approximation to another.

## Workbench tools

The audio workspace should provide:

- Sound-event and layer editor.
- Acoustic material inspector.
- Direct/reflection/diffraction path visualizer.
- Impulse-response waveform, frequency, decay, and RT60 views.
- Runtime voice, latency, deadline, and simulation profiling.
- Acoustic bake manager with scene-revision invalidation.

The graph explorer should show Orcha's module relationships, optional solvers,
capability providers, and validation state.
