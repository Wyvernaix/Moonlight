# Moonlight Editor

The Editor is the first native application host for Moonlight Core. It owns
the desktop window and Dear ImGui presentation layer; engine state and module
APIs remain in Core and `modules/`.

The current host creates an SDL3 Vulkan window and renders Atlas scene data
through Spectral's Vulkan backend. The sample scene contains an Atlas-owned
indexed sphere mesh description, and Spectral creates the Vulkan swapchain,
pipeline, buffers, and shader modules needed to draw it. The sphere is shown
through the ImGui `Scene View` window, with separate `Hierarchy` and `Inspector`
windows available for additional Workbench tools. This is the first rendering
slice; advanced materials are added separately.

Build it with the repository preset:

```sh
cmake --preset workbench
cmake --build --preset workbench --target moonlight_workbench
build/workbench/bin/moonlight_workbench
```

The Workbench preset requires Vulkan headers, the Vulkan loader, SDL3, and
`glslc`. It automatically discovers the HFS Vulkan SDK at
`/opt/hfs21.0.700/toolkit` when present, so no extra CMake profile option is
needed on the development machine. The default Debug and Release presets
remain headless and do not compile the Vulkan backend.
