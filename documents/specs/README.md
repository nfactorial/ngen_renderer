# ngen

> - **Template:** v1
> - **Area:** the ngen rendering package: platform layer, RHI, Vulkan backend and renderer
> - **Design:** [renderer](../design/renderer.md) · **Technical:** [renderer](../technical/renderer.md)

ngen is rendering infrastructure for smaller games, layered as platform, RHI (with its Vulkan
backend) and renderer.

## Specs

## Not yet planned

### Milestone: Triangle on the RHI

Check: run the example: the vertex-coloured triangle draws; resizing, minimising and restoring the window keep it drawing with no validation messages; the unit tests pass.

- `ngen_platform` (new spec): the surface and lifecycle interface (create a surface, report size changes, suspend and resume), with an SDL implementation.
- `ngen_rhi` (new spec): the backend-neutral device, typed generational handles, resource descriptions, command lists, submit and present, with a fake backend for tests and the GoogleTest setup.
- `ngen_rhi_vulkan` (new spec): the Vulkan backend of `ngen_rhi`, ported from `ngen_vulkan`, with VMA and swap chain recreation on resize and resume.
- Example on the RHI (new spec for the example): the example draws its triangle through `ngen_rhi` and `ngen_platform`, and `ngen_vulkan` is deleted.

### Milestone: Render graph and stages

Check: run the example: the triangle draws through the renderer; the unit tests show passes ordered by stage and barriers inferred from declared reads and writes.

- `ngen_renderer` (new spec): renderer configuration with stages, the render graph inferring barriers from declared reads and writes, and the per-frame render list.
- Example on the renderer (change to the example's spec): the example submits its triangle as a draw item.

### Milestone: Android triangle

Check: install the example on an Android phone: the triangle draws, and survives backgrounding, foregrounding and rotation with no validation messages.

- Android build (change to `ngen_platform`): the SDL implementation handles Android surface loss, restoration and rotation (pre-transform).
- Android packaging (change to the example's spec): a Gradle project building the example as an APK, with its compiled shaders in the package.

### Milestone: Sprites

Check: run the example: textured sprites draw in layered order, batched, alongside the triangle.

- Textures and uploads (change to `ngen_rhi` and `ngen_rhi_vulkan`): textures, samplers and device-local uploads.
- Descriptor sets (change to `ngen_rhi` and `ngen_rhi_vulkan`): set slots by update frequency (frame, pass, material, draw).
- Sprite path (change to `ngen_renderer`): batched sprite draw items.

### Milestone: Lit meshes (forward)

Check: run the example: a rotating, lit 3D mesh draws with depth beside the sprites.

- Meshes and cameras (change to `ngen_renderer`): mesh draw items, cameras and depth.
- Materials and forward lighting (change to `ngen_renderer`): the shared Slang surface model and the forward lighting path.

### Later

- Deferred lighting (change to `ngen_renderer`): the deferred path behind the same material model.
- On-tile deferred for mobile (change to `ngen_renderer`): keep the G-buffer in tile memory on tile-based GPUs.
- iOS build (change to `ngen_platform` and the example's spec): the example on iOS through MoltenVK.
- Bindless (change to `ngen_rhi` and `ngen_rhi_vulkan`): an optional bindless binding path where descriptor indexing is available.
- Parallel recording (change to `ngen_rhi` and `ngen_renderer`): record command lists on several threads.
- `ngen_scene` (new spec): scene management built on the renderer.
- `ngen_assets` (new spec): asset management.
