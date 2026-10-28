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

## First milestone: rendering foundation
1. Build and test the portable C++ core.
2. Establish renderer/resource interfaces and an OpenGL ES backend for Android.
3. Render a triangle, then a 3D mesh with camera transforms and depth testing.
4. Add the MatCap shader and selectable MatCap presets as the first viewport material mode.
5. Add the PBR shader/material inputs, direct dynamic lights, and environment-map/reflection-probe IBL incrementally.
6. Add Android app integration and repeatable CI artifacts after the native render path is validated.

The current branch is the **foundation**, not a feature-complete engine or a working visual demo yet. The GLES backend is compiled only in Android builds; portable core tests run in ordinary CI.

## Build core
```sh
cmake -S . -B build -DSPANK3D_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## License
Apache-2.0. See [LICENSE](LICENSE).
