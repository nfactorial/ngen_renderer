# ngen

> - **Template:** v1
> - **Area:** the ngen rendering package: platform layer, RHI, Vulkan backend and renderer
> - **Design:** [renderer](../design/renderer.md) · **Technical:** [renderer](../technical/renderer.md)

ngen is rendering infrastructure for smaller games, layered as platform, RHI (with its Vulkan
backend) and renderer.

## Specs

- [Platform](platform.md): the window interface (native handles, pixel size, resize and suspend events), its SDL3 implementation, and `ngen::Error`.
- [RHI](rhi.md): the backend-neutral device, handles, resource descriptions, command lists and the fake backend for tests.
- [RHI Vulkan backend](rhi_vulkan.md): the Vulkan implementation of the RHI.
- [Example application](example.md): the triangle drawn through ngen, the visual check.

## Not yet planned

### Milestone: Triangle on the RHI

Check: run the example: the vertex-coloured triangle draws; resizing, minimising and restoring the window keep it drawing with no validation messages; the unit tests pass.

### Milestone: Render graph and stages

Check: run the example: the triangle draws through the renderer; the unit tests show passes ordered by stage and barriers inferred from declared reads and writes.

- `ngen_renderer` (new spec): renderer configuration with stages, the render graph inferring barriers from declared reads and writes, and the per-frame render list.
- Example on the renderer (change to `example`): the example submits its triangle as a draw item.

### Milestone: Android triangle

Check: install the example on an Android phone: the triangle draws, and survives backgrounding, foregrounding and rotation with no validation messages.

- Android build (change to `platform`): the SDL implementation handles Android surface loss, restoration and rotation (pre-transform).
- Android packaging (change to `example`): a Gradle project building the example as an APK, with its compiled shaders in the package.

### Milestone: Sprites

Check: run the example: textured sprites draw in layered order, batched, alongside the triangle.

- Textures and uploads (change to `rhi` and `rhi-vulkan`): textures, samplers and device-local uploads.
- Descriptor sets (change to `rhi` and `rhi-vulkan`): set slots by update frequency (frame, pass, material, draw).
- Sprite path (change to `ngen_renderer`): batched sprite draw items.

### Milestone: Lit meshes (forward)

Check: run the example: a rotating, lit 3D mesh draws with depth beside the sprites.

- Meshes and cameras (change to `ngen_renderer`): mesh draw items, cameras and depth.
- Materials and forward lighting (change to `ngen_renderer`): the shared Slang surface model and the forward lighting path.

### Later

- Deferred lighting (change to `ngen_renderer`): the deferred path behind the same material model.
- On-tile deferred for mobile (change to `ngen_renderer`): keep the G-buffer in tile memory on tile-based GPUs.
- Vulkan 1.3 path (change to `rhi-vulkan`): dynamic rendering and synchronization2 where the device has them.
- iOS build (change to `platform` and `example`): the example on iOS through MoltenVK.
- Bindless (change to `rhi` and `rhi-vulkan`): an optional bindless binding path where descriptor indexing is available.
- Parallel recording (change to `rhi` and `ngen_renderer`): record command lists on several threads.
- `ngen_scene` (new spec): scene management built on the renderer.
- `ngen_assets` (new spec): asset management.
