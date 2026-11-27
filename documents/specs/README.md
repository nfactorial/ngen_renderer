# ngen

> - **Template:** v1
> - **Area:** the ngen rendering package: platform layer, RHI, Vulkan backend and renderer
> - **Design:** [renderer](../design/renderer.md) · **Technical:** [renderer](../technical/renderer.md)

ngen is rendering infrastructure for smaller games, layered as platform, RHI (with its Vulkan
backend) and renderer.

## Specs

- [Platform](platform/platform.md): the window interface (native handles, pixel size, resize and suspend events), its SDL3 implementation, and `ngen::Error`.
- [RHI](graphics/rhi.md): the backend-neutral device, handles, resource descriptions, command lists and the fake backend for tests.
- [RHI Vulkan backend](graphics/rhi_vulkan.md): the Vulkan implementation of the RHI.
- [Renderer](graphics/renderer/README.md): the renderer games draw through, with its stages, render list and render graph.
- [Example application](examples/example.md): the triangle drawn through ngen, the visual check.

## Not yet planned

### Milestone: Render graph and stages

Check: run the example: the triangle draws through the renderer; the unit tests show passes ordered by stage and barriers inferred from declared reads and writes.

- Test-only barrier target (change to `rhi`): a test-only alternative in `BarrierTarget` that `FakeDevice` accepts and shaders may read, so the render graph's read rules (reads in `ShaderRead`, no barrier for a read after a read, colour target before reads) get tests.


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
- SDL platform target (change to `platform`): move `SdlWindow` into its own target that links SDL3 publicly, so `ngen::platform` and the modules on it (`ngen_rhi_vulkan`) carry no SDL3 dependency, and code including `sdl_window.h` need not link SDL3 itself.
- Shared misuse checks (change to `rhi` and `rhi-vulkan`): one call-order and handle checker in `ngen_rhi` used by both `FakeCommandList` and the Vulkan command list, adopting the Vulkan backend's extra misuse rules (negative scissor offset, vertex binding past the device limit, draw with no pipeline, draw with a binding the pipeline reads unbound) in the RHI contract and `FakeDevice`.
- `FakeDevice` spec (new spec, split from `rhi`): move `FakeDevice` and its recorded commands into a spec of their own, bringing `rhi`'s body back under the size guide.
- `HandlePool` clear or iteration (change to `rhi`): let a pool be cleared or iterated, so a backend can free its live resources without resetting the pool.
- Event pump helper (change to `platform`): a shared helper that runs the SDL wait/poll loop and feeds `SdlWindow::handleEvent`, so each application need not own that loop.
- Live resize redraw (change to `example`): keep drawing the triangle while the window is being dragged to a new size, not only once the drag ends.
- `FakeDevice` frame errors (change to `rhi`): an opt-in way to make `FakeDevice`'s `beginFrame` or `endFrame` fail, so code built on the RHI can test that their errors are passed through.
