# Renderer — Spec

> - **Template:** v1 · **ID:** renderer
> - **Status:** built
> - **Blocked by:** [Render graph](render_graph/render_graph.md), [RHI](../rhi.md)
> - **Background (not needed to build):** Feature: [Renderer](README.md) · Design: [renderer](../../../design/renderer.md) · Technical: [renderer](../../../technical/renderer.md) · Inputs: none

## Goal

The renderer is what a game draws through. It is created once with a configuration: the
lighting path and the ordered stages. Each frame it takes a render list of draw items,
builds that frame's render graph (one pass for each stage that has draw items), records it
and presents. It keeps no game state between frames.

## Scope

**In:**
- `RendererConfiguration`, its stages and the default stage set.
- `DrawItem` and `RenderList`.
- `Renderer`: creation, and rendering a frame on an RHI `Device`.

**Out (non-goals):**
- Meshes, sprites, materials, cameras and transforms: a draw item in this spec names a
  pipeline and vertex buffers directly.
- Sorting or batching beyond keeping list order within a stage.
- A stage's own camera or render target: every stage draws to the swap chain image.
- Deferred lighting.
- Forwarding window events: the application still forwards them to the device.

## Constraints and key decisions

- Every public type is in namespace `ngen::renderer`, in the `ngen_renderer` target beside the
  render graph. It reaches the GPU only through `ngen::rhi::Device`, `CommandList` and
  `RenderGraph`.
- The configuration is fixed once the renderer is created.
- The renderer uses the device it was created with and does not own it. It must be destroyed
  before that device.
- A stage with no draw items in a frame adds no pass.
- The renderer writes no barrier and chooses no load op: its render graph does both.

## Modules and interfaces

### Configuration  (ngen_renderer/include/ngen/renderer/configuration.h)

The fixed description of a renderer: its lighting path and its stages, in order.

**Interface:**
- `enum class LightingPath { Forward }`
- `struct StageDescription { std::string name; }`
- `std::vector<StageDescription> defaultStages()`: Shadow, Opaque, Transparent, PostProcess
  and UI, in that order.
- `struct RendererConfiguration { LightingPath lighting = LightingPath::Forward; std::vector<StageDescription> stages = defaultStages(); std::array<float, 4> clearColor = {0, 0, 0, 1}; }`
- `std::expected<void, ngen::Error> validate(const RendererConfiguration&)`

**Requirements:**
- There is at least one stage. Stage names are non-empty and unique. A breach returns
  `InvalidArgument` naming the stage, or saying there are none.

### RenderList  (ngen_renderer/include/ngen/renderer/render_list.h)

One frame's request: the draw items, in the order the game added them.

**Interface:**
- `struct StageId { uint32_t index; }`, comparable with `==`.
- `struct VertexBufferBinding { uint32_t binding; rhi::BufferHandle buffer; uint64_t offset = 0; }`
- `struct DrawItem { StageId stage; rhi::PipelineHandle pipeline; std::vector<VertexBufferBinding> vertexBuffers; uint32_t vertexCount; uint32_t instanceCount = 1; uint32_t firstVertex = 0; uint32_t firstInstance = 0; }`
- `class RenderList`: `void add(DrawItem)`, `void clear()`,
  `std::span<const DrawItem> items() const`.

### Renderer  (ngen_renderer/include/ngen/renderer/renderer.h)

Turns a render list into a frame on the device: one render graph pass per stage that has
items, in stage order.

**Interface:**
- `enum class FrameResult { Presented, Skipped }`
- `class Renderer`:
  - `static std::expected<std::unique_ptr<Renderer>, ngen::Error> create(rhi::Device&, RendererConfiguration)`:
    validates the configuration first.
  - `std::optional<StageId> stage(std::string_view name) const`: the stage of that name.
  - `std::expected<FrameResult, ngen::Error> render(const RenderList&)`
  - `const RendererConfiguration &configuration() const`

**Requirements:**
- `render` first checks every item: its `StageId` names a configured stage and its
  `vertexCount` is above 0. A breach returns `InvalidArgument` naming the item's index in the
  list, and begins no frame.
- `render` then calls `beginFrame`. When it returns no command list, `render` returns
  `Skipped`, records nothing and does not call `endFrame`. An error from `beginFrame` or
  `endFrame` is returned as it is.
- The frame's graph imports the swap chain image as `Undefined`. Each stage with items adds one
  pass named after the stage, with `order` equal to the stage's index, writing the swap chain
  image. The first such pass in stage order clears to `clearColor`; the others take what the
  graph decides. A list with no items adds one pass named "Clear", clearing to `clearColor`
  and drawing nothing, so every presented frame is cleared.
- Within a pass, items are drawn in list order. For each item the renderer binds its pipeline
  (skipping the bind when it is the pipeline already bound in that pass), binds each of its
  vertex buffers, then calls `draw(vertexCount, instanceCount, firstVertex, firstInstance)`.
- After recording, `render` calls `endFrame` and returns `Presented`. When executing the graph
  fails, `render` still calls `endFrame`, so the device is never left recording, and returns
  the execution error.

**Acceptance (against `FakeDevice` with a window attached):**
- `stage("Opaque")` on the default configuration is `StageId{1}`; `stage("Missing")` is empty.
- A list holding a UI item (pipeline p, buffer b at binding 0, 3 vertices), then an Opaque item
  (pipeline q, buffer c, 6 vertices), records one frame:
  `Barrier{SwapChainImage, Undefined, ColorAttachment}`, `beginRendering(Clear (0, 0, 0, 1))`,
  `bindPipeline(q)`, `bindVertexBuffer(0, c)`, `draw(6)`, `endRendering`,
  `Barrier{SwapChainImage, ColorAttachment, ColorAttachment}`, `beginRendering(Load)`,
  `bindPipeline(p)`, `bindVertexBuffer(0, b)`, `draw(3)`, `endRendering`. It returns
  `Presented`.
- An empty list records `Barrier{SwapChainImage, Undefined, ColorAttachment}`,
  `beginRendering(Clear)` and `endRendering`.

