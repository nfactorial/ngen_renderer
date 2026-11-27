# RHI — Spec

> - **Template:** v1 · **ID:** rhi
> - **Status:** built
> - **Blocked by:** [Platform](../platform/platform.md)
> - **Background (not needed to build):** Feature: [ngen](../README.md) · Design: [renderer](../../design/renderer.md) · Technical: [renderer](../../technical/renderer.md) · Inputs: none

## Goal

`ngen_rhi` is ngen's rendering hardware interface: a backend-neutral API for creating GPU
resources, recording draw commands and presenting frames. Code above it sees only its own
types, never a graphics API's. A graphics backend implements its abstract `Device`; `ngen_rhi`
also provides `FakeDevice`, an implementation that records what it is asked to do, so code
built on the RHI can be tested without a GPU.

## Scope

**In:**
- Typed generational handles, and the handle pool that issues and checks them.
- Descriptions of buffers, shaders and graphics pipelines, and the value types they use.
- The abstract `Device`: resources, frames in flight, a swap chain for one window, and its
  surface events.
- The abstract `CommandList`: rendering to the swap chain image, binding, and drawing.
- `FakeDevice` and its recorded commands.
- The device's message sink, through which every backend reports diagnostics.
- Barriers on the swap chain image, and the resource states they move between.

**Out (non-goals):**
- Textures, samplers, descriptor sets, index buffers and compute (later milestones).
- Recording on more than one thread.
- Choosing which backend to create: the application constructs the backend it wants.

## Constraints and key decisions

- No graphics API type or header appears in `ngen_rhi`'s public headers.
- Every public type is in namespace `ngen::rhi`. Public headers live under
  `ngen_rhi/include/ngen/rhi/` and are included as `<ngen/rhi/…>`.
- The CMake target is `ngen_rhi`, with the alias `ngen::rhi`; it depends on `ngen::platform` for
  `ngen::Error`, `NativeWindow` and `PixelSize`.
- Functions that can fail return `std::expected<T, ngen::Error>` (or `std::expected<void, ngen::Error>`),
  marked `[[nodiscard]]`. No exceptions.
- All rendering happens on the thread that created the device. The API holds no global or
  static mutable state, so devices are independent of each other.
- The test target is `ngen_rhi_tests`.
- Up to `kMaxFramesInFlight = 2` frames are recorded or executing at once.
- A resource is destroyed only once no frame in flight uses it: `destroy` retires the handle at
  once and frees the resource after every frame begun before the call has finished. Destruction
  counts frames, not calls: a resource destroyed while no frame is in flight is freed by the next
  `beginFrame`.
- ngen prints nothing when all is well. Diagnostics (validation warnings and errors, misuse such
  as a stale handle) go to the device's message sink, which by default writes errors and
  warnings to `stderr`.

## Modules and interfaces

### Handles  (ngen_rhi/include/ngen/rhi/handle.h)

Small, freely copied values naming a resource, and the pool that issues them and tells a live
handle from a stale one.

**Interface:**
- `template <typename Tag> struct Handle { uint32_t index; uint32_t generation; }`, comparable
  with `==`; `bool isNull() const`. A default-constructed handle is null.
- `using BufferHandle = Handle<struct BufferTag>`, `ShaderHandle`, `PipelineHandle` likewise.
- `template <typename Tag, typename T> class HandlePool`:
  - `Handle<Tag> insert(T value)`
  - `T *get(Handle<Tag>)`: the value, or `nullptr` for a null, stale or out-of-range handle.
  - `std::optional<T> remove(Handle<Tag>)`: removes and returns the value; empty for a handle
    `get` would reject.
  - `size_t size() const`: live values.

**Requirements:**
- The null handle is `{0, 0}`; issued handles have a generation of 1 or more.
- Removing a value increments its slot's generation; a later `insert` may reuse the slot, and
  the earlier handle stays stale.

**Acceptance:**
- Insert A then B, remove A, insert C: C reuses A's index with a higher generation; `get` on A's
  handle returns `nullptr`; `get` on C returns C; `size()` is 2.
