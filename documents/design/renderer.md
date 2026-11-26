# ngen renderer

Technical choices: [../technical/renderer.md](../technical/renderer.md)

## What it is

ngen is rendering infrastructure for smaller games that don't need a full engine such as Unreal.
It is part of the `ngen` package (a play on *n!*, nfactorial, and *engine*), built for fun and
experiment, and owned and designed by its author.

- **Platforms:** Windows and macOS desktops, Android and iOS.
- **Graphics API:** Vulkan only; Apple platforms run it through MoltenVK. Nothing may rule out
  adding a D3D12 backend later (e.g. for Xbox).
- **Shaders:** written in Slang, so the same source can compile to SPIR-V now and to DXIL for
  D3D12 later.
- **Minimum Vulkan version:** 1.1, the Android baseline. Newer features (dynamic rendering,
  synchronization2, descriptor indexing) are used where the device has them, with a 1.1 path
  behind each.

## What it is not

The renderer draws what a game tells it to. It holds no game state.

- **Scene management** is a separate module (`ngen_scene`), added later in this repository.
- **Asset management** (loading files, compiling shaders) is a separate, self-contained module
  (`ngen_assets`), designed and built later. The renderer accepts only in-memory data: pixels,
  vertices and compiled SPIR-V.
- **Dependency rule:** modules above the renderer depend on it, never the reverse.
- A game, or the example application, ties the modules together.

## Layers and modules

Each module is an `ngen_` library with a matching `ngen::` CMake target.

**The modules are reusable on their own.** They form a cohesive set someone can pick from to
build their own engine: the RHI and the renderer pieces, say, can be integrated into another
engine with nothing else. A default ngen 3D pipeline may configure the modules together, but
someone can assemble their own.

- The lower a module's layer, the fewer dependencies it has. Higher layers depend on lower ones
  by necessity (a scene module is tightly coupled to the rendering API).
- A module takes no dependency a user might not want, such as SDL, unless it is the module's
  purpose; an optional implementation lives in a target of its own.
- Where two modules genuinely can't be disentangled, coupling them is fine.

| Module | Target | Holds |
|---|---|---|
| `ngen_platform` | `ngen::platform` | The surface and lifecycle interface, with an SDL implementation. |
| `ngen_rhi` | `ngen::rhi` | The backend-neutral rendering hardware interface. |
| `ngen_rhi_vulkan` | `ngen::rhi_vulkan` | The Vulkan backend of the RHI. |
| `ngen_renderer` | `ngen::renderer` | Stages, render graph, render list, materials, sprite and mesh paths. |
| `ngen_scene` (later) | `ngen::scene` | Scene management, built on the renderer. |
| `ngen_assets` (later) | `ngen::assets` | Asset management. |

- **Platform** owns nothing a game must use: games may still use SDL directly, e.g. for input.
- **RHI:** no Vulkan type is visible above it. The Vulkan backend is its only real
  implementation; tests use a fake backend.
- **Renderer** is what games use. It talks to the GPU only through the RHI.

## What the renderer draws

- **2D sprites and 3D meshes** from the start, sharing stages, passes and materials.
- **Lighting:** forward lighting first; deferred lighting later. Transparent objects use forward
  lighting on both paths.

## Foundations

### Interfaces

- **Platform surface:** creates a presentable surface for a window, and reports to the renderer
  when the surface's size changes and when the application is suspended or resumed (on mobile
  the surface is destroyed while suspended).
- **RHI device:** creates and destroys resources (buffers, textures, samplers, shaders,
  pipelines) by handle, records commands into command lists, submits them and presents. A
  destroyed resource stays alive until the GPU has finished every frame that uses it.
- **Render list:** each frame the game fills a list of cameras and draw items and hands it to the
  renderer, which sorts, batches and records them. The list holds the frame's whole request; the
  renderer keeps no game state between frames.

### Data models

- **Handle:** a small typed value naming an RHI resource (e.g. a buffer handle), holding an index
  and a generation. Copying it is free; using a handle whose resource was destroyed is detected
  rather than undefined.
- **Draw item:** what to draw (a mesh or a sprite), its material, its transform, and the stage it
  belongs to.
- **Renderer configuration:** the lighting path and the list of stages. It is fixed once the
  renderer is created.

### Terms

- **Stage:** an ordered group of render graph passes within a frame, optionally with its own
  camera or render target. The renderer provides a default set (Shadow, Opaque, Transparent,
  Post-process, UI), and a game can add, remove or reorder stages in the renderer
  configuration. Stages never change while the renderer is running; a stage with no draw items
  in a frame records no passes and costs nothing, so a game's one layout also serves screens that
  use only some of its stages (a front-end menu drawing only UI). Every draw item names one.
- **Render graph:** the frame's passes, each declaring the attachments and resources it reads
  and writes, with how each attachment is loaded and stored. The renderer orders the passes and
  inserts every synchronisation barrier; nothing above the render graph writes barriers.
- **Lighting path:** forward or deferred, chosen in the renderer configuration (a project flag,
  or a setting when the renderer is created). It is never switched while running.
- **Material:** a Slang function that computes a surface (albedo, normal, roughness and the
  like) for a point on an object. Both lighting paths light the same surface, so one material
  works on either. A material that does its own lighting works on the forward path only.

## Cross-cutting

- **Language:** C++20.
- **Threading:** one thread records and submits for now. Nothing in the API relies on global
  state, so command recording can later be spread across threads (one command pool per thread).
- **Errors:** functions that can fail return a `[[nodiscard]]` result holding either the value or
  an error code with a message. No exceptions.
- **Diagnostics:** when everything is fine, ngen prints nothing. Validation warnings and errors
  and failures are reported.
- **Testing:**
  - Renderer logic (render graph ordering and barriers, sorting, stage order) has unit tests
    against a fake RHI backend, written with GoogleTest and GoogleMock.
  - A small set of smoke tests runs on real Vulkan with validation on; any validation message
    fails the test.
  - The example application is the visual check.
