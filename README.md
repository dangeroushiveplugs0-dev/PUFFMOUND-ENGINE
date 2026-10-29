# PUFFMOUND-ENGINE

**PUFFMOUND-ENGINE** is a mobile-first, 3D-first rendering engine in active development.

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

## Current milestone: touch-controlled modeling camera
The Android host uses `GLSurfaceView` with an OpenGL ES 3 context and calls the native C++ renderer through JNI. The renderer now:
- Compiles and links GLSL ES 3.00 shaders with an MVP matrix uniform.
- Uploads a 36-vertex cube mesh into a VBO and configures a VAO.
- Uses a controllable orbit camera with clamped pitch and distance, pan, pinch zoom, and perspective/orthographic projection modes.
- Keeps a reset-camera action available and uses the animated cube as a repeatable framing target.
- Applies model rotation and depth testing.
- Animates the cube using elapsed time passed from the Android host.
- Reports shader compilation/link errors to stderr.
- Expects the host to create and own the EGL context and perform the buffer swap.

The APK builds in GitHub Actions and is uploaded as the `spank3d-debug-apk` artifact. This is a rendering test app, not yet a 3D editor. The cube change needs to pass CI and be checked on the phone before it is considered verified.

## Next rendering milestones
1. Verify orbit, pan, pinch zoom, projection toggle, and reset on a real Android device; then harden lifecycle/context recreation.
2. Replace per-face debug colors with the default MatCap viewport material.
3. Extract reusable mesh, camera, and material interfaces instead of keeping the demo mesh inside the GLES backend.
4. Add PBR material inputs, dynamic direct lights, and environment-map/reflection-probe IBL incrementally.

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