- `remove` called twice on one handle returns the value the first time and empty the second.

### Descriptions  (ngen_rhi/include/ngen/rhi/descriptions.h)

The plain values that say what to create.

**Interface:**
- `enum class BufferUsage : uint32_t { Vertex = 1, Uniform = 2 }`, a flag set with `|` and `&`
- `struct BufferDescription { uint64_t size; BufferUsage usage; }`: buffers are host-visible in
  this milestone.
- `enum class ShaderStage { Vertex, Fragment }`
- `struct ShaderDescription { std::span<const std::byte> bytecode; }`: bytecode in the format the
  backend consumes (SPIR-V for Vulkan); the span is copied by `createShader`.
- `enum class Format { R32G32Float, R32G32B32Float, R32G32B32A32Float }` (vertex attributes)
- `struct VertexAttribute { uint32_t location; uint32_t binding; Format format; uint32_t offset; }`
- `struct VertexBinding { uint32_t binding; uint32_t stride; bool perInstance = false; }`
- `struct ShaderEntry { ShaderHandle shader; std::string entryPoint; }`
- `enum class Topology { TriangleList, TriangleStrip, LineList }`, `enum class CullMode { None, Front, Back }`,
  `enum class FrontFace { Clockwise, CounterClockwise }`, `enum class PolygonMode { Fill, Line }`
- `struct GraphicsPipelineDescription { ShaderEntry vertex; ShaderEntry fragment; std::vector<VertexBinding> bindings; std::vector<VertexAttribute> attributes; Topology topology = TriangleList; CullMode cull = Back; FrontFace frontFace = Clockwise; PolygonMode polygonMode = Fill; }`:
  the pipeline renders to the swap chain's colour format; viewport and scissor are set while
  recording.
- `std::expected<void, ngen::Error> validate(const BufferDescription&)`, and likewise for
  `ShaderDescription` and `GraphicsPipelineDescription`: checks the requirements below, for every
  backend to share.
- `ColorTarget` compares with `==`.
- `enum class LoadOp { Load, Clear, DontCare }`, `enum class StoreOp { Store, DontCare }`
- `struct ColorTarget { LoadOp load = Clear; StoreOp store = Store; std::array<float, 4> clearColor = {0, 0, 0, 1}; }`
- `enum class ResourceState { Undefined, ColorAttachment, ShaderRead, Present }`: how a
  resource is used. `Undefined` means its contents may be discarded.
- `struct SwapChainImage {}`: names this frame's swap chain image, comparable with `==`.
- `using BarrierTarget = std::variant<SwapChainImage>`: what a barrier applies to. Backends
  `std::visit` it, so a target added later fails to compile wherever it is unhandled.
- `struct Barrier { BarrierTarget target; ResourceState before; ResourceState after; }`,
  comparable with `==`.
- `std::expected<void, ngen::Error> validate(const Barrier&, ResourceState trackedState)`: the
  barrier rules below, for every backend to share. A barrier breaking several rules gets one
  error, checked in order: `ShaderRead` on the swap chain image, `after` of `Undefined`, then
  `before` not matching `trackedState`.

**Requirements:**
- A buffer `size` is above 0, and `usage` is non-empty with no unknown bits.
- Shader `bytecode` is non-empty.
- Every attribute's `binding` names a binding in `bindings`; binding numbers are unique;
  attribute locations are unique.
- The vertex and fragment `entryPoint`s are non-empty.

### Device  (ngen_rhi/include/ngen/rhi/device.h)

The abstract interface a backend implements: creating and destroying resources, and running
frames that end by presenting to the swap chain.

**Interface:**
- `enum class Severity { Info, Warning, Error }`
- `using MessageSink = std::function<void(Severity, std::string_view)>`
- `struct DeviceDescription { std::string applicationName; bool enableValidation = false; MessageSink messageSink; }`:
  an empty `messageSink` means the default (warnings and errors to `stderr`, info dropped).
