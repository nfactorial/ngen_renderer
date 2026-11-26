# Render graph — Spec

> - **Template:** v1 · **ID:** render-graph
> - **Status:** ready
> - **Blocked by:** [RHI](../../rhi.md)
> - **Background (not needed to build):** Feature: [Renderer](../README.md) · Design: [renderer](../../../../design/renderer.md) · Technical: [renderer](../../../../technical/renderer.md) · Inputs: none

## Goal

The render graph holds one frame's passes. Each pass declares the colour target it writes and
the resources it reads. The graph orders the passes, works out every barrier between them and
how each pass loads and stores its target, and records the frame onto an RHI `CommandList`.
Nothing that uses the graph writes a barrier or chooses a load op.

## Scope

**In:**
- Resources imported into the graph with their current state.
- Passes with an order key, one colour target, the resources they read, and a record
  callback.
- Compiling: pass order, barriers, and each pass's `ColorTarget`, as data.
- Executing the compiled frame onto a `CommandList`.

**Out (non-goals):**
- Resources the graph creates itself (transient targets), and memory aliasing.
- Culling passes whose output nothing uses.
- Depth attachments, and more than one colour target per pass.
- Recording passes on several threads.

## Constraints and key decisions

- Every public type is in namespace `ngen::renderer`. Public headers live under
  `ngen_renderer/include/ngen/renderer/` and are included as `<ngen/renderer/…>`.
- The CMake target is `ngen_renderer`, alias `ngen::renderer`, depending on `ngen::rhi`. It
  takes no other dependency. The test target is `ngen_renderer_tests`, using GoogleTest and
  `ngen::rhi`'s `FakeDevice`.
- Functions that can fail return `[[nodiscard]] std::expected<T, ngen::Error>`. No exceptions.
- A graph describes one frame. It is filled, compiled or executed, then cleared for the next
  frame.

## Modules and interfaces

### RenderGraph  (ngen_renderer/include/ngen/renderer/render_graph.h)

Owns one frame's passes and resources, and turns their declarations into an ordered frame
with every barrier and load op decided.

**Interface:**
- `struct ResourceId { uint32_t index; }`, comparable with `==`.
- `struct ColorWrite { ResourceId target; std::optional<std::array<float, 4>> clear; }`: the
  pass's colour target, cleared to `clear` when it is given.
- `struct PassDescription { std::string name; int32_t order = 0; ColorWrite color; std::vector<ResourceId> reads; std::function<void(rhi::CommandList&)> record; }`
- `struct CompiledPass { std::string name; std::vector<rhi::Barrier> barriers; rhi::ColorTarget colorTarget; }`:
  the barriers recorded before the pass, and its colour target.
- `struct CompiledGraph { std::vector<CompiledPass> passes; }`
- `class RenderGraph`:
  - `ResourceId importTarget(rhi::BarrierTarget, rhi::ResourceState current)`
  - `void addPass(PassDescription)`
  - `std::expected<CompiledGraph, ngen::Error> compile() const`
  - `std::expected<void, ngen::Error> execute(rhi::CommandList&) const`: compiles, then, for
    each pass in order, records its barriers, `beginRendering(colorTarget)`, its `record`
    callback and `endRendering`. When compiling fails it records nothing.
  - `void clear()`: removes every pass and resource.

**Requirements:**
- **Order:** passes run in ascending `order`, and passes with equal `order` run in the order
  they were added.
- **States:** a pass uses its colour target in `ColorAttachment` and each read in
  `ShaderRead`. Each resource starts in the state it was imported with.
- **Barriers:** before each pass, for each resource it uses, the graph adds one barrier from
  the resource's state to the state the pass uses it in when the two differ, or when an
  earlier pass wrote the resource. A read following a read in the same state needs none. A
  pass's barriers are in the order: colour target first, then its reads in declared order.
- **Load:** `Clear` with the given colour when `clear` is set; otherwise `Load` when the
  target has defined contents (an earlier pass wrote it, or it was imported in a state other
  than `Undefined`); otherwise `DontCare`. **Store:** always `Store`, because every resource is
  imported and outlives the frame.
- `compile` returns `InvalidArgument`, naming the pass, when: a pass name is empty or used
  twice; a pass names a `ResourceId` the graph did not issue; a pass reads its own colour
  target; a pass reads a resource with no defined contents at that point; or a pass reads the
  swap chain image, because the RHI never lets shaders read it.
- The graph adds no barrier after the last pass: the resource's final state is left to the
  caller.

**Acceptance:**
- With the swap chain image imported as `Undefined`, a pass "a" with `order` 2 writing it and
  a pass "b" with `order` 1 writing it with `clear` (0, 0, 0, 1): `compile` gives "b" then "a".
  "b" has barrier `{SwapChainImage, Undefined, ColorAttachment}` and target `Clear` (0, 0, 0,
  1), `Store`. "a" has barrier `{SwapChainImage, ColorAttachment, ColorAttachment}` and target
  `Load`, `Store`.
- Two passes with equal `order` run in the order they were added.
- One pass writing an `Undefined` import with no `clear` gets `DontCare`; writing an import in
  `ColorAttachment` with no `clear`, it gets `Load` and no barrier.
- A pass reading an `Undefined` import that no earlier pass writes, and a second pass named
  like the first, each make `compile` return `InvalidArgument` naming the pass.
- `execute` onto a `FakeDevice` frame records, for the first acceptance's graph: the "b"
  barrier, `beginRendering(Clear)`, the commands "b" records, `endRendering`, the "a" barrier,
  `beginRendering(Load)`, the commands "a" records, `endRendering`, and nothing else.

## Build

**Milestone:** [Render graph and stages](../../../README.md)

**Read first:** [RHI](../../rhi.md) and `ngen_rhi/include/ngen/rhi/`, `ngen_rhi/CMakeLists.txt` and
`ngen_rhi/tests/CMakeLists.txt` for the target layout, `CMakeLists.txt`.

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| ngen_renderer/CMakeLists.txt, ngen_renderer/tests/CMakeLists.txt | create | config |
| ngen_renderer/include/ngen/renderer/render_graph.h, ngen_renderer/source/render_graph.cpp | create | code |
| ngen_renderer/tests/render_graph_tests.cpp | create | code |
| ngen_renderer/readme.md | create | code |
| CMakeLists.txt (`add_subdirectory(ngen_renderer)`) | modify | config |

**Fixed:** the names in *Interface*; the target and namespace in *Constraints*;
`render_graph.h` opens with `// Spec: render-graph. Behaviour changes go through it.`;
`readme.md` is a short note pointing to this spec, as the other modules' readmes are.
**Free:** internal structure, private helpers and the tests.
**Guardrails:**
- `compile` is `const` and pure: compiling twice gives equal results, and `execute` compiles
  the same way.
- The resolved load op comes from the rule in *Requirements*. A pass's `clear` is the only
  input the caller gives about it.

**Done when:**
- *Agent-checkable:* every requirement and acceptance criterion is covered by a test. The
  build and tests pass on the machine in use, and existing tests still pass, each checking what
  it checked.
- *Human check:* none of its own: the milestone check runs it through the renderer.

**Delegation:** Sonnet implementer, full review: pure C++ against `FakeDevice` with stated
rules, but every later pass is built on it.
