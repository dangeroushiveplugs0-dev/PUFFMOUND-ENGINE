# Engine and Plugin Package Architecture

Status: design contract for the early development stage. This document describes the intended package boundary; it does not claim that package installation or dynamic loading is implemented yet.

For the detailed native-code compatibility requirements, base-engine extension points, validation rules, and incremental implementation order, see [Native Plugin Compatibility Rules](NATIVE_PLUGIN_COMPATIBILITY.md).

## 1. Two separate extension types

The modeling and animation application is the host. It uses one compatible engine package and may load zero or more compatible plugin packages.

- **Engine package**: required. Supplies the runtime services the host needs, beginning with rendering and core engine services.
- **Plugin package**: optional. Adds a capability through the host's plugin interface. Physics is a plugin, not a second required "physics edition" of the engine.
- **Android test APK**: a temporary development harness used to test engine code on a real device. It is not the final modeling/animation application and is not the engine deliverable.

The host should report a clear error if no compatible engine is installed. It should not require the physics plugin, and it should continue to work when optional plugins are absent.

## 2. Package files and metadata

RAR is the intended user-facing distribution container. An archive is only a container: its contents, manifest, compatibility checks, and loader contract define what the package actually does.

Use a manifest at the package root. Suggested engine manifest fields:

- `package_type`: `engine`
- `id`: stable machine-readable identifier, e.g. `puffmound-engine`
- `display_name`: human-readable name
- `package_version`: package release version
- `engine_api_version`: interface version implemented by this engine
- `platform`: `android`
- `abi`: supported Android CPU ABI, initially selected explicitly (for example, `arm64-v8a`)
- `entrypoint`: native or other supported runtime entry point
- `files`: optional file list with hashes for integrity validation

Suggested plugin manifest fields:

- `package_type`: `plugin`
- `id`, `display_name`, and `package_version`
- `plugin_api_version`: host plugin-interface version
- `engine_api_min` and `engine_api_max`: compatible engine API range
- `dependencies`: other required plugin IDs and compatible version ranges, if any
- `entrypoint`: plugin entry point
- `files`: optional file list with hashes for integrity validation

The final schema should be formalized in code and validated strictly before a package can be activated.

## 3. Suggested archive layout

An engine archive may contain:

```text
PUFFMOUND-ENGINE.rar
  engine.json
  native/
    arm64-v8a/
      libpuffmound_engine.so
  shaders/
  resources/
  licenses/
```

A physics plugin archive may contain:

```text
PhysicsPlugin.rar
  plugin.json
  native/
    arm64-v8a/
      libphysics_plugin.so
  resources/
  licenses/
```

These names are a proposed layout, not a claim that these files already exist. The loader must reject unsupported ABIs or missing entry points rather than guessing.

## 4. Stable interface and loading rules

- Define a small, versioned C-compatible ABI at the engine/plugin boundary. Do not make plugins depend on compiler-specific C++ class layouts or an unversioned internal header.
- The host owns the application lifecycle, user interface, document state, and plugin activation order.
- The engine exposes documented services through the agreed interface; plugins receive only the services they are allowed to use.
- Validate archive paths, manifest fields, file hashes where provided, API compatibility, ABI compatibility, and dependencies before activation.
- Treat third-party packages as untrusted. Do not execute an entry point merely because an archive extracted successfully.
- Install or stage packages in app-controlled storage. Do not load executable code directly from an arbitrary public/shared-storage path. Follow Android's current native-code loading and file-permission requirements.
- Keep the currently working engine/plugin set until a replacement has passed validation. If installation or startup fails, retain or restore the last known-good set.
- Show a useful error message and diagnostic log for corrupt archives, incompatible versions, missing dependencies, or failed initialization.

The precise native-loading mechanism must be verified against the Android API levels and target SDK supported by the app before implementation.

## 5. Physics plugin and animation baking

The physics plugin should register simulation capabilities with the host rather than being hard-wired into the required engine package.

A typical workflow:

1. The artist enables a compatible physics plugin.
2. The artist assigns physics behavior to supported scene objects, rigs, or mesh components.
3. The plugin simulates the scene using the host's time step and scene data.
4. On **Bake**, the plugin evaluates the selected frame range and writes the resulting motion into a persistent animation representation: for example, transform keyframes, rig controls, or a documented simulation cache for supported deforming meshes.
5. Playback uses the baked result without needing to re-simulate every frame. The project may retain the source settings so the simulation can be rebaked after changes.

Physics does not magically become ordinary keyframes without a bake step. The plugin must define which outputs it can bake, the cache format, how changes invalidate the bake, and how the host saves/reloads that data. Some high-detail mesh simulations may be better stored as caches than as transform keyframes.

## 6. Incremental implementation order

1. Document the engine API boundary and package manifest schema.
2. Extract the current demo renderer behind a small engine-facing interface while keeping the existing APK test harness working.
3. Add portable tests for manifest validation and API-version compatibility.
4. Add safe archive inspection and installation into app-controlled storage.
5. Load the engine through the chosen Android-compatible entry point and show clear startup diagnostics.
6. Add the plugin registry and a harmless example plugin before implementing physics.
7. Add physics simulation and baking only after the plugin lifecycle and persisted animation format are tested.

## 7. Scope boundary

The immediate project is **PUFFMOUND-ENGINE**, a reusable engine package. The separate modeling/animation application is the host, and physics is one optional plugin among many. Do not create a separate required physics-engine package or treat the temporary test APK as the final product.
