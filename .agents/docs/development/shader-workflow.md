# Shader workflow

Shaders are backend inputs and must be treated as versioned interfaces. Keep source, entry-point names, resource bindings, coordinate conventions, precision assumptions, and supported feature levels documented with the owning render module. Generated binaries/intermediates stay in ignored build directories.

Before submitting a shader change:

```sh
python scripts/moonlight.py validate
python python/build.py --skip managed --skip rust
```

The repository currently has no dedicated shader compiler command in `python/build.py`; use the backend/compiler command required by the target platform and record it in the module README. A missing compiler or unavailable backend is an infrastructure/coverage result, not a passing shader test. Do not check in machine-specific caches.

Validate compilation for every supported backend, reflection/binding compatibility, and a deterministic image or numerical fixture. Test bounds checks, invalid resources, precision-sensitive inputs, and fallback paths. Compare output to the reference renderer with a documented tolerance; explain intentional approximations.

Linux commonly uses Vulkan/GL toolchains; macOS may require Metal tooling from Xcode; Windows may use the DirectX shader compiler/SDK. Keep platform-specific source isolated and make unsupported backends report a clear capability error. Never execute untrusted shader source during tests without sandboxing. Ownership includes resource lifetime, descriptor layouts, synchronization, and cleanup on compilation failure.