- `MessageSink makeDefaultMessageSink()`: that default sink.
- `class Device` (abstract):
  - `std::expected<BufferHandle, ngen::Error> createBuffer(const BufferDescription&)`
  - `std::expected<void, ngen::Error> writeBuffer(BufferHandle, uint64_t offset, std::span<const std::byte>)`:
    copies into the buffer at once; the caller must not write a range a frame in flight is reading.
  - `std::expected<ShaderHandle, ngen::Error> createShader(const ShaderDescription&)`
  - `std::expected<PipelineHandle, ngen::Error> createPipeline(const GraphicsPipelineDescription&)`
  - `void destroy(BufferHandle)`, `void destroy(ShaderHandle)`, `void destroy(PipelineHandle)`:
    retire the handle (see *Constraints*); a null or stale handle is ignored.
  - `std::expected<void, ngen::Error> attachWindow(const ngen::platform::NativeWindow&, ngen::platform::PixelSize)`:
    creates the swap chain; a device holds at most one, and a second call returns
    `InvalidArgument`.
  - `void onResized(ngen::platform::PixelSize)`, `void onSuspended()`,
    `std::expected<void, ngen::Error> onResumed(const ngen::platform::NativeWindow&, ngen::platform::PixelSize)`:
    the application forwards the window's events. With no window attached, `onResized` and
    `onSuspended` are ignored and `onResumed` returns `InvalidArgument`.
  - `std::expected<CommandList*, ngen::Error> beginFrame()`: waits until a frame slot is free and
    returns its command list, recording; `nullptr` (no error) when there is nothing to present to:
    no window attached, suspended, or a 0-sized window. The command list is valid until `endFrame`.
  - `std::expected<void, ngen::Error> endFrame()`: ends recording, submits, and presents. Called
    only after a `beginFrame` that returned a command list.
  - `ngen::platform::PixelSize swapChainSize() const`: the size of the images being presented;
    `{0, 0}` with no swap chain, which is the case while suspended or while the window is 0 in
    either dimension.
  - `void waitIdle()`: blocks until the GPU has finished all submitted work.
  - `virtual ~Device()`: waits for the GPU, then frees every resource still alive. A `Device` is
    not copyable.

**Requirements:**
- Creating from a description that breaks a *Descriptions* requirement returns `InvalidArgument`
  naming the field.
- `createPipeline` naming a null or stale shader returns `InvalidArgument`.
- `writeBuffer` past the end of the buffer, or to a stale handle, returns `InvalidArgument`.
- `beginFrame` while a frame is being recorded, or `endFrame` without one, reports an `Error`
  message to the sink and returns `InvalidArgument`.
- After `onResized`, the next `beginFrame` presents at the new size (the resize applies there); after `onSuspended`,
  `beginFrame` returns `nullptr` until `onResumed` succeeds.
- A destroyed resource stays alive while a frame begun before `destroy` has not finished; it is
  freed by the first `beginFrame` (or the destructor) after that frame finishes, whether or not
  that `beginFrame` returns a command list.

### CommandList  (ngen_rhi/include/ngen/rhi/command_list.h)

The abstract recorder for one frame's commands.

**Interface:**
- `void beginRendering(const ColorTarget&)`: starts rendering to this frame's swap chain image,
  with the full image as render area.
- `void endRendering()`
- `void setViewport(float x, float y, float width, float height, float minDepth = 0, float maxDepth = 1)`
- `void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)`
- `void bindPipeline(PipelineHandle)`
- `void bindVertexBuffer(uint32_t binding, BufferHandle, uint64_t offset = 0)`
- `void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0)`
- `void barrier(const Barrier&)`: makes every earlier use of the target in `before` finish
  before any later use in `after`, and changes its state to `after`. A barrier whose `before`
  and `after` are both `ColorAttachment` orders one pass's colour writes before the next's.

**Requirements:**
- Misuse skips the command and sends an `Error` message to the sink:
  - any command called after `endFrame` (reported as called after `endFrame`, nothing else);
  - `draw`, `bindPipeline` or `bindVertexBuffer` outside `beginRendering`…`endRendering`;
  - `beginRendering` while rendering, or `endRendering` while not rendering;
  - `bindVertexBuffer` with a buffer lacking `BufferUsage::Vertex`, or an offset at or past the
    buffer's size;
  - `barrier` inside `beginRendering`…`endRendering`; `barrier` whose `before` is not the
    target's tracked state (the message names both states); `barrier` with `after` of
    `Undefined`; `barrier` on the swap chain image with `before` or `after` of `ShaderRead`,
    because the swap chain image is never read by shaders.
