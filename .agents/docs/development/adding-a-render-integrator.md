# Adding a render integrator

A render integrator consumes immutable extracted scene data and produces a frame through a declared backend. Begin with a design note covering supported features, coordinate/colour spaces, units, sample strategy, numerical assumptions, resource ownership, synchronization, and fallback behavior.

## Sequence

1. Add a module and manifest dependency on core scene/extraction contracts.
2. Define the integrator interface and capability query; reject unsupported features explicitly.
3. Implement resource creation, update, render, and destruction with clear thread ownership.
4. Add a deterministic reference-scene test and an image/numerical tolerance.
5. Exercise resize, empty scene, missing resource, device loss, and shutdown paths.
6. Validate/build/test on all supported platforms.

```sh
python scripts/moonlight.py validate
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

The render loop must not mutate Atlas or retain pointers into transient extraction buffers. GPU resources are owned by the integrator/device layer, released in reverse dependency order, and synchronized before destruction. Avoid blocking or unbounded allocation on frame-critical threads. Log capability failures without leaking scene/user data.

Approximation is acceptable only when documented and compared with the reference renderer under a stated tolerance. Record GPU/driver, resolution, seed, and capture settings. Add a performance budget and regression fixture; owner approval is required for public API, shader binding, frame-graph, or serialization changes.
