# Native Engine and Plugin Compatibility Rules

Status: design contract only. This document sets requirements for future implementation; it does not claim that package installation, engine swapping, native loading, or plugin execution already works.

## The product model

The modeling and animation app is the launcher and workspace. It requires one compatible engine package and can optionally install plugin packages.

- The engine is required, replaceable, and supplies the shared runtime foundation.
- Plugins are optional, independently replaceable capabilities. Physics is a plugin, not a separate required engine edition.
- The Android APK is a temporary device-test harness, not the final modeling app.
- Engine and plugin packages are distributed as RAR archives, but RAR is only a container; it is not executable code.

This is not a remote-service design. Native compiled code runs in the supported host process. The versioned API/ABI is the technical boundary that allows separately packaged native code to interoperate and remain replaceable.

## Native implementation

The performance-critical engine is native compiled C++, targeting explicitly supported platforms and CPU ABIs. On Android, implementation must follow the relevant target-SDK, native-library loading, storage, signing, and distribution requirements.

Do not load executable code directly from an archive or arbitrary shared-storage location. Inspect and safely extract packages into app-controlled storage first. Native plugins have substantial privileges inside the host process; manifest validation is not a sandbox, so package trust and explicit user consent matter.

## Required base-engine responsibilities

Plugin capability must be designed into the first engine, not bolted on later. The initial implementation should establish extension points for:

- Lifecycle: initialize, start, stop, orderly shutdown, error reporting, and cleanup after partial initialization.
- Version and capability negotiation: engine API version, native ABI version, supported capabilities, and compatibility ranges.
- Scene integration: object identity, transforms, hierarchy, mesh references, and safe scene mutation.
- Frame/time integration: frame callbacks, fixed-step simulation hooks, pause/resume, and deterministic time-step rules.
- Rendering integration: resource registration and documented ways for plugins to contribute geometry, materials, or render work.
- Resource ownership: explicit lifetime, thread-affinity, and release rules for engine-owned handles.
- Animation and persistence: supported keyframe channels, simulation-cache registration, serialization/versioning, and save/reload behavior.
- Diagnostics: structured errors and useful logs for package rejection, initialization failure, or missing capabilities.

These are requirements to implement incrementally, not a claim that every service already exists. Avoid exposing compiler-specific C++ object layouts as the stable plugin contract. Prefer a small, versioned C-compatible ABI with opaque handles and explicit function tables. Keep implementation-specific C++ classes private, and test the ABI before compiling real plugins against it.

The engine does not need to contain a physics solver. It must provide the scene, timing, resource, lifecycle, animation, and rendering integration that lets a compatible physics plugin simulate objects and make its results visible in the app.

## Package compatibility and validation

The engine manifest should declare its package ID/version, engine API version, native ABI version, platform, supported CPU ABIs, entry point, files/integrity metadata, and capabilities.

A plugin manifest should declare its package ID/version, plugin API version, required engine API range, native ABI/platform/CPU ABI, entry point, dependencies, capabilities provided and required, and files/integrity metadata.

Reject a package before activation if its platform or ABI is unsupported, API versions do not match, dependencies are missing, required files are absent, integrity checks fail, or the manifest is malformed. Safely reject path traversal, unsafe archive entries, and unreasonable extraction sizes. Never treat successful extraction as proof that code is safe or compatible.

Stage replacements separately. Preserve the known-good engine/plugin set until the replacement passes validation and startup checks. On failure, restore the previous working set and show a useful diagnostic. Verify the exact Android native-loading constraints against the target SDK before implementation.

## Example: physics plugin

1. The user installs a compatible PhysicsPlugin RAR archive.
2. The app validates the manifest and stages its native library and resources.
3. The engine loads the plugin through the approved native entry point and supplies its versioned service table.
4. The plugin registers simulation behavior and uses shared scene/time services.
5. The plugin contributes updated transforms or mesh data through documented engine hooks so the renderer can display the results.
6. When the user bakes the simulation, the plugin saves supported transform/rig keyframes or a versioned deformation cache.
7. Playback reads the baked result without re-running the simulation every frame.

The plugin must document bake output formats, invalidation rules, and save/reload behavior. Removing the plugin must not erase already baked data that the host can still read. Live simulation requires a compatible physics plugin.

The same design should accommodate future cloth, fluids, particles, procedural modeling, and other extensions without making them mandatory parts of the engine.

## Incremental implementation order

1. Preserve the current camera behavior and APK test harness while extracting reusable engine-facing renderer interfaces.
2. Specify the first ABI: lifecycle, version negotiation, ownership, thread rules, scene data, time, and rendering contributions.
3. Add portable tests for manifest validation, compatibility negotiation, and failure paths.
4. Add safe RAR inspection and extraction into app-controlled storage.
5. Verify Android-compatible native loading for the supported target SDK and OS versions.
6. Build a harmless example plugin and test registration, rendering contribution, shutdown, and failed initialization cleanup.
7. Prove the full plugin-to-scene-to-renderer path before implementing the physics plugin.
8. Implement physics baking only after the plugin lifecycle and persistent animation/cache formats are testable.

## Scope and status

PUFFMOUND-ENGINE is the required, replaceable native engine package. The separate modeling/animation app is the host and launcher. Plugins are optional and independently replaceable. Physics is the first planned plugin, not a separate required engine.

This document is a design contract. RAR installation, native engine swapping, plugin loading, and physics baking are not yet claimed as implemented or tested.
