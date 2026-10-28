# Spank3d

**Spank3d** is a mobile-first, 3D-first rendering engine in active development.

## Direction
- **C++**: performance-critical engine, rendering, math, and resource systems.
- **OpenGL ES 3.0 first**: practical baseline for Android/mobile GPUs; keep the renderer backend modular so Vulkan can be added later.
- **C#**: planned editor/tooling and higher-level workflows, not a dependency of the native runtime.
- **Python**: build helpers, asset processing, validation, and developer automation.
- **Mobile-first**: memory budgets, GPU capability checks, lifecycle/context loss, and touch-friendly tools are design constraints from day one.

## Material and lighting strategy
- **MatCap is the default viewport material mode.** It provides readable sculpting/modeling shading without requiring a scene-light setup, with swappable MatCap presets.
- **PBR is the physically based material path.** Materials will expose base color, metallic, roughness, and optional normal/ambient-occlusion inputs.
- **Environment maps and reflection probes** provide environment reflections and image-based lighting (IBL) for PBR materials. Probe placement, blending, and update cost must be considered for mobile GPUs.
- **Dynamic direct lighting is a separate system.** Scene lights contribute direct illumination; probes/environment maps provide environment lighting and reflections. MatCap remains a viewport-shading mode rather than pretending to be physically lit PBR.

## Current milestone: first draw call
The Android OpenGL ES backend now has an end-to-end debug-triangle path:
- Compile and link a GLSL ES 3.00 vertex/fragment shader program.
- Upload interleaved position/color vertex data into a VBO and configure a VAO.
- Draw the triangle through `GlesRenderer::draw_debug_triangle()`.
- Call `GlesRenderer::shutdown()` while the EGL context used to create the resources is still current.

The host application is responsible for creating/making current the EGL context, calling `initialize()`, setting the surface size through `resize()`, and performing the EGL buffer swap after `end_frame()`. This repository still does **not** include the Android host or a packaged APK, so a visible on-device demo is not claimed yet.

## Next rendering milestones
1. Add an Android host/surface lifecycle and verify the triangle on a real device.
2. Add camera transforms and depth-tested 3D mesh drawing.
3. Implement MatCap sampling as the default viewport material mode.
4. Add PBR material inputs, direct dynamic lights, and environment-map/reflection-probe IBL incrementally.

## Build core and test
Portable core:
```sh
cmake -S . -B build -DSPANK3D_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Compile-check the GLES backend on a Linux machine with EGL/GLES development libraries installed:
```sh
cmake -S . -B build-gles -DSPANK3D_BUILD_TESTS=ON -DSPANK3D_BUILD_GLES_BACKEND=ON
cmake --build build-gles
ctest --test-dir build-gles --output-on-failure
```

## License
Apache-2.0. See [LICENSE](LICENSE).
