# Spank3d

**Spank3d** is a mobile-first, 3D-first rendering engine in active development.

## Direction
- **C++**: performance-critical engine, rendering, math, and resource systems.
- **OpenGL ES 3.0 first**: practical baseline for Android/mobile GPUs; keep the renderer backend modular so Vulkan can be added later.
- **C#**: planned editor/tooling and higher-level workflows, not a dependency of the native runtime.
- **Python**: build helpers, asset processing, validation, and developer automation.
- **Mobile-first**: memory budgets, GPU capability checks, lifecycle/context loss, and touch-friendly tools are design constraints from day one.

## First milestone: rendering foundation
1. Build and test the portable C++ core.
2. Establish renderer/resource interfaces and an OpenGL ES backend for Android.
3. Render a triangle, then a textured mesh with a camera and basic lighting.
4. Add Android app integration and repeatable CI artifacts after the native render path is validated.

The current branch is the **foundation**, not a feature-complete engine. The GLES backend is compiled only in Android builds; portable core tests run in ordinary CI.

## Build core
```sh
cmake -S . -B build -DSPANK3D_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## License
Apache-2.0. See [LICENSE](LICENSE).
