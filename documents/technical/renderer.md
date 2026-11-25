# ngen renderer: technical choices

Design: [../design/renderer.md](../design/renderer.md)

Each choice records the option selected and why the others were not, where a future session
might plausibly propose them again.

## Synchronisation: a render graph

**Selected:** passes declare what they read and write, with load and store intent; the render
graph orders them and inserts barriers. On Vulkan 1.1 a pass maps to a `VkRenderPass`; where
dynamic rendering is available it is used instead. Transient memory aliasing is left out at first.

- **Explicit passes with hand-written barriers:** deemed unacceptable because barriers are the most
  error-prone part of Vulkan, and with a 1.1 path and a newer path they would be written twice.
- Declared load and store intent also lets tile-based mobile GPUs keep attachments in on-chip
  memory, which hand-written passes tend to get wrong.

## Lighting: forward first, deferred second, one material model

**Selected:** both paths share one material model (a Slang function producing a surface). Forward
is built first; deferred is a second set of passes added later. The path is chosen in the
renderer configuration.

- **Deferred only:** deemed unacceptable because G-buffer bandwidth is costly on tile-based mobile
  GPUs unless the G-buffer stays in tile memory, and transparency and MSAA are awkward. Keeping
  the G-buffer in tile memory on mobile is later work, once deferred exists.
- **Separate materials per path:** deemed unacceptable because every material would be written
  twice.
- **Switching the path while running:** not wanted; a configuration choice keeps pipelines and
  render targets fixed for the renderer's lifetime.

## Shader binding: descriptor sets by update frequency

**Selected:** fixed set slots by how often they change: per frame, per pass, per material, and per
draw (push constants). It maps cleanly to D3D12 root signatures.

- **Bindless only:** deemed unacceptable because many Vulkan 1.1 Android devices lack descriptor
  indexing, so a fallback would be needed anyway. Bindless may come later as an optional path.

## Resources: handles owned by the device

**Selected:** typed handles with generation counters; the device owns the objects and destroys
them once every frame in flight that uses them has finished.

- **RAII objects (as in the original wrappers):** deemed unacceptable because destroying a resource
  the GPU still uses becomes easy, so a deferred-delete queue is needed anyway, and objects are
  harder to pass across a backend-neutral API.

## GPU memory: VMA

**Selected:** the Vulkan Memory Allocator (VMA) inside the Vulkan backend.

- **One `vkAllocateMemory` per resource (as in the original `Buffer`):** deemed unacceptable because
  drivers limit the number of allocations, and sub-allocation, alignment and memory type choice
  are what VMA already does well.

## Shading language: Slang

**Selected:** Slang, compiled by `slangc` from the Vulkan SDK to SPIR-V 1.3 (the oldest SPIR-V Slang
produces, and what Vulkan 1.1 accepts).

- **GLSL (the original shaders):** deemed unacceptable because it does not compile to DXIL, which a
  D3D12 backend needs.
- **HLSL with DXC:** a sound alternative, rejected because it has no modules or generics, and
  Metal would need a separate cross-compile step. Slang's interfaces also suit the shared
  material model.

## Testing: GoogleTest with GoogleMock

**Selected:** GoogleTest, with GoogleMock for the fake RHI backend where a mock reads more clearly
than a hand-written fake.

- **Catch2 v3 and doctest:** both sound, rejected because neither is enough of an improvement to
  outweigh the author's existing familiarity with GoogleTest, and GoogleTest has the widest IDE
  and CMake support.