- The command list tracks the swap chain image's state. It is `Undefined` at `beginFrame`, and
  each `barrier` on it leaves it in `after`.
- `beginRendering` on an image that is not in `ColorAttachment` first moves it there from its
  tracked state, as a barrier would; a `beginRendering` that fails makes no transition.
- `endFrame` moves the image from its tracked state to `Present`, unless it is already there.
- `setViewport` and `setScissor` may be called anywhere in a frame.
- `endFrame` with rendering still open sends an `Error` message and still submits.
- A null or stale handle passed to a command skips that command and sends an `Error` message
  naming it.
- In a GPU backend, each `beginRendering` sets the viewport and scissor to the full target
  image; a `setViewport` or `setScissor` lasts until the end of its pass.

### FakeDevice  (ngen_rhi/include/ngen/rhi/fake_device.h)

A `Device` that keeps resources in handle pools and records commands as values, for testing
code built on the RHI.

**Interface:**
- `explicit FakeDevice(DeviceDescription = {})`
- `using RecordedCommand = std::variant<…>`: one struct per `CommandList` call, holding its
  arguments, each comparable with `==`.
- `const std::vector<std::vector<RecordedCommand>> &frames() const`: the commands of each frame
  ended so far, in order.
- `size_t liveBuffers() const`, `liveShaders()`, `livePipelines()`: resources not yet freed.
- `void completeFrames()`: marks every submitted frame as finished on the "GPU". Until it is
  called, `FakeDevice` treats submitted frames as still executing, so deferred destruction can
  be tested; but a `beginFrame` that finds every frame slot busy finishes the oldest frame, as
  the GPU wait would. `waitIdle()` finishes every submitted frame.

**Requirements:**
- `FakeDevice` keeps every requirement of `Device` and `CommandList`, including the validation
  of descriptions, handles and call order.
- `attachWindow` accepts any `NativeWindow`; `beginFrame` returns `nullptr` before it.
- It records only the commands actually called (no default viewport or scissor), and keeps a
  buffer's size and usage but not its contents.
- `barrier` is recorded as the `Barrier` it was given. The transition `beginRendering` makes for
  itself is recorded as a `Barrier` just before the `beginRendering` command. The transition
  `endFrame` makes is not recorded.
- `FakeDevice` and its command list derive from `Device` and `CommandList` directly; a backend
  adds its own classes beneath the same bases.

**Acceptance:**
- A buffer destroyed during frame 1 is still counted by `liveBuffers()` after `endFrame`; after
  `completeFrames()` and the next `beginFrame`, it is not.
- A frame that records `beginRendering`, `bindPipeline(p)`, `bindVertexBuffer(0, b)`, `draw(3)`,
  `endRendering` appears in `frames()` as exactly `Barrier{SwapChainImage, Undefined, ColorAttachment}`
  then those five commands, with those arguments.
- A frame recording `barrier({SwapChainImage, Undefined, ColorAttachment})`, a pass,
  `barrier({SwapChainImage, ColorAttachment, ColorAttachment})` and a second pass records both
  barriers and no other.
- `barrier({SwapChainImage, ColorAttachment, Present})` at the start of a frame records nothing
  and sends one `Error` message naming `Undefined` and `ColorAttachment`.
- `draw(3)` before `beginRendering` records nothing and sends one `Error` message to the sink.
- After `onSuspended`, `beginFrame` returns `nullptr`; after `onResumed`, it returns a command list.
- `createPipeline` with an attribute naming binding 1 when only binding 0 exists returns
  `InvalidArgument`.


## Pending changes

### FakeDevice frame errors  [ready]

**Blocked by:** none

**Brief:** `FakeDevice` gains an opt-in way to make its next `beginFrame` or `endFrame` fail
with a given error, so code built on the RHI can test that it passes those errors through.

