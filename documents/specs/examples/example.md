# Example application — Spec

> - **Template:** v1 · **ID:** example
> - **Status:** built
> - **Blocked by:** [RHI Vulkan backend](../graphics/rhi_vulkan.md), [RHI](../graphics/rhi.md), [Platform](../platform/platform.md)
> - **Background (not needed to build):** Feature: [ngen](../README.md) · Design: [renderer](../../design/renderer.md) · Technical: none · Inputs: none

## Goal

The example application is ngen's visual check: a window that draws a vertex-coloured
triangle through ngen, and keeps drawing it as the window is resized, minimised and restored.
It is a composition root: it creates the SDL window and the Vulkan device and ties them
together. Its code is `example/`: `main.cpp` holds start-up, the frame loop and shutdown, and
`render.cpp` the shader loading and the triangle's resources.

## Scope

**In:**
- The window, device, triangle resources and frame loop of the example.
- Forwarding the window's events to the device.

**Out (non-goals):**
- Input beyond closing the window.
- Any rendering feature beyond the triangle.

## Constraints and key decisions

- The example uses `ngen::platform::SdlWindow` and `ngen::rhi::vulkan::VulkanDevice`, with
  validation on, and reaches the GPU only through `ngen::rhi::Device` and `CommandList`.
- The window is 640 × 480, resizable, titled "nGen - Vulkan Test".
- The triangle's vertices are positions (`R32G32Float`, location 0) and colours
  (`R32G32B32Float`, location 1), interleaved and packed in binding 0 (position at offset 0,
  colour at offset 8, stride 20): (0, −0.5) red, (0.5, 0.5) green,
  (−0.5, 0.5) blue. Its shader is `example/shaders/triangle.slang`, with entry points
  `vertexMain` and `fragmentMain`, compiled to `shaders/triangle.spv` beside the executable as
  the build does today. The pipeline culls nothing, so the winding has no effect.
- Each frame clears to black and draws the triangle over the whole window.
- The compiled shader is read before SDL is started, so a missing shader is reported before
  anything else is created.
- The example prints nothing when all is well, and exits with status 0 when the window is
  closed. Any failure, at start-up or after it (starting SDL, fetching the window's handles,
  `onResumed`, `beginFrame` or `endFrame` returning an error), is printed once to `stderr`; the example then
  shuts down as the guardrails order and exits with status 1. `beginFrame` returning no command
  list is not a failure.
- The frame loop passes every SDL event, polled or returned by `SDL_WaitEventTimeout`, to
  `SdlWindow::handleEvent`, then drains the window's `pollEvents` and forwards its `Resized`,
  `Suspended` and `Resumed` events to the device, and stops on `CloseRequested`, drawing no further frame. On `Resumed` it
  fetches fresh handles with `nativeWindow()` before calling `onResumed`, because handles from
  before a suspension are invalid.
  While `beginFrame` returns no command list, it waits for events (`SDL_WaitEventTimeout`, 100 ms)
  instead of spinning.
- The example links SDL3 only.
- Shutdown runs in reverse order of creation: every handle the example creates is destroyed
  before the device, the device before the window, and the window before `SDL_Quit`.

## Modules and interfaces

### Example  (example/main.cpp, example/render.cpp)

Starts SDL, creates the window and device, loads the triangle, runs the frame loop, and shuts
down in reverse order.

**Acceptance:**
- Started with the compiled shader missing, the example prints an error naming
  `shaders/triangle.spv` and exits with status 1.
- Run normally and closed, it exits with status 0 and prints nothing to `stdout` or `stderr`.

## Pending changes

### Example on the renderer  [ready]

**Blocked by:** [Renderer](../graphics/renderer/renderer.md)
**Milestone:** [Render graph and stages](../README.md)

**Brief:** The example draws its triangle through `ngen::renderer::Renderer`: each frame it
submits the triangle as one draw item in the Opaque stage, instead of recording commands
itself.

**Changes:**
- **Replace** (Goal): "It is a composition root: it creates the SDL window and the Vulkan
  device and ties them together." → "It is a composition root: it creates the SDL window, the
  Vulkan device and the renderer, and ties them together."
- **Replace** (Constraints): "The example uses `ngen::platform::SdlWindow` and
  `ngen::rhi::vulkan::VulkanDevice`, with validation on, and reaches the GPU only through
  `ngen::rhi::Device` and `CommandList`." → "The example uses `ngen::platform::SdlWindow`,
  `ngen::rhi::vulkan::VulkanDevice` with validation on, and `ngen::renderer::Renderer` with the
  default configuration. It creates its resources through `ngen::rhi::Device` and draws only
  through `Renderer::render`."
- **Replace** (Constraints): "Each frame clears to black and draws the triangle over the whole
  window." → "Each frame submits a render list holding one draw item: the triangle's pipeline
  and vertex buffer at binding 0, 3 vertices, in the stage `stage("Opaque")` returns. The
  renderer clears to black and draws it over the whole window."
- **Replace** (Constraints): "Any failure, at start-up or after it (starting SDL, fetching the
  window's handles, `onResumed`, `beginFrame` or `endFrame` returning an error)" → "Any failure,
  at start-up or after it (starting SDL, fetching the window's handles, creating the renderer,
  `onResumed` or `render` returning an error)"; and "`beginFrame` returning no command list is
  not a failure." → "`render` returning `Skipped` is not a failure."
- **Replace** (Constraints): "While `beginFrame` returns no command list, it waits for events" →
  "While `render` returns `Skipped`, it waits for events".
- **Replace** (Constraints): "The example links SDL3 only." → "Beyond the ngen modules, the
  example links SDL3 only."
- **Replace** (Constraints): "every handle the example creates is destroyed before the device,
  the device before the window" → "every handle the example creates, and the renderer, are
  destroyed before the device, the device before the window".
- **Replace** (Modules, Example): "Starts SDL, creates the window and device, loads the
  triangle, runs the frame loop, and shuts down in reverse order." → "Starts SDL, creates the
  window, device and renderer, loads the triangle, runs the frame loop, and shuts down in
  reverse order."

**Read first:** `example/main.cpp`, `example/render.h`, `example/render.cpp`,
`example/CMakeLists.txt`, and the renderer's headers in
`ngen_renderer/include/ngen/renderer/`.

**Touchpoints:**

| Path / symbol | Create / modify | Kind |
|---|---|---|
| example/main.cpp | modify | code |
| example/render.h, example/render.cpp `Triangle::draw` | modify | code |
| example/CMakeLists.txt (link `ngen::renderer`) | modify | config |

**Free:** whether `Triangle` returns its draw item or adds it to a list; reusing one render
list across frames.
**Guardrails:** the missing-shader check still runs before SDL starts, so its test keeps
checking what it checked.

**Done when:**
- *Agent-checkable:* the build and tests pass, and existing tests still pass, each checking
  what it checked; `example/` records no `CommandList` call (grep).
- *Human check:* run the example: the triangle draws as before, survives resizing, minimising
  and restoring, prints nothing, and exits with status 0 when closed.

**Delegation:** Sonnet implementer, light review: a composition root that nothing else
relies on.