**Changes:**
- **Add** (FakeDevice, Interface): "`void failNextBeginFrame(ngen::Error)`,
  `void failNextEndFrame(ngen::Error)`: the next call of that function returns the given error
  instead of doing its work. Each applies to one call and is then cleared."
- **Add** (FakeDevice, Requirements): "A `beginFrame` failed through `failNextBeginFrame` begins
  no frame and returns no command list. An `endFrame` failed through `failNextEndFrame` still
  ends the open frame, so the next `beginFrame` can begin one, but the frame is not added to
  `frames()`. Neither sends a message to the sink. A failure set again before it is used
  replaces the earlier one."
- **Add** (FakeDevice, Acceptance): "After `failNextBeginFrame(DeviceLost, "lost")`,
  `beginFrame` returns that error (code and message), and the following `beginFrame` returns a
  command list."
- **Add** (FakeDevice, Acceptance): "After `failNextEndFrame(DeviceLost, "lost")`, a frame that
  records `draw`s and calls `endFrame` gets that error; `frames()` gains no frame, the sink
  received nothing, and the next `beginFrame` returns a command list."

**Read first:** `ngen_rhi/include/ngen/rhi/fake_device.h`, `ngen_rhi/source/fake_device.cpp`
and `ngen_rhi/tests/fake_device_tests.cpp`.

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| ngen_rhi/include/ngen/rhi/fake_device.h `FakeDevice` | modify | code |
| ngen_rhi/source/fake_device.cpp | modify | code |
| ngen_rhi/tests/fake_device_tests.cpp | modify | code |

**Fixed:** the two function names and signatures in *Changes*. A failure set and not yet used
does not affect any other call (`attachWindow`, `onSuspended`, resource creation).
**Free:** how the pending failure is stored; private helpers; the tests.
**Guardrails:**
- A failed call leaves the device in the state the requirement states: no frame after a failed
  `beginFrame`, no open frame after a failed `endFrame`.
- A failed `endFrame` still frees what a normal `endFrame` would let the next `beginFrame` free,
  so deferred destruction behaves as after a lost submit: resources destroyed during that
  frame are freed once frames are completed.

**Done when:**
- *Agent-checkable:* every requirement and acceptance criterion in Changes is covered by a
  test. The build and tests pass, and existing tests still pass, each checking what it checked.
- *Human check:* none: it is test infrastructure, proven by its tests.

**Delegation:** Sonnet implementer, light review: a test-only addition to `FakeDevice` that no
production code relies on.

### Test image barrier target  [ready]

**Blocked by:** [RHI Vulkan backend](rhi_vulkan.md), [Render graph](renderer/render_graph/render_graph.md)
**Milestone:** [Render graph and stages](../README.md)

**Brief:** `BarrierTarget` gains a test-only alternative, `TestImage`, that shaders may read
and that no device records barriers on. The render graph can then be given an imported
resource other than the swap chain image, so its read rules get tests through `compile`.

**Changes:**
- **Replace** (Descriptions, Interface): "`using BarrierTarget = std::variant<SwapChainImage>`:
  what a barrier applies to." → "`struct TestImage { uint32_t id; }`: a test-only target,
  comparable with `==`, for code that only computes barriers (such as a render graph's
  `compile`). Shaders may read it. `using BarrierTarget = std::variant<SwapChainImage,
  TestImage>`: what a barrier applies to."
- **Add** (Descriptions, Requirements): "`validate(Barrier, trackedState)` applies its
  `after` and `before` rules to a `TestImage` as to any target, and accepts `ShaderRead` on
  it."
- **Add** (CommandList, Requirements, misuse list): "`barrier` on a `TestImage` (reported as a
  test-only target, nothing else), because a device has no image behind it."
- **Add** (FakeDevice, Acceptance): "`barrier({TestImage{1}, Undefined, ShaderRead})` records
  nothing and sends one `Error` message naming the test-only target."
- **Add** (Render graph › RenderGraph, Requirements): "A `TestImage` import may be read by a
  pass's shaders; the swap chain image may not."
- **Add** (Render graph › RenderGraph, Acceptance): "With the swap chain image imported as
  `Undefined` and `TestImage{1}` imported as `Undefined`: a pass "w" (`order` 1) writing the
  test image, then a pass "r" (`order` 2) writing the swap chain image and reading the test
  image, compiles to "w" with barrier `{TestImage{1}, Undefined, ColorAttachment}`, then "r"
  with barriers `{SwapChainImage, Undefined, ColorAttachment}` then
  `{TestImage{1}, ColorAttachment, ShaderRead}`, in that order."
- **Add** (Render graph › RenderGraph, Acceptance): "With `TestImage{1}` imported as
  `ShaderRead`, two passes writing the swap chain image and each reading the test image give
  the first pass no barrier on the test image and the second none either."
- **Add** (Render graph › RenderGraph, Acceptance): "A pass reading `TestImage{1}` imported as
  `ShaderRead` while writing it as its colour target makes `compile` return `InvalidArgument`
  naming the pass; so does a pass reading the swap chain image imported as `ColorAttachment`."
- **Add** (Render graph › RenderGraph, Requirements): "`execute` of a graph with a pass that
  writes or reads a `TestImage` import returns `InvalidArgument` naming the first such pass in
  run order, and records nothing, because no device has an image behind a `TestImage`."
- **Add** (Render graph › RenderGraph, Acceptance): "`execute` onto a `FakeDevice` frame of the
  graph with passes "w" and "r" above returns `InvalidArgument` naming "w", and the frame
  records no command."
- **Add** (RHI Vulkan backend, Constraints): "A `barrier` on a `TestImage` records nothing and
  sends an `Error` message naming the test-only target."
- **Add** (RHI Vulkan backend, Acceptance): "In a frame, `barrier({TestImage{1}, Undefined,
  ShaderRead})` sends one `Error` message naming the test-only target and records no pipeline
  barrier; a following `barrier({SwapChainImage, Undefined, ColorAttachment})` is accepted and
  records one."

**Read first:** `ngen_rhi/include/ngen/rhi/descriptions.h`, `ngen_rhi/source/descriptions.cpp`,
`ngen_rhi/source/fake_device.cpp`, `ngen_rhi_vulkan/source/command_list.cpp`,
`ngen_renderer/source/render_graph.cpp` and `ngen_renderer/tests/render_graph_tests.cpp`.

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| ngen_rhi/include/ngen/rhi/descriptions.h `TestImage`, `BarrierTarget` | modify | code |
| ngen_rhi/source/descriptions.cpp `validate(const Barrier&, …)` | modify | code |
| ngen_rhi/source/fake_device.cpp `barrier`, `trackedState` | modify | code |
| ngen_rhi/tests/descriptions_tests.cpp, fake_device_tests.cpp | modify | code |
| ngen_rhi_vulkan/source/command_list.cpp `barrier`, `tracked` | modify | code |
| ngen_rhi_vulkan/tests/window_tests.cpp | modify | code |
| ngen_renderer/source/render_graph.cpp `shaderReadable`, `execute` | modify | code |
| ngen_renderer/tests/render_graph_tests.cpp | modify | code |

**Fixed:** the names `TestImage` and its `id` field. Every place that visits a
`BarrierTarget` keeps using `std::visit` with one case per alternative, so a later target
still fails to compile wherever it is unhandled.
**Free:** how a backend detects a `TestImage` before tracking state; private helpers; the
tests.
**Guardrails:**
- The rejection of a `TestImage` barrier happens before any state tracking, so the swap chain
  image's tracked state is unchanged by it.
- The render graph treats a `TestImage` like any imported resource: the only difference from
  the swap chain image is that shaders may read it.

**Done when:**
- *Agent-checkable:* every requirement and acceptance criterion in Changes is covered by a
  test. The build and tests pass, and existing tests still pass, each checking what it checked.
- *Human check:* none: it is test infrastructure and tests; the milestone check runs the
  example.

**Delegation:** Sonnet implementer, full review: it changes a public RHI type that every
backend and the render graph visit.
